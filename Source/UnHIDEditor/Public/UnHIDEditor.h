// Copyright 2026 - Roberto De Ioris

#pragma once

#include "Modules/ModuleManager.h"

class FUnHIDEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static TSharedRef<SDockTab> CreateUnHIDDashboard(const FSpawnTabArgs& Args);
	static TSharedRef<SDockTab> CreateUnHIDHUT(const FSpawnTabArgs& Args);
};
