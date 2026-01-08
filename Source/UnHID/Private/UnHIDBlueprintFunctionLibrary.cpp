// Copyright 2026 - Roberto De Ioris


#include "UnHIDBlueprintFunctionLibrary.h"

THIRD_PARTY_INCLUDES_START
#include "hidapi.h"
THIRD_PARTY_INCLUDES_END

#include "UnHID.h"
#include "UnHIDDevice.h"


TArray<FUnHIDDeviceInfo> UUnHIDBlueprintFunctionLibrary::UnHIDEnumerate()
{
	TArray<FUnHIDDeviceInfo> DeviceInfos;

	struct hid_device_info* Devs = hid_enumerate(0x0, 0x0);
	for (struct hid_device_info* CurrentDev = Devs; CurrentDev; CurrentDev = CurrentDev->next)
	{
		FUnHIDDeviceInfo DeviceInfo;

		UnHID::FillDeviceInfo(CurrentDev, DeviceInfo);

		DeviceInfos.Add(MoveTemp(DeviceInfo));
	}

	if (Devs)
	{
		hid_free_enumeration(Devs);
	}

	return DeviceInfos;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDUsagePageToString(const int32 UsagePage)
{
	if (UsagePage >= 0xF1D1 && UsagePage <= 0xFEFF)
	{
		return "Reserved";
	}

	if (UsagePage >= 0xFF00 && UsagePage <= 0xFFFF)
	{
		return "Vendor-defined";
	}

	switch (UsagePage)
	{
	case 0x01:
		return "Generic Desktop";
	case 0x02:
		return "Simulation Controls";
	case 0x03:
		return "VR Controls";
	case 0x04:
		return "Sport Controls";
	case 0x05:
		return "Game Controls";
	case 0x06:
		return "Generic Device Controls";
	case 0x07:
		return "Keyboard/Keypad";
	case 0x08:
		return "LED";
	case 0x09:
		return "Button";
	case 0x0A:
		return "Ordinal";
	case 0x0B:
		return "Telephony Device";
	case 0x0C:
		return "Consumer";
	case 0x0D:
		return "Digitizers";
	case 0x0E:
		return "Haptics";
	case 0x0F:
		return "Physical Input Device";
	case 0x10:
		return "Unicode";
	case 0x11:
		return "SoC";
	case 0x12:
		return "Eye and Head Trackers";
	case 0x14:
		return "Auxiliary Display";
	case 0x20:
		return "Sensors";
	case 0x40:
		return "Medical Instrument";
	case 0x41:
		return "Braille Display";
	case 0x59:
		return "Lighting And Illumination";
	case 0x80:
		return "Monitor";
	case 0x81:
		return "Monitor Enumerated";
	case 0x82:
		return "VESA Virtual Controls";
	case 0x84:
		return "Power";
	case 0x85:
		return "Battery System";
	case 0x8C:
		return "Barcode Scanner";
	case 0x8D:
		return "Scales";
	case 0x8E:
		return "Magnetic Stripe Reader";
	case 0x90:
		return "Camera Control";
	case 0x91:
		return "Arcade";
	case 0x92:
		return "Gaming Device";
	case 0xF1D0:
		return "FIDO Alliance";
	default:
		break;
	}

	return "Unknown";
}

void UUnHIDBlueprintFunctionLibrary::UnHIDGetHidapiVersion(int32& Major, int32& Minor, int32& Patch)
{
	const struct hid_api_version* HidApiVersion = hid_version();
	Major = HidApiVersion->major;
	Minor = HidApiVersion->minor;
	Patch = HidApiVersion->patch;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDGetHidapiVersionString()
{
	return UTF8_TO_TCHAR(hid_version_str());
}

TArray<uint8> UUnHIDBlueprintFunctionLibrary::UnHIDGetReportDescriptor(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage)
{
	TArray<uint8> Descriptor;
	if (UnHIDDeviceInfo.Path.IsEmpty())
	{
		ErrorMessage = "Empty UnHIDDeviceInfo Path";
		return Descriptor;
	}

	hid_device* HidDevice = hid_open_path(TCHAR_TO_UTF8(*UnHIDDeviceInfo.Path));
	if (!HidDevice)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(nullptr));
		return Descriptor;
	}

	Descriptor.AddUninitialized(HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

	const int32 ReportDescriptorSize = hid_get_report_descriptor(HidDevice, Descriptor.GetData(), Descriptor.Num());

	if (ReportDescriptorSize <= 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(HidDevice));
		hid_close(HidDevice);
		Descriptor.Empty();
		return Descriptor;
	}

	Descriptor.SetNum(ReportDescriptorSize);

	hid_close(HidDevice);

	return Descriptor;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDGetSerialNumberString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage)
{
	if (UnHIDDeviceInfo.Path.IsEmpty())
	{
		ErrorMessage = "Empty UnHIDDeviceInfo Path";
		return "";
	}

	hid_device* HidDevice = hid_open_path(TCHAR_TO_UTF8(*UnHIDDeviceInfo.Path));
	if (!HidDevice)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(nullptr));
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

	const FString SerialNumberString = WCHAR_TO_TCHAR(SerialNumberBuffer.GetData());

	hid_close(HidDevice);

	return SerialNumberString;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDGetManufacturerString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage)
{
	if (UnHIDDeviceInfo.Path.IsEmpty())
	{
		ErrorMessage = "Empty UnHIDDeviceInfo Path";
		return "";
	}

	hid_device* HidDevice = hid_open_path(TCHAR_TO_UTF8(*UnHIDDeviceInfo.Path));
	if (!HidDevice)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(nullptr));
		return "";
	}

	TArray<wchar_t> ManufacturerBuffer;
	ManufacturerBuffer.AddZeroed(256);

	int32 Result = hid_get_manufacturer_string(reinterpret_cast<hid_device*>(HidDevice), ManufacturerBuffer.GetData(), 256);
	if (Result < 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return "";
	}

	const FString ManufacturerString = WCHAR_TO_TCHAR(ManufacturerBuffer.GetData());

	hid_close(HidDevice);

	return ManufacturerString;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDGetProductString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage)
{
	if (UnHIDDeviceInfo.Path.IsEmpty())
	{
		ErrorMessage = "Empty UnHIDDeviceInfo Path";
		return "";
	}

	hid_device* HidDevice = hid_open_path(TCHAR_TO_UTF8(*UnHIDDeviceInfo.Path));
	if (!HidDevice)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(nullptr));
		return "";
	}

	TArray<wchar_t> ProductBuffer;
	ProductBuffer.AddZeroed(256);

	int32 Result = hid_get_product_string(reinterpret_cast<hid_device*>(HidDevice), ProductBuffer.GetData(), 256);
	if (Result < 0)
	{
		ErrorMessage = WCHAR_TO_TCHAR(hid_error(reinterpret_cast<hid_device*>(HidDevice)));
		return "";
	}

	const FString ProductString = WCHAR_TO_TCHAR(ProductBuffer.GetData());

	hid_close(HidDevice);

	return ProductString;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString(const TArray<uint8>& Bytes)
{
	FString HexString;
	for (int32 ByteIndex = 0; ByteIndex < Bytes.Num(); ByteIndex++)
	{
		HexString += FString::Printf(TEXT("%02X "), Bytes.GetData()[ByteIndex]);
	}
	return HexString.LeftChop(1);
}

UUnHIDDevice* UUnHIDBlueprintFunctionLibrary::UnHIDOpenDevice(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage)
{
	UUnHIDDevice* UnHIDDevice = NewObject<UUnHIDDevice>();
	if (!UnHIDDevice)
	{
		return nullptr;
	}

	FUnHIDReadNativeDelegate UnHIDReadNativeDelegate;

	UnHIDReadNativeDelegate.BindLambda([InUnHIDReadDynamicDelegate](UUnHIDDevice* UnHIDDevice, const TArray<uint8>& Data, const FString& ErrorMessage)
		{
			InUnHIDReadDynamicDelegate.ExecuteIfBound(UnHIDDevice, Data, ErrorMessage);
		});

	if (!UnHIDDevice->Initialize(UnHIDDeviceInfo, UnHIDReadNativeDelegate, ErrorMessage))
	{
		return nullptr;
	}

	return UnHIDDevice;
}

UUnHIDDevice* UUnHIDBlueprintFunctionLibrary::UnHIDOpenDeviceByUsageFilter(const int32 UsagePage, const int32 Usage, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage)
{
	for (const FUnHIDDeviceInfo& UnHIDDeviceInfo : UnHIDEnumerate())
	{
		if (UnHIDDeviceInfo.UsagePage == UsagePage && UnHIDDeviceInfo.Usage == Usage)
		{
			return UnHIDOpenDevice(UnHIDDeviceInfo, InUnHIDReadDynamicDelegate, ErrorMessage);
		}
	}

	return nullptr;
}

TArray<UUnHIDDevice*> UUnHIDBlueprintFunctionLibrary::UnHIDOpenDevicesByUsageFilter(const int32 UsagePage, const int32 Usage, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, TArray<FString>& ErrorMessages)
{
	TArray<UUnHIDDevice*> UnHIDDevices;

	for (const FUnHIDDeviceInfo& UnHIDDeviceInfo : UnHIDEnumerate())
	{
		if (UnHIDDeviceInfo.UsagePage == UsagePage && UnHIDDeviceInfo.Usage == Usage)
		{
			FString ErrorMessage;
			UUnHIDDevice* UnHIDDevice = UnHIDOpenDevice(UnHIDDeviceInfo, InUnHIDReadDynamicDelegate, ErrorMessage);
			if (!UnHIDDevice)
			{
				ErrorMessages.Add(ErrorMessage);
			}
			UnHIDDevices.Add(UnHIDDevice);
		}
	}

	return UnHIDDevices;
}

UUnHIDDevice* UUnHIDBlueprintFunctionLibrary::UnHIDOpenDeviceByUsageFilterHexStrings(const FString& UsagePageHexString, const FString& UsageHexString, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage)
{
	return UnHIDOpenDeviceByUsageFilter(UnHIDHexStringToInt32(UsagePageHexString), UnHIDHexStringToInt32(UsageHexString), InUnHIDReadDynamicDelegate, ErrorMessage);
}

TArray<UUnHIDDevice*> UUnHIDBlueprintFunctionLibrary::UnHIDOpenDevicesByUsageFilterHexStrings(const FString& UsagePageHexString, const FString& UsageHexString, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, TArray<FString>& ErrorMessages)
{
	return UnHIDOpenDevicesByUsageFilter(UnHIDHexStringToInt32(UsagePageHexString), UnHIDHexStringToInt32(UsageHexString), InUnHIDReadDynamicDelegate, ErrorMessages);
}

UUnHIDDevice* UUnHIDBlueprintFunctionLibrary::UnHIDOpenDevice(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadNativeDelegate& InUnHIDReadNativeDelegate, FString& ErrorMessage)
{
	UUnHIDDevice* UnHIDDevice = NewObject<UUnHIDDevice>();
	if (!UnHIDDevice)
	{
		return nullptr;
	}

	if (!UnHIDDevice->Initialize(UnHIDDeviceInfo, InUnHIDReadNativeDelegate, ErrorMessage))
	{
		return nullptr;
	}

	return UnHIDDevice;
}

TArray<uint8> UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToBytes(const FString& HexString)
{
	TArray<uint8> OutputBytes;

	FString CleanHexString = HexString;

	CleanHexString = CleanHexString.ToLower().Replace(TEXT("0x"), TEXT(""));

	int32 SymbolsCounter = 0;
	for (TCHAR Char : CleanHexString)
	{
		if (FChar::IsWhitespace(Char))
		{
			continue;
		}

		SymbolsCounter += 1;
	}

	// leading '0'
	if ((SymbolsCounter % 2) != 0)
	{
		CleanHexString = "0" + CleanHexString;
	}

	uint8 CurrentValue = 0;
	bool bFirst = true;
	for (TCHAR Char : CleanHexString)
	{
		if (FChar::IsWhitespace(Char))
		{
			continue;
		}

		if (Char >= '0' && Char <= '9')
		{
			if (bFirst)
			{
				CurrentValue = (Char - '0') << 4;
			}
			else
			{
				CurrentValue |= (Char - '0') & 0x0f;
			}
		}
		else if (Char >= 'a' && Char <= 'f')
		{
			if (bFirst)
			{
				CurrentValue = (10 + (Char - 'a')) << 4;
			}
			else
			{
				CurrentValue |= (10 + (Char - 'a')) & 0x0f;
			}
		}
		else
		{
			return TArray<uint8>();
		}

		if (!bFirst)
		{
			OutputBytes.Add(CurrentValue);
		}

		bFirst = !bFirst;
	}

	return OutputBytes;
}

int32 UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToInt32(const FString& HexString)
{
	TArray<uint8> Bytes = UnHIDHexStringToBytes(HexString);
	if (Bytes.Num() < 4)
	{
		Bytes.AddZeroed(4 - Bytes.Num());
	}

	return *(reinterpret_cast<const int32*>(Bytes.GetData()));
}

FUnHIDDeviceDescriptorReports UUnHIDBlueprintFunctionLibrary::UnHIDGetReportsFromReportDescriptorBytes(const TArray<uint8>& UnHIDReportDescriptorBytes, FString& ErrorMessage)
{
	struct FReportDescriptorGlobalState
	{
		int64 UsagePage = 0;
		int64 LogicalMinimum = 0;
		int64 LogicalMaximum = 0;
		int64 PhysicalMinimum = 0;
		int64 PhysicalMaximum = 0;
		int64 UnitExponent = 0;
		int64 Unit = 0;
		int64 ReportSize = 0;
		int64 ReportID = 0;
		int64 ReportCount = 0;
	};

	struct FReportDescriptorLocalState
	{
		TArray<int64> Usage;
		int64 UsageMinimum = 0;
		int64 UsageMaximum = 0;
		int64 DesignatorIndex = 0;
		int64 DesignatorMinimum = 0;
		int64 DesignatorMaximum = 0;
		int64 StringIndex = 0;
		int64 StringMinimum = 0;
		int64 StringMaximum = 0;
		int64 Delimiter = 0;
	};

	TArray<FReportDescriptorGlobalState> GlobalStack;
	FReportDescriptorLocalState LocalState;

	GlobalStack.AddDefaulted();

	FUnHIDDeviceDescriptorReports DescriptorReports;

	auto GetOrCreateDescriptorReportInput = [&DescriptorReports](const int64 ReportID) -> FUnHIDDeviceDescriptorReport&
		{
			for (FUnHIDDeviceDescriptorReport& DescriptorReportInput : DescriptorReports.Inputs)
			{
				if (DescriptorReportInput.ReportId == ReportID)
				{
					return DescriptorReportInput;
				}
			}

			return DescriptorReports.Inputs.AddDefaulted_GetRef();
		};

	auto GetOrCreateDescriptorReportOutput = [&DescriptorReports](const int64 ReportID) -> FUnHIDDeviceDescriptorReport&
		{
			for (FUnHIDDeviceDescriptorReport& DescriptorReportOutput : DescriptorReports.Outputs)
			{
				if (DescriptorReportOutput.ReportId == ReportID)
				{
					return DescriptorReportOutput;
				}
			}

			return DescriptorReports.Outputs.AddDefaulted_GetRef();
		};

	auto GetOrCreateDescriptorReportFeature = [&DescriptorReports](const int64 ReportID) -> FUnHIDDeviceDescriptorReport&
		{
			for (FUnHIDDeviceDescriptorReport& DescriptorReportFeature : DescriptorReports.Features)
			{
				if (DescriptorReportFeature.ReportId == ReportID)
				{
					return DescriptorReportFeature;
				}
			}

			return DescriptorReports.Features.AddDefaulted_GetRef();
		};

	auto FillDescriptorReport = [](FUnHIDDeviceDescriptorReport& DescriptorReport, const FReportDescriptorGlobalState& GlobalState, const FReportDescriptorLocalState& LocalState) {
		DescriptorReport.ReportId = GlobalState.ReportID;

		FUnHIDDeviceDescriptorReportItem DescriptorReportItem;
		DescriptorReportItem.BitOffset = DescriptorReport.NumBits;
		DescriptorReportItem.BitSize = GlobalState.ReportSize;
		DescriptorReportItem.Count = GlobalState.ReportCount;
		DescriptorReportItem.UsagePage = GlobalState.UsagePage;
		DescriptorReportItem.Usage = LocalState.Usage;
		DescriptorReportItem.UsageMinimum = LocalState.UsageMinimum;
		DescriptorReportItem.UsageMaximum = LocalState.UsageMaximum;
		DescriptorReportItem.LogicalMinimum = GlobalState.LogicalMinimum;
		DescriptorReportItem.LogicalMaximum = GlobalState.LogicalMaximum;
		DescriptorReportItem.PhysicalMinimum = GlobalState.PhysicalMinimum;
		DescriptorReportItem.PhysicalMaximum = GlobalState.PhysicalMaximum;
		DescriptorReportItem.UnitExponent = GlobalState.UnitExponent;
		DescriptorReportItem.Unit = GlobalState.Unit;

		DescriptorReport.Items.Add(MoveTemp(DescriptorReportItem));

		DescriptorReport.NumBits += GlobalState.ReportSize * GlobalState.ReportCount;
		DescriptorReport.NumBytes = (DescriptorReport.NumBits + 7) / 8;
		};

	int32 Offset = 0;
	while (Offset < UnHIDReportDescriptorBytes.Num())
	{
		const uint8 CurrentByte = UnHIDReportDescriptorBytes[Offset++];

		const uint8 ItemSize = CurrentByte & 0x3;
		const uint8 ItemType = (CurrentByte >> 2) & 0x3;
		const uint8 ItemTag = (CurrentByte >> 4) & 0xF;

		uint64 UnsignedValue = 0;
		uint64 SignedValue = 0;

		if (ItemTag == 0xF)
		{
			if (Offset + 1 > UnHIDReportDescriptorBytes.Num() || Offset + 1 + UnHIDReportDescriptorBytes[Offset + 1] > UnHIDReportDescriptorBytes.Num())
			{
				ErrorMessage = FString::Printf(TEXT("Not enough data at offset %d"), Offset);
				return DescriptorReports;
			}

			Offset++;

			const uint8 LongItemSize = UnHIDReportDescriptorBytes[Offset];

			Offset += LongItemSize;
		}
		else
		{
			if (Offset + (ItemSize == 3 ? 4 : ItemSize) > UnHIDReportDescriptorBytes.Num())
			{
				ErrorMessage = FString::Printf(TEXT("Not enough data at offset %d"), Offset);
				return DescriptorReports;
			}

			if (ItemSize == 1)
			{
				UnsignedValue = UnHIDReportDescriptorBytes[Offset];
				SignedValue = *(reinterpret_cast<const int8*>(UnHIDReportDescriptorBytes.GetData() + Offset));
				Offset++;
			}
			else if (ItemSize == 2)
			{
				UnsignedValue = *(reinterpret_cast<const uint16*>(UnHIDReportDescriptorBytes.GetData() + Offset));
				SignedValue = *(reinterpret_cast<const int16*>(UnHIDReportDescriptorBytes.GetData() + Offset));
				Offset += 2;
			}
			else if (ItemSize == 3)
			{
				UnsignedValue = *(reinterpret_cast<const uint32*>(UnHIDReportDescriptorBytes.GetData() + Offset));
				SignedValue = *(reinterpret_cast<const int32*>(UnHIDReportDescriptorBytes.GetData() + Offset));
				Offset += 4;
			}
		}

		// Main
		if (ItemType == 0)
		{
			const FReportDescriptorGlobalState& GlobalState = GlobalStack.Last();

			switch (static_cast<EUnHIDReportDescriptorMainItems>(ItemTag))
			{
			case EUnHIDReportDescriptorMainItems::Input:
			{
				FUnHIDDeviceDescriptorReport& DescriptorReportInput = GetOrCreateDescriptorReportInput(GlobalState.ReportID);
				FillDescriptorReport(DescriptorReportInput, GlobalState, LocalState);
			}
			break;
			case EUnHIDReportDescriptorMainItems::Output:
			{
				FUnHIDDeviceDescriptorReport& DescriptorReportOutput = GetOrCreateDescriptorReportOutput(GlobalState.ReportID);
				FillDescriptorReport(DescriptorReportOutput, GlobalState, LocalState);
			}
			break;
			case EUnHIDReportDescriptorMainItems::Feature:
			{
				FUnHIDDeviceDescriptorReport& DescriptorReportFeature = GetOrCreateDescriptorReportFeature(GlobalState.ReportID);
				FillDescriptorReport(DescriptorReportFeature, GlobalState, LocalState);
			}
			break;
			default:
				break;
			}

			// reset local state
			LocalState = {};
		}
		// Global
		else if (ItemType == 1)
		{
			switch (static_cast<EUnHIDReportDescriptorGlobalItems>(ItemTag))
			{
			case EUnHIDReportDescriptorGlobalItems::UsagePage:
				GlobalStack.Last().UsagePage = UnsignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::LogicalMinimum:
				GlobalStack.Last().LogicalMinimum = SignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::LogicalMaximum:
				GlobalStack.Last().LogicalMaximum = SignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::PhysicalMinimum:
				GlobalStack.Last().PhysicalMinimum = SignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::PhysicalMaximum:
				GlobalStack.Last().PhysicalMaximum = SignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::UnitExponent:
				GlobalStack.Last().UnitExponent = SignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::Unit:
				GlobalStack.Last().Unit = UnsignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::ReportSize:
				GlobalStack.Last().ReportSize = UnsignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::ReportID:
				GlobalStack.Last().ReportID = UnsignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::ReportCount:
				GlobalStack.Last().ReportCount = UnsignedValue;
				break;
			case EUnHIDReportDescriptorGlobalItems::Push:
				GlobalStack.AddDefaulted();
				break;
			case EUnHIDReportDescriptorGlobalItems::Pop:
				if (GlobalStack.Num() < 1)
				{
					ErrorMessage = "Global Stack is Empty";
					return DescriptorReports;
				}
				GlobalStack.Pop();
				break;
			default:
				break;
			}
		}
		// Local
		else if (ItemType == 2)
		{
			switch (static_cast<EUnHIDReportDescriptorLocalItems>(ItemTag))
			{
			case EUnHIDReportDescriptorLocalItems::Usage:
				LocalState.Usage.Add(UnsignedValue);
				break;
			case EUnHIDReportDescriptorLocalItems::UsageMinimum:
				LocalState.UsageMinimum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::UsagelMaximum:
				LocalState.UsageMaximum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::DesignatorIndex:
				LocalState.DesignatorIndex = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::DesignatorMinimum:
				LocalState.DesignatorMinimum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::DesignatorMaximum:
				LocalState.DesignatorMaximum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::StringIndex:
				LocalState.StringIndex = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::StringMinimum:
				LocalState.StringMinimum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::StringMaximum:
				LocalState.StringMaximum = UnsignedValue;
				break;
			case EUnHIDReportDescriptorLocalItems::Delimiter:
				LocalState.Delimiter = UnsignedValue;
				break;
			default:
				break;
			}
		}
	}

	DescriptorReports.bValid = true;

	return DescriptorReports;
}

void UUnHIDBlueprintFunctionLibrary::UnHIDSetVitualInputDeviceAxis(const int32 ControllerId, const uint8 AxisId, const float Value)
{
	FUnHIDModule::Get().SetVitualInputDeviceAxis(ControllerId, AxisId, Value);
}

TArray<bool> UUnHIDBlueprintFunctionLibrary::UnHIDParseBitmaskFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize)
{
	TArray<bool> Bitmask;

	for (int64 Index = 0; Index < BitSize; Index++)
	{
		const uint64 BitIndex = BitOffset + Index;
		const uint64 ByteIndex = BitIndex / 8;
		const uint64 BitInByte = BitIndex % 8;

		if (!Bytes.IsValidIndex(ByteIndex))
		{
			break;
		}

		const uint8 Bit = (Bytes[ByteIndex] >> BitInByte) & 1;
		Bitmask.Add(Bit == 1);
	}

	return Bitmask;
}

int64 UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize)
{
	if (BitOffset < 0 || BitSize <= 0 || BitSize > 64)
	{
		return 0;
	}

	uint64 Value = 0;

	for (int64 Index = 0; Index < BitSize; Index++)
	{
		const uint64 BitIndex = BitOffset + Index;
		const uint64 ByteIndex = BitIndex / 8;
		const uint64 BitInByte = BitIndex % 8;

		if (!Bytes.IsValidIndex(ByteIndex))
		{
			break;
		}

		const uint64 Bit = (Bytes[ByteIndex] >> BitInByte) & 1;
		Value |= (Bit << Index);
	}

	return Value;
}

int64 UUnHIDBlueprintFunctionLibrary::UnHIDParseSignedIntegerFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize)
{
	const int64 Value = UnHIDParseUnsignedIntegerFromBytes(Bytes, BitOffset, BitSize);

	const uint64 Mask = (1ULL << BitSize) - 1;

	// sign extend?
	if (Value & (1ULL << (BitSize - 1)))
	{
		return static_cast<int64>(Value | ~Mask);
	}

	return Value;
}

float UUnHIDBlueprintFunctionLibrary::UnHIDParseAnalogFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize, const int64 Minimum, const int64 Maximum, const float AnalogMin, const float AnalogMax)
{
	if (Minimum < 0 || Maximum < 0)
	{
		return FMath::GetMappedRangeValueClamped(TRange<double>(Minimum, Maximum), TRange<double>(AnalogMin, AnalogMax), static_cast<double>(UnHIDParseSignedIntegerFromBytes(Bytes, BitOffset, BitSize)));
	}

	return FMath::GetMappedRangeValueClamped(TRange<double>(Minimum, Maximum), TRange<double>(AnalogMin, AnalogMax), static_cast<double>(UnHIDParseUnsignedIntegerFromBytes(Bytes, BitOffset, BitSize)));
}

bool UUnHIDBlueprintFunctionLibrary::UnHIDGetBitOffsetAndSizeFromDescriptorReportsAndUsage(const TArray<FUnHIDDeviceDescriptorReport>& UnHIDDescriptorReports, const int32 UsagePage, const int32 Usage, int64& BitOffset, int64& BitSize)
{
	const bool bHasReportIdPrefix = UnHIDDescriptorReports.Num() > 1 || (UnHIDDescriptorReports.Num() == 1 && UnHIDDescriptorReports[0].ReportId != 0);
	for (const FUnHIDDeviceDescriptorReport& UnHIDDeviceDescriptorReport : UnHIDDescriptorReports)
	{
		for (const FUnHIDDeviceDescriptorReportItem& UnHIDDeviceDescriptorReportItem : UnHIDDeviceDescriptorReport.Items)
		{
			if (UnHIDDeviceDescriptorReportItem.UsagePage == UsagePage)
			{
				if (UnHIDDeviceDescriptorReportItem.Usage.Contains(Usage))
				{
					BitOffset = (bHasReportIdPrefix ? 8 : 0) + UnHIDDeviceDescriptorReportItem.BitOffset + (UnHIDDeviceDescriptorReportItem.BitSize * UnHIDDeviceDescriptorReportItem.Usage.IndexOfByKey(Usage));
					BitSize = UnHIDDeviceDescriptorReportItem.BitSize;
					return true;
				}
				else if (Usage >= UnHIDDeviceDescriptorReportItem.UsageMinimum && Usage <= UnHIDDeviceDescriptorReportItem.UsageMaximum)
				{
					BitOffset = (bHasReportIdPrefix ? 8 : 0) + UnHIDDeviceDescriptorReportItem.BitOffset + (UnHIDDeviceDescriptorReportItem.BitSize * UnHIDDeviceDescriptorReportItem.Usage.Num()) + (UnHIDDeviceDescriptorReportItem.BitSize * (Usage - UnHIDDeviceDescriptorReportItem.UsageMinimum));
					BitSize = UnHIDDeviceDescriptorReportItem.BitSize;
					return true;
				}
			}
		}
	}

	return false;
}

FString UUnHIDBlueprintFunctionLibrary::UnHIDInt32ToHexString(const int32 Value)
{
	TArray<uint8> Bytes;
	Bytes.Append(reinterpret_cast<const uint8*>(&Value), sizeof(int32));

	return UnHIDBytesToHexString(Bytes);
}