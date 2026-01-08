// Copyright 2026 - Roberto De Ioris

#pragma once

#include "Modules/ModuleManager.h"
#include "Framework/Application/IInputProcessor.h"

class FUnHIDInputProcessor : public IInputProcessor
{
public:
	FUnHIDInputProcessor(TWeakPtr<class SUnHIDVirtualInputConsole> InUnHIDVirtualInputConsole);

	bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	
	bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	
	bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	
protected:
	TWeakPtr<SUnHIDVirtualInputConsole> UnHIDVirtualInputConsole = nullptr;
};

class FUnHIDEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static TSharedRef<SDockTab> CreateUnHIDDashboard(const FSpawnTabArgs& Args);
	static TSharedRef<SDockTab> CreateUnHIDHUT(const FSpawnTabArgs& Args);
	static TSharedRef<SDockTab> CreateUnHIDVirtualInputConsole(const FSpawnTabArgs& Args);
};
