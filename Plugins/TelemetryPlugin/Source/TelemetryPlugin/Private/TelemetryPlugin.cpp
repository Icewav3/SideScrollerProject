#include "TelemetryPlugin.h"

#define LOCTEXT_NAMESPACE "FTelemetryPluginModule"

void FTelemetryPluginModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("[TelemetryPlugin] Module loaded"));
}

void FTelemetryPluginModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("[TelemetryPlugin] Module unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTelemetryPluginModule, TelemetryPlugin)
