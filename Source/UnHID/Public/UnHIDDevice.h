// Copyright 2026 - Roberto De Ioris

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnHIDDevice.generated.h"

DECLARE_DELEGATE_ThreeParams(FUnHIDReadNativeDelegate, UUnHIDDevice*, const TArray<uint8>&, const FString&);

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

struct hid_device_info;
namespace UnHID
{
	EUnHIDBusType ToUnHIDBusType(const int32 BusType);
	void FillDeviceInfo(const hid_device_info*, FUnHIDDeviceInfo& UnHIDDeviceInfo);
}

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnHID")
	TArray<int64> CollectionUsage;
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

/**
 *
 */
UCLASS(BlueprintType)
class UNHID_API UUnHIDDevice : public UObject
{
	GENERATED_BODY()

public:
	~UUnHIDDevice();
	bool Initialize(const FUnHIDDeviceInfo& UnHIDDeviceInfo, const FUnHIDReadNativeDelegate& InReadNativeDelegate, FString& ErrorMessage);
	void StopWorkerThread();
	void Terminate();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Write Bytes"), Category = "UnHID")
	bool WriteBytes(const TArray<uint8>& Bytes, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Write HexString"), Category = "UnHID")
	bool WriteHexString(const FString& HexString, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Get Feature Report Bytes"), Category = "UnHID")
	bool GetFeatureReportBytes(const uint8 ReportId, const int32 Size, TArray<uint8>& Bytes, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Get Feature Report HexString"), Category = "UnHID")
	bool GetFeatureReportHexString(const uint8 ReportId, const int32 Size, FString& HexString, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Set Feature Report Bytes"), Category = "UnHID")
	bool SetFeatureReportBytes(const TArray<uint8>& Bytes, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Set Feature Report HexString"), Category = "UnHID")
	bool SetFeatureReportHexString(const FString& HexString, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Get Serial Number String"), Category = "UnHID")
	FString GetSerialNumberString(FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHIDDevice Get Report Descriptor"), Category = "UnHID")
	TArray<uint8> GetReportDescriptor() const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Get Descriptor Reports"), Category = "UnHID")
	bool GetDescriptorReports(FUnHIDDeviceDescriptorReports& DeviceDescriptorReports, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHIDDevice Get Device Info"), Category = "UnHID")
	FUnHIDDeviceInfo GetDeviceInfo() const;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHIDDevice Get BitOffset and BitSize from DescriptorReports and Usage"), Category = "UnHID")
	bool GetBitOffsetAndSizeFromDescriptorReportsAndUsage(const int32 UsagePage, const int32 Usage, int64& BitOffset, int64& BitSize, FString& ErrorMessage);


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Analog from Bytes and Usage Checked"), Category = "UnHID")
	bool ParseAnalogFromBytesAndUsageChecked(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage, const float AnalogMin, const float AnalogMax, float& Value, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Analog from Bytes and Usage"), Category = "UnHID")
	float ParseAnalogFromBytesAndUsage(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage, const float AnalogMin = -1, const float AnalogMax = 1);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Unsigned Integer from Bytes and Usage Checked"), Category = "UnHID")
	bool ParseUnsignedIntegerFromBytesAndUsageChecked(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage, int64& Value, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Unsigned Integer from Bytes and Usage"), Category = "UnHID")
	int64 ParseUnsignedIntegerFromBytesAndUsage(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Signed Integer from Bytes and Usage Checked"), Category = "UnHID")
	bool ParseSignedIntegerFromBytesAndUsageChecked(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage, int64& Value, FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UnHID Parse Signed Integer from Bytes and Usage"), Category = "UnHID")
	int64 ParseSignedIntegerFromBytesAndUsage(const TArray<uint8>& Bytes, const int32 UsagePage, const int32 Usage);

protected:
	void* HidDevice = nullptr;

	class FUnHIDDeviceWorkerThread* UnHIDDeviceWorkerThread = nullptr;

	TSharedPtr<TArray<uint8>> ReportDescriptor;
	TSharedPtr<FUnHIDDeviceInfo> DeviceInfo;
	TSharedPtr<FUnHIDDeviceDescriptorReports> DescriptorReports;
};
