#include "TelemetryPluginBlueprintLibrary.h"
#include "TelemetrySubsystem.h"
#include "Engine/GameInstance.h"

UTelemetrySubsystem* UTelemetryBlueprintLibrary::GetTelemetrySubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TelemetryBP] GetTelemetrySubsystem called with null WorldContextObject"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TelemetryBP] Failed to get World from context object"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TelemetryBP] Failed to get GameInstance"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UTelemetrySubsystem>();
}

void UTelemetryBlueprintLibrary::ConfigureTelemetry(const UObject* WorldContextObject, const FString& ServerURL)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->Configure(ServerURL);
	}
}

void UTelemetryBlueprintLibrary::StartSession(const UObject* WorldContextObject)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->StartNewSession();
	}
}

void UTelemetryBlueprintLibrary::EndSession(const UObject* WorldContextObject)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->EndSession();
	}
}

void UTelemetryBlueprintLibrary::StartRun(const UObject* WorldContextObject)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->StartRun();
	}
}

void UTelemetryBlueprintLibrary::EndRun(const UObject* WorldContextObject, const FString& Reason)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->EndRun(Reason);
	}
}

void UTelemetryBlueprintLibrary::LogPosition(const UObject* WorldContextObject, FVector Position, float GameTime)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->SendPositionUpdate(Position, GameTime);
	}
}

void UTelemetryBlueprintLibrary::LogInputAction(const UObject* WorldContextObject, UInputAction* InputAction, float GameTime)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->SendPlayerInputAction(InputAction, GameTime);
	}
}

void UTelemetryBlueprintLibrary::LogDamage(
	const UObject* WorldContextObject,
	float DamageAmount,
	float HealthBefore,
	float HealthAfter,
	const FString& DamageSource,
	FVector Position,
	float GameTime)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->SendDamageEvent(DamageAmount, HealthBefore, HealthAfter, DamageSource, Position, GameTime);
	}
}

void UTelemetryBlueprintLibrary::LogDeath(const UObject* WorldContextObject, const FString& Cause, FVector Position, float GameTime)
{
	if (UTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject))
	{
		Telemetry->SendDeathEvent(Cause, Position, GameTime);
	}
}