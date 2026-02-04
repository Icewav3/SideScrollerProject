#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "TelemetrySubsystem.generated.h"

/**
 * Game Instance Subsystem for sending telemetry to HTTP endpoint
 * Automatically managed by UGameInstance - no manual instantiation needed
 */
//TODO add intuitive keywords for all blueprint callable methods
UCLASS()
class TELEMETRYPLUGIN_API UTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Configure the server endpoint - call this in GameInstance BP */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void Configure(const FString& ServerURL);

	/** Start a new session*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void StartNewSession();

	/** End current session*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void EndSession();

	/** Send position update - call from a timer */
	// NOTE: Could probably remove by having it private and called when other logging methods are called instead
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendPositionUpdate(FVector Position, float GameTime);
	
	/** Send log when IA is called - call from input events */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendPlayerInputAction(UInputAction* InputAction, float GameTime); //not sure should pass pos here

	/** Send log for when IMC changes - idk where to call this tbh */
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void SendPlayerInputMappingContextUpdate(UInputMappingContext* InputMappingContext, float GameTime);

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

	/** Death event*/
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
