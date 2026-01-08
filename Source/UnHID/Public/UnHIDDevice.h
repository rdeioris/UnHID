// Fill out your copyright notice in the Description page of Project Settings.

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
    
    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "UnHIDDevice Get Device Info"), Category = "UnHID")
    FUnHIDDeviceInfo GetDeviceInfo() const;

protected:
	void* HidDevice = nullptr;

	class FUnHIDDeviceWorkerThread* UnHIDDeviceWorkerThread = nullptr;
    
    TSharedPtr<TArray<uint8>> ReportDescriptor;
    TSharedPtr<struct FUnHIDDeviceInfo> DeviceInfo;
};
