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
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Configure the server endpoint - call this in GameInstance BP */
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(Keywords="start, config, configure, endpoint, telemetry"))
	void Configure(const FString& ServerURL);

	/** Start a new session*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords = "start session telemetry"))
	void StartNewSession();

	/** End current session*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="end session telemetry"))
	void EndSession();

	/** Send position update - call from a timer */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="position update location tracking"))
	void SendPositionUpdate(FVector Position, float GameTime);

	/** Send log when IA is called - call from input events */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="input action player control"))
	void SendPlayerInputAction(UInputAction* InputAction, float GameTime);

	/** Send damage event - call when player takes damage */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="damage health combat"))
	void SendDamageEvent(
		float DamageAmount,
		float HealthBefore,
		float HealthAfter,
		const FString& DamageSource,
		FVector Position,
		float GameTime
	);

	/** Death event*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="death player end"))
	void SendDeathEvent(FVector Position, float GameTime);

private:
	// Internal helper to send JSON to server
	void SendTelemetryEvent(const TSharedPtr<FJsonObject>& JsonData) const;

	// Create base telemetry object with common fields
	TSharedPtr<FJsonObject> CreateBaseTelemetryObject(const FString& EventType, float GameTime);
	TSharedPtr<FJsonObject> CreatePositionObject(FVector Position);
	bool IsTelemetryReady() const;

	// State
	FString ServerURL;
	FString CurrentSessionID;
	FString MachineName;
	FString UserName;
	int32 FrameCounter;

	static constexpr float RequestTimeout = 5.0f;
};
