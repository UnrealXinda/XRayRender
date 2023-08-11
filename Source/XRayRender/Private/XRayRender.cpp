// Fill out your copyright notice in the Description page of Project Settings.

#include "XRayRender.h"

#include "ISettingsModule.h"
#include "XRayRenderSettings.h"
#include "XRayViewExtension.h"

#define LOCTEXT_NAMESPACE "FXRayRenderModule"

void FXRayRenderModule::StartupModule()
{
	[[maybe_unused]] FXRayViewExtension& Extension = FXRayViewExtension::Get();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"Plugins",
			"XRayRenderSettings",
			LOCTEXT("RuntimeSettingsName", "X-Ray Render Settings"),
			LOCTEXT("RuntimeSettingsDescription", "Configure X-Ray Render setting"),
			GetMutableDefault<UXRayRenderSettings>());
	}
}

void FXRayRenderModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings(
			"Project",
			"Plugins",
			"XRayRenderSettings");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FXRayRenderModule, XRayRender)