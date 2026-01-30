#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "TelemetrySubsystem.generated.h"

/**
 * Game Instance Subsystem for sending telemetry to HTTP endpoint
 * Automatically managed by UGameInstance - no manual instantiation needed
 */
UCLASS()
class TELEMETRYPLUGIN_API UTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle - called automatically by engine
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Configure the server endpoint - call this in GameInstance BP */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void Configure(const FString& ServerURL);

	/** Start a new session - call when player spawns */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void StartNewSession();

	/** End current session - call on player death/level end */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void EndSession();

	/** Send position update - call from timer */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendPositionUpdate(FVector Position, float GameTime);

	/** Send damage event - call when player takes damage */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendDamageEvent(
		float DamageAmount,
		float HealthBefore,
		float HealthAfter,
		const FString& DamageSource,
		FVector Position,
		float GameTime
	);

	/** Send death event - call on player death */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendDeathEvent(FVector Position, float GameTime);

private:
	// Internal helper to send JSON to server
	void SendTelemetryEvent(const TSharedPtr<FJsonObject>& JsonData);

	// Create base telemetry object with common fields
	TSharedPtr<FJsonObject> CreateBaseTelemetryObject(const FString& EventType, float GameTime);

	// State
	FString ServerURL;
	FString CurrentSessionID;
	FString MachineName;
	int32 FrameCounter;

	static constexpr float RequestTimeout = 5.0f;
};
