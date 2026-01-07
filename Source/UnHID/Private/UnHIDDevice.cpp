// Fill out your copyright notice in the Description page of Project Settings.


#include "UnHIDDevice.h"

THIRD_PARTY_INCLUDES_START
#include "hidapi.h"
THIRD_PARTY_INCLUDES_END

#include "UnHIDBlueprintFunctionLibrary.h"

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