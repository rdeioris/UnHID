// Copyright 2026 - Roberto De Ioris


#include "UnHIDDevice.h"

THIRD_PARTY_INCLUDES_START
#include "hidapi.h"
THIRD_PARTY_INCLUDES_END

#include "UnHIDBlueprintFunctionLibrary.h"

EUnHIDBusType UnHID::ToUnHIDBusType(const int32 BusType)
{
	switch (BusType)
	{
	case HID_API_BUS_USB:
		return EUnHIDBusType::USB;
	case HID_API_BUS_BLUETOOTH:
		return EUnHIDBusType::Bluetooth;
	case HID_API_BUS_I2C:
		return EUnHIDBusType::I2C;
	case HID_API_BUS_SPI:
		return EUnHIDBusType::SPI;
	case HID_API_BUS_VIRTUAL:
		return EUnHIDBusType::Virtual;
	default:
		break;
	}

	return EUnHIDBusType::Unknown;
}

void UnHID::FillDeviceInfo(const hid_device_info* CurrentDev, FUnHIDDeviceInfo& UnHIDDeviceInfo)
{
	UnHIDDeviceInfo.Path = UTF8_TO_TCHAR(CurrentDev->path);
	UnHIDDeviceInfo.VendorId = CurrentDev->vendor_id;
	UnHIDDeviceInfo.ProductId = CurrentDev->product_id;
	UnHIDDeviceInfo.SerialNumber = WCHAR_TO_TCHAR(CurrentDev->serial_number);
	UnHIDDeviceInfo.ReleaseNumber = CurrentDev->release_number;
	UnHIDDeviceInfo.Manufacturer = WCHAR_TO_TCHAR(CurrentDev->manufacturer_string);
	UnHIDDeviceInfo.Product = WCHAR_TO_TCHAR(CurrentDev->product_string);
	UnHIDDeviceInfo.UsagePage = CurrentDev->usage_page;
	UnHIDDeviceInfo.Usage = CurrentDev->usage;
	UnHIDDeviceInfo.InterfaceNumber = CurrentDev->interface_number;
	UnHIDDeviceInfo.BusType = UnHID::ToUnHIDBusType(CurrentDev->bus_type);
}

class FUnHIDDeviceWorkerThread : public FRunnable
{
public:
	FUnHIDDeviceWorkerThread(TWeakObjectPtr<UUnHIDDevice> InUnHIDDevice, hid_device* InHidDevice, const FUnHIDReadNativeDelegate& InReadNativeDelegate) : bStopThread(false)
	{
		UnHIDDevice = InUnHIDDevice;
		HidDevice = InHidDevice;
		ReadNativeDelegate = InReadNativeDelegate;
		Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("UnHIDDeviceWorkerThread@%p"), this));
	}

	virtual ~FUnHIDDeviceWorkerThread()
	{
		Stop();
		if (Thread)
		{
			Thread->WaitForCompletion();
			delete Thread;
		}

		Thread = nullptr;
	}

	// FRunnable interface
	virtual bool Init() override
	{
		return true;
	}

	virtual uint32 Run() override
	{
		ReadBuffer.AddZeroed(0xFFFF);

		while (!bStopThread)
		{
			const int32 ReadSize = hid_read_timeout(HidDevice, ReadBuffer.GetData(), ReadBuffer.NumBytes(), 100);
			if (ReadSize < 0)
			{
				const FString ErrorMessage = WCHAR_TO_TCHAR(hid_read_error(HidDevice));
				FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, ErrorMessage]()
					{
						if (UnHIDDevice.IsValid())
						{
							ReadNativeDelegate.Execute(UnHIDDevice.Get(), {}, ErrorMessage);
						}
					}, TStatId(), nullptr, ENamedThreads::GameThread);
				break;
			}
			else if (ReadSize == 0)
			{
				// timeout
				continue;
			}

			TArray<uint8> HidMessage;
			HidMessage.Append(ReadBuffer.GetData(), ReadSize);

			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, HidMessage]()
				{
					if (UnHIDDevice.IsValid())
					{
						ReadNativeDelegate.Execute(UnHIDDevice.Get(), HidMessage, "");
					}
				}, TStatId(), nullptr, ENamedThreads::GameThread);
		}

		return 0;
	}

	virtual void Stop() override
	{
		bStopThread = true;
	}

	virtual void Exit() override
	{

	}

private:
	FRunnableThread* Thread;

	TAtomic<bool> bStopThread;

	hid_device* HidDevice;

	TWeakObjectPtr<UUnHIDDevice> UnHIDDevice;
	FUnHIDReadNativeDelegate ReadNativeDelegate;

	TArray<uint8> ReadBuffer;
};

bool UUnHIDDevice::Initialize(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadNativeDelegate& InReadNativeDelegate, FString& ErrorMessage)
{
	if (HidDevice || UnHIDDeviceWorkerThread)
	{
		ErrorMessage = "Already initialized";
		return false;
	}

	if (!InReadNativeDelegate.IsBound())
	{
		ErrorMessage = "Unbound delegate";
		return false;
	}

	if (UnHIDDeviceInfo.Path.IsEmpty())
	{
		ErrorMessage = "Empty UnHIDDeviceInfo Path";
		return false;
	}

	HidDevice = hid_open_path(TCHAR_TO_UTF8(*UnHIDDeviceInfo.Path));
	if (!HidDevice)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(nullptr));
		return false;
	}

	ReportDescriptor = MakeShared<TArray<uint8>>();
	ReportDescriptor->AddUninitialized(HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

	const int32 ReportDescriptorSize = hid_get_report_descriptor(reinterpret_cast<hid_device*>(HidDevice), ReportDescriptor->GetData(), ReportDescriptor->Num());

	if (ReportDescriptorSize > 0)
	{
		ReportDescriptor->SetNum(ReportDescriptorSize);
	}
	else
	{
		ReportDescriptor = nullptr;
	}

	hid_device_info* HidDeviceInfo = hid_get_device_info(reinterpret_cast<hid_device*>(HidDevice));
	if (HidDeviceInfo)
	{
		DeviceInfo = MakeShared<FUnHIDDeviceInfo>();
		UnHID::FillDeviceInfo(HidDeviceInfo, *DeviceInfo);
	}

	UnHIDDeviceWorkerThread = new FUnHIDDeviceWorkerThread(TWeakObjectPtr<UUnHIDDevice>(this), reinterpret_cast<hid_device*>(HidDevice), InReadNativeDelegate);

	return true;
}

UUnHIDDevice::~UUnHIDDevice()
{
	Terminate();
}

void UUnHIDDevice::StopWorkerThread()
{
	if (UnHIDDeviceWorkerThread)
	{
		UnHIDDeviceWorkerThread->Stop();
	}
}

void UUnHIDDevice::Terminate()
{
	StopWorkerThread();

	if (UnHIDDeviceWorkerThread)
	{
		delete UnHIDDeviceWorkerThread;
	}

	UnHIDDeviceWorkerThread = nullptr;

	if (HidDevice)
	{
		hid_close(reinterpret_cast<hid_device*>(HidDevice));
	}

	HidDevice = nullptr;
}

bool UUnHIDDevice::WriteBytes(const TArray<uint8>& Bytes, FString& ErrorMessage)
{
	if (!HidDevice)
	{
		ErrorMessage = "Invalid HidDevice";
		return false;
	}

	const int32 WriteSize = hid_write(reinterpret_cast<hid_device*>(HidDevice), Bytes.GetData(), Bytes.Num());
	if (WriteSize <= 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return false;
	}

	return true;
}

bool UUnHIDDevice::WriteHexString(const FString& HexString, FString& ErrorMessage)
{
	return WriteBytes(UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToBytes(HexString), ErrorMessage);
}

bool UUnHIDDevice::GetFeatureReportBytes(const uint8 ReportId, const int32 Size, TArray<uint8>& Bytes, FString& ErrorMessage)
{
	if (!HidDevice)
	{
		ErrorMessage = "Invalid HidDevice";
		return false;
	}

	if (Size < 0)
	{
		ErrorMessage = "Negative Size";
		return false;
	}

	Bytes.SetNumZeroed(Size + 1);

	Bytes[0] = ReportId;

	const int32 ReportSize = hid_get_feature_report(reinterpret_cast<hid_device*>(HidDevice), Bytes.GetData(), Bytes.Num());
	if (ReportSize <= 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return false;
	}

	Bytes.SetNum(ReportSize);

	return true;
}

bool UUnHIDDevice::GetFeatureReportHexString(const uint8 ReportId, const int32 Size, FString& HexString, FString& ErrorMessage)
{
	TArray<uint8> Bytes;
	if (!GetFeatureReportBytes(ReportId, Size, Bytes, ErrorMessage))
	{
		return false;
	}

	HexString = UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString(Bytes);

	return true;
}

bool UUnHIDDevice::SetFeatureReportBytes(const TArray<uint8>& Bytes, FString& ErrorMessage)
{
	if (!HidDevice)
	{
		ErrorMessage = "Invalid HidDevice";
		return false;
	}

	const int32 WriteSize = hid_send_feature_report(reinterpret_cast<hid_device*>(HidDevice), Bytes.GetData(), Bytes.Num());
	if (WriteSize <= 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return false;
	}

	return true;
}

bool UUnHIDDevice::SetFeatureReportHexString(const FString& HexString, FString& ErrorMessage)
{
	return SetFeatureReportBytes(UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToBytes(HexString), ErrorMessage);
}

FString UUnHIDDevice::GetSerialNumberString(FString& ErrorMessage)
{
	if (!HidDevice)
	{
		ErrorMessage = "Invalid HidDevice";
		return "";
	}

	TArray<wchar_t> SerialNumberBuffer;
	SerialNumberBuffer.AddZeroed(256);

	int32 Result = hid_get_serial_number_string(reinterpret_cast<hid_device*>(HidDevice), SerialNumberBuffer.GetData(), 256);
	if (Result < 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return "";
	}

	return WCHAR_TO_TCHAR(SerialNumberBuffer.GetData());
}

TArray<uint8> UUnHIDDevice::GetReportDescriptor() const
{
	if (ReportDescriptor.IsValid())
	{
		return *ReportDescriptor;
	}

	return TArray<uint8>();
}

FUnHIDDeviceInfo UUnHIDDevice::GetDeviceInfo() const
{
	if (DeviceInfo.IsValid())
	{
		return *DeviceInfo;
	}

	return FUnHIDDeviceInfo();
}

bool UUnHIDDevice::GetBitOffsetAndSizeFromDescriptorReportsAndUsage(const int32 UsagePage, const int32 Usage, int64& BitOffset, int64& BitSize, FString& ErrorMessage)
{
	if (!ReportDescriptor.IsValid())
	{
		ErrorMessage = "Invalid Report Descriptor";
		return false;
	}

	const FUnHIDDeviceDescriptorReports DescriptorReports = UUnHIDBlueprintFunctionLibrary::UnHIDGetReportsFromReportDescriptorBytes(*ReportDescriptor, ErrorMessage);

	return UUnHIDBlueprintFunctionLibrary::UnHIDGetBitOffsetAndSizeFromDescriptorReportsAndUsage(DescriptorReports.Inputs, UsagePage, Usage, BitOffset, BitSize);
}