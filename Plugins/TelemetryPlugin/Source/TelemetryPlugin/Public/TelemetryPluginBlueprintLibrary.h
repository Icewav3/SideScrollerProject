#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TelemetrySubsystem.h"
#include "TelemetryPluginBlueprintLibrary.generated.h"

class UInputAction;

/**
 * Blueprint Function Library for Telemetry
 * Provides convenient static Blueprint nodes that wrap the TelemetrySubsystem
 * All functions automatically resolve the TelemetrySubsystem from WorldContext
 */
UCLASS()
class TELEMETRYPLUGIN_API UTelemetryBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 
	 * Configure telemetry server endpoint
	 * Call once at game start before using any other telemetry functions
	 * @param ServerURL - Full HTTP endpoint (e.g., "http://10.20.5.27:8080/telemetry")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="config setup start telemetry"))
	static void ConfigureTelemetry(const UObject* WorldContextObject, const FString& ServerURL);

	/** 
	 * Start new telemetry session
	 * Call once when game instance starts
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="start begin session telemetry"))
	static void StartSession(const UObject* WorldContextObject);

	/** 
	 * End current telemetry session
	 * Call when game instance shuts down
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="end stop session telemetry"))
	static void EndSession(const UObject* WorldContextObject);

	/** 
	 * Start new gameplay run
	 * Call when player spawns/respawns
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="start run telemetry"))
	static void StartRun(const UObject* WorldContextObject);

	/** 
	 * End current gameplay run
	 * Call when player dies or completes level
	 * @param Reason - Why the run ended (e.g., "death", "victory", "quit")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="end run telemetry"))
	static void EndRun(const UObject* WorldContextObject, const FString& Reason);

	/** 
	 * Increment rooms cleared counter
	 * Call when player clears a room/arena
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="room cleared progress telemetry"))
	static void IncrementRoomsCleared(const UObject* WorldContextObject);

	/** 
	 * Log position update
	 * Typically called on a timer for player tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="position location telemetry"))
	static void LogPosition(const UObject* WorldContextObject, FVector Position, float GameTime);

	/** 
	 * Log input action
	 * Call when player triggers an input action
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="input action telemetry"))
	static void LogInputAction(const UObject* WorldContextObject, UInputAction* InputAction, float GameTime);

	/** 
	 * Log damage event
	 * Call when player takes damage
	 */
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

	/** 
	 * Log death event
	 * Call when player dies
	 * @param Cause - What killed the player (e.g., "enemy", "fall_damage", "lava")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(WorldContext="WorldContextObject", Keywords="death die killed telemetry"))
	static void LogDeath(const UObject* WorldContextObject, const FString& Cause, FVector Position, float GameTime);

private:
	/** Internal helper to get telemetry subsystem from world context */
	static UTelemetrySubsystem* GetTelemetrySubsystem(const UObject* WorldContextObject);
};