// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnHIDDevice.h"
#include "UnHIDBlueprintFunctionLibrary.generated.h"


DECLARE_DYNAMIC_DELEGATE_ThreeParams(FUnHIDReadDynamicDelegate, UUnHIDDevice*, UnHIDDevice, const TArray<uint8>&, Data, const FString&, ErrorMessage);

UENUM()
enum class EUnHIDBusType : uint8
{
	Unknown,
	USB,
	Bluetooth,
	I2C,
	SPI,
	Virtual
};

namespace UnHID
{
	EUnHIDBusType ToUnHIDBusType(const int32 BusType);
}

UENUM()
enum class EUnHIDReportDescriptorGlobalItems : uint8
{
	UsagePage = 0x0,
	LogicalMinimum = 0x1,
	LogicalMaximum = 0x2,
	PhysicalMinimum = 0x3,
	PhysicalMaximum = 0x4,
	UnitExponent = 0x5,
	Unit = 0x6,
	ReportSize = 0x7,
	ReportID = 0x8,
	ReportCount = 0x9,
	Push = 0xA,
	Pop = 0xB,
	ReservedMin = 0xC,
	ReservedMax = 0xF,
};

UENUM()
enum class EUnHIDReportDescriptorLocalItems : uint8
{
	Usage = 0x0,
	UsageMinimum = 0x1,
	UsagelMaximum = 0x2,
	DesignatorIndex = 0x3,
	DesignatorMinimum = 0x4,
	DesignatorMaximum = 0x5,
	StringIndex = 0x7,
	StringMinimum = 0x8,
	StringMaximum = 0x9,
	Delimiter = 0xA,
	ReservedMin = 0xB,
	ReservedMax = 0xF,
};

UENUM()
enum class EUnHIDReportDescriptorMainItems : uint8
{
	Input = 0x8,
	Output = 0x9,
	Collection = 0xA,
	Feature = 0xB,
	EndCollection = 0xC,
	ReservedMin = 0xD,
	ReservedMax = 0xF
};

USTRUCT(BlueprintType)
struct FUnHIDDeviceDescriptorReportItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 BitOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 BitSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 UsagePage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<int64> Usage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 UsageMinimum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 UsageMaximum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 LogicalMinimum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 LogicalMaximum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 PhysicalMinimum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 PhysicalMaximum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 UnitExponent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int64 Unit = 0;
};

USTRUCT(BlueprintType)
struct FUnHIDDeviceDescriptorReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 ReportId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 NumBits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 NumBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<FUnHIDDeviceDescriptorReportItem> Items;
};

USTRUCT(BlueprintType)
struct FUnHIDDeviceDescriptorReports
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	bool bValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<FUnHIDDeviceDescriptorReport> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<FUnHIDDeviceDescriptorReport> Outputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<FUnHIDDeviceDescriptorReport> Features;
};

USTRUCT(BlueprintType)
struct FUnHIDDeviceInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	FString Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 VendorId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 ProductId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	FString SerialNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 ReleaseNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	FString Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	FString Product;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 UsagePage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 Usage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	int32 InterfaceNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	EUnHIDBusType BusType = EUnHIDBusType::Unknown;
};

/**
 *
 */
UCLASS()
class UNHID_API UUnHIDBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Enumerate"), Category = "UnHID")
	static TArray<FUnHIDDeviceInfo> UnHIDEnumerate();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID UsagePage To String"), Category = "UnHID")
	static FString UnHIDUsagePageToString(const int32 UsagePage);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID HexString to Int32"), Category = "UnHID")
	static int32 UnHIDHexStringToInt32(const FString& HexString);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID Get hidapi Version"), Category = "UnHID")
	static void UnHIDGetHidapiVersion(int32& Major, int32& Minor, int32& Patch);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID Get hidapi Version String"), Category = "UnHID")
	static FString UnHIDGetHidapiVersionString();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Get Report Descriptor"), Category = "UnHID")
	static TArray<uint8> UnHIDGetReportDescriptor(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID Bytes to HexString"), Category = "UnHID")
	static FString UnHIDBytesToHexString(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHID HexString to Bytes"), Category = "UnHID")
	static TArray<uint8> UnHIDHexStringToBytes(const FString& HexString);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Open Device"), Category = "UnHID")
	static UUnHIDDevice* UnHIDOpenDevice(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Open Device by Usage filter with HexStrings"), Category = "UnHID")
	static UUnHIDDevice* UnHIDOpenDeviceByUsageFilter(const int32 UsagePage, const int32 Usage, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Open Multiple Devices by Usage filter with HexStrings"), Category = "UnHID")
	static TArray<UUnHIDDevice*> UnHIDOpenDevicesByUsageFilter(const int32 UsagePage, const int32 Usage, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, TArray<FString>& ErrorMessages);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Open Device by Usage filter with HexStrings"), Category = "UnHID")
	static UUnHIDDevice* UnHIDOpenDeviceByUsageFilterHexStrings(const FString& UsagePageHexString, const FString& UsageHexString, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Open Multiple Devices by Usage filter with HexStrings"), Category = "UnHID")
	static TArray<UUnHIDDevice*> UnHIDOpenDevicesByUsageFilterHexStrings(const FString& UsagePageHexString, const FString& UsageHexString, const FUnHIDReadDynamicDelegate& InUnHIDReadDynamicDelegate, TArray<FString>& ErrorMessages);

	static UUnHIDDevice* UnHIDOpenDevice(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadNativeDelegate& InUnHIDReadNativeDelegate, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Get Reports from Report Descriptor Bytes"), Category = "UnHID")
	static FUnHIDDeviceDescriptorReports UnHIDGetReportsFromReportDescriptorBytes(const TArray<uint8>& UnHIDReportDescriptorBytes, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Set Virtual InputDevice Axis"), Category = "UnHID")
	static void UnHIDSetVitualInputDeviceAxis(const int32 ControllerId, const uint8 AxisId, const float Value);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Bitmask from Bytes"), Category = "UnHID")
	static TArray<bool> UnHIDParseBitmaskFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Unsigned Integer from Bytes"), Category = "UnHID")
	static int64 UnHIDParseUnsignedIntegerFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Signed Integer from Bytes"), Category = "UnHID")
	static int64 UnHIDParseSignedIntegerFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Analog from Bytes"), Category = "UnHID")
	static float UnHIDParseAnalogFromBytes(const TArray<uint8>& Bytes, const int64 BitOffset, const int64 BitSize, const int64 Minimum, const int64 Maximum, const float AnalogMin = -1, const float AnalogMax = 1);
    
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Get Serial Number String"), Category = "UnHID")
    static FString UnHIDGetSerialNumberString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage);
    
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Get Manufacturer String"), Category = "UnHID")
    static FString UnHIDGetManufacturerString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage);
    
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Get Product String"), Category = "UnHID")
    static FString UnHIDGetProductString(const FUnHIDDeviceInfo& UnHIDDeviceInfo, FString& ErrorMessage);
};
