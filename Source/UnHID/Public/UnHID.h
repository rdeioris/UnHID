// Copyright 2026 - Roberto De Ioris

#pragma once

#include "Modules/ModuleManager.h"
#include "IInputDeviceModule.h"

class FUnHIDModule : public IInputDeviceModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** IInputDeviceModule implementation */
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	static FUnHIDModule& Get()
	{
		static FUnHIDModule& Singleton = FModuleManager::LoadModuleChecked<FUnHIDModule>("UnHID");
		return Singleton;
	}

	void SetVitualInputDeviceAxis(const int32 ControllerId, const uint8 AxisId, const float Value);

protected:

	TSharedPtr<class FUnHIDInputDevice> UnHIDInputDevice;

	TMap<uint8, FName> AxisCache;
	TMap<uint8, FName> ButtonCache;
};
