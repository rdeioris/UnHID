// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnHIDDevice.generated.h"

struct FUnHIDDeviceInfo;

DECLARE_DELEGATE_ThreeParams(FUnHIDReadNativeDelegate, UUnHIDDevice*, const TArray<uint8>&, const FString&);

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

protected:
	void* HidDevice = nullptr;

	class FUnHIDDeviceWorkerThread* UnHIDDeviceWorkerThread = nullptr;
};
