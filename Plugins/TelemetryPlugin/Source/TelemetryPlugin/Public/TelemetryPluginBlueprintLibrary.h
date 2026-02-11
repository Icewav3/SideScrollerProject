#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TelemetrySubsystem.h"
#include "TelemetryPluginBlueprintLibrary.generated.h"

class UInputAction;

UCLASS()
class TELEMETRYPLUGIN_API UTelemetryBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Configure telemetry - call once at game start */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="config setup start telemetry"))
	static void ConfigureTelemetry(const UObject* WorldContextObject, const FString& ServerURL);

	/** Start new session */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="start begin session telemetry"))
	static void StartSession(const UObject* WorldContextObject);

	/** End current session */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="end stop session telemetry"))
	static void EndSession(const UObject* WorldContextObject);

	/** Log position update */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="position location telemetry"))
	static void LogPosition(const UObject* WorldContextObject, FVector Position, float GameTime);

	/** Log input action */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="input action telemetry"))
	static void LogInputAction(const UObject* WorldContextObject, UInputAction* InputAction, float GameTime);

	/** Log damage event */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="damage hurt health telemetry"))
	static void LogDamage(
		const UObject* WorldContextObject,
		float DamageAmount,
		float HealthBefore,
		float HealthAfter,
		const FString& DamageSource,
		FVector Position,
		float GameTime
	);

	/** Log death event */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="death die killed telemetry"))
	static void LogDeath(const UObject* WorldContextObject, FVector Position, float GameTime);

private:
	static UTelemetrySubsystem* GetTelemetrySubsystem(const UObject* WorldContextObject);
};
