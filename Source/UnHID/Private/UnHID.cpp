// Copyright 2026 - Roberto De Ioris

#include "UnHID.h"

#define LOCTEXT_NAMESPACE "FUnHIDModule"

class FUnHIDInputDevice : public IInputDevice
{
public:
	FUnHIDInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) : MessageHandler(InMessageHandler)
	{

	}

	virtual ~FUnHIDInputDevice() {}

	virtual void Tick(float DeltaTime) override {}

	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
	{
		MessageHandler = InMessageHandler;
	}

	virtual void SendControllerEvents() override
	{
		for (const TPair<FName, TPair<int32, float>>& Pair : AxisSnapshot)
		{
			FPlatformUserId UserId = FPlatformMisc::GetPlatformUserForUserIndex(Pair.Value.Key);
			FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
			IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Pair.Value.Key, UserId, DeviceId);

			MessageHandler->OnControllerAnalog(Pair.Key, UserId, DeviceId, Pair.Value.Value);
		}

		for (const TPair<FName, int32>& Pair : ButtonPressSnapshot)
		{
			FPlatformUserId UserId = FPlatformMisc::GetPlatformUserForUserIndex(Pair.Value);
			FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
			IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Pair.Value, UserId, DeviceId);

			MessageHandler->OnControllerButtonPressed(Pair.Key, UserId, DeviceId, false);
		}

		for (const TPair<FName, int32>& Pair : ButtonReleaseSnapshot)
		{
			FPlatformUserId UserId = FPlatformMisc::GetPlatformUserForUserIndex(Pair.Value);
			FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
			IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Pair.Value, UserId, DeviceId);

			MessageHandler->OnControllerButtonReleased(Pair.Key, UserId, DeviceId, false);
		}

		AxisSnapshot.Empty();
		ButtonPressSnapshot.Empty();
		ButtonReleaseSnapshot.Empty();
	}

	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override {}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override {}
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }

	void SetAxis(const int32 ControllerId, const FName& Axis, const float Value)
	{
		TPair<int32, float>& Pair = AxisSnapshot.FindOrAdd(Axis);
		Pair.Key = ControllerId;
		Pair.Value = Value;
	}

	void ButtonPress(const int32 ControllerId, const FName& Button)
	{
		int32& FoundControllerId = ButtonPressSnapshot.FindOrAdd(Button);
		FoundControllerId = ControllerId;
	}

	void ButtonRelease(const int32 ControllerId, const FName& Button)
	{
		int32& FoundControllerId = ButtonReleaseSnapshot.FindOrAdd(Button);
		FoundControllerId = ControllerId;
	}

protected:
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	TMap<FName, TPair<int32, float>> AxisSnapshot;
	TMap<FName, int32> ButtonPressSnapshot;
	TMap<FName, int32> ButtonReleaseSnapshot;
};

#if PLATFORM_MAC
extern "C"
{
	void hid_darwin_set_open_exclusive(int32 OpenExclusive);
}
#endif

void FUnHIDModule::StartupModule()
{
#if PLATFORM_MAC
	hid_darwin_set_open_exclusive(0);
#endif
	IInputDeviceModule::StartupModule();

	const FName NAME_UnHID(TEXT("UnHID"));

	EKeys::AddMenuCategoryDisplayInfo(NAME_UnHID, FText::FromString("UnHID"), TEXT("GraphEditor.KeyEvent_16x"));

	for (uint8 Index = 0; Index < 128; Index++)
	{
		const FName AxisName = *FString::Printf(TEXT("UnHID_Axis%u"), Index);
		EKeys::AddKey(FKeyDetails(AxisName, FText::FromString(FString::Printf(TEXT("UnHID Axis %u"), Index)), FKeyDetails::Axis1D, NAME_UnHID));
		AxisCache.Add(Index, AxisName);
	}
	for (uint8 Index = 0; Index < 128; Index++)
	{
		const FName ButtonName = *FString::Printf(TEXT("UnHID_Button%u"), Index);
		EKeys::AddKey(FKeyDetails(ButtonName, FText::FromString(FString::Printf(TEXT("UnHID Button %u"), Index)), FKeyDetails::GamepadKey, NAME_UnHID));
		ButtonCache.Add(Index, ButtonName);
	}
}

void FUnHIDModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

TSharedPtr<IInputDevice> FUnHIDModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	UnHIDInputDevice = MakeShared<FUnHIDInputDevice>(InMessageHandler);
	return UnHIDInputDevice;
}

void FUnHIDModule::VirtualInputDeviceSetAxis(const int32 ControllerId, const uint8 AxisId, const float Value)
{
	if (AxisId > 127)
	{
		return;
	}
	UnHIDInputDevice->SetAxis(ControllerId, AxisCache[AxisId], Value);
}

void FUnHIDModule::VirtualInputDeviceButtonPress(const int32 ControllerId, const uint8 ButtonId)
{
	if (ButtonId > 127)
	{
		return;
	}
	UnHIDInputDevice->ButtonPress(ControllerId, ButtonCache[ButtonId]);
}

void FUnHIDModule::VirtualInputDeviceButtonRelease(const int32 ControllerId, const uint8 ButtonId)
{
	if (ButtonId > 127)
	{
		return;
	}
	UnHIDInputDevice->ButtonRelease(ControllerId, ButtonCache[ButtonId]);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnHIDModule, UnHID)
