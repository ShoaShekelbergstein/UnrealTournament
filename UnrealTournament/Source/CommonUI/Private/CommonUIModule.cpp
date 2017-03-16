// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "ICommonUIModule.h"
#include "CommonUISettings.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif // WITH_EDITOR

/**
 * Implements the FCommonUIModule module.
 */
class FCommonUIModule
	: public ICommonUIModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void SetStreamableManagerGetter(const FStreamableManagerGetter& Getter) override;
	virtual FStreamableManager* GetStreamableManager() const override;

	virtual TAsyncLoadPriority GetLazyLoadPriority() const override;

private:
	FStreamableManagerGetter StreamableManagerGetter;
};

void FCommonUIModule::StartupModule()
{
#if WITH_EDITOR
	// Register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "CommonUI",
			NSLOCTEXT("CommonUIPlugin", "CommonUISettingsName", "Common UI Framework"),
			NSLOCTEXT("CommonUIPlugin", "CommonUISettingsDescription", "Configure Common UI Framework defaults."),
			GetMutableDefault<UCommonUISettings>()
		);
	}
#endif //WITH_EDITOR
}

void FCommonUIModule::ShutdownModule()
{
#if WITH_EDITOR
	// Unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "CommonUI");
	}
#endif //WITH_EDITOR
}

void FCommonUIModule::SetStreamableManagerGetter(const FStreamableManagerGetter& Getter)
{
	StreamableManagerGetter = Getter;
}

FStreamableManager* FCommonUIModule::GetStreamableManager() const
{
	return StreamableManagerGetter();
}

TAsyncLoadPriority FCommonUIModule::GetLazyLoadPriority() const
{
	return 0;
}

IMPLEMENT_MODULE(FCommonUIModule, CommonUI);

DEFINE_LOG_CATEGORY(LogCommonUI);