#include "TelemetryPlugin.h"

#define LOCTEXT_NAMESPACE "FTelemetryPluginModule"

void FTelemetryPluginModule::StartupModule()
{
	// This is called when your plugin is loaded
	// We don't need any special initialization
	UE_LOG(LogTemp, Log, TEXT("[TelemetryPlugin] Module loaded"));
}

void FTelemetryPluginModule::ShutdownModule()
{
	// This is called when your plugin is unloaded
	UE_LOG(LogTemp, Log, TEXT("[TelemetryPlugin] Module unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTelemetryPluginModule, TelemetryPlugin)
