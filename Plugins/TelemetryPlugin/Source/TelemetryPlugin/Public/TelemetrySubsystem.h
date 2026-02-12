#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "TelemetryTypes.h"
#include "TelemetrySubsystem.generated.h"

/**
 * Game Instance Subsystem for sending telemetry to HTTP endpoint
 * Automatically managed by UGameInstance - no manual instantiation needed
 * DATA STRUCTURE:
 * Each event contains:
 * - Base fields: machine_id, session_id, event_type, frame, game_time
 * - Run data: run_id, run_start_time, run_end_time, run_total_time
 * - Event-specific fields: position, damage, input, etc.
 */
UCLASS()
class TELEMETRYPLUGIN_API UTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Configure the server endpoint - call this in GameInstance*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry",
		meta=(Keywords="start setup config configure endpoint telemetry"))
	void Configure(const FString& ServerURL);

	/** Start a new session - call on startup (will prob need delay)*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords = "start session telemetry"))
	void StartNewSession();

	/** 
	 * End the current telemetry session
	 * Call when game instance shuts down
	 * Automatically ends any active run
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", 
		meta=(Keywords="end stop session telemetry"))
	void EndSession();

	/** Start a new run - call on player respawn
	* automatically ends current run
	*/
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="start run telemetry"))
	void StartRun();
	
	/** 
	 * End the current gameplay run
	 * Call when player dies or completes level
	 * @param Reason - Why the run ended (e.g., "death", "victory", "quit")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", 
		meta=(Keywords="end run telemetry"))
	void EndRun(const FString& Reason);

	/** Send position update - call from a timer */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", meta=(Keywords="position update location tracking"))
	void SendPositionUpdate(FVector Position, float GameTime);

	/** 
	 * Send input action event
	 * Call when player triggers an input action
	 * @param InputAction - The UInputAction that was triggered
	 * @param GameTime - Current game time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", 
		meta=(Keywords="input action player control"))
	void SendPlayerInputAction(UInputAction* InputAction, float GameTime);

	/** 
	 * Send damage event
	 * Call when player takes damage
	 * @param DamageAmount - Amount of damage dealt
	 * @param HealthBefore - Health before damage
	 * @param HealthAfter - Health after damage
	 * @param DamageSource - Name/type of damage source
	 * @param Position - Where damage occurred
	 * @param GameTime - Current game time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", 
		meta=(Keywords="damage health combat"))
	void SendDamageEvent(
		//TODO once verifying this is working, remove health after
		float DamageAmount,
		float HealthBefore,
		float HealthAfter,
		const FString& DamageSource,
		FVector Position,
		float GameTime
	);

	/** 
	 * Send death event
	 * Call when player dies
	 * @param Cause - What killed the player
	 * @param Position - Where death occurred
	 * @param GameTime - Current game time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry", 
		meta=(Keywords="death player end"))
	void SendDeathEvent(const FString& Cause, FVector Position, float GameTime);

private:
	/** Send JSON telemetry event to server */
	void SendTelemetryEvent(const TSharedPtr<FJsonObject>& JsonData) const;

	/** Create base telemetry object with common fields and run data */
	TSharedPtr<FJsonObject> CreateBaseTelemetryObject(const FString& EventType, float GameTime);
	
	/** Create position JSON object from vector */
	TSharedPtr<FJsonObject> CreatePositionObject(FVector Position);
	
	/** Check if telemetry is ready to send events */
	bool IsTelemetryReady() const;

	// State Variables
	
	/** HTTP endpoint for telemetry server */
	FString ServerURL;
	
	/** Current session identifier (machine_timestamp) */
	FString CurrentSessionID;
	
	/** Machine name (computer name) */
	FString MachineName;
	
	/** User name (OS username) */
	FString UserName;
	
	/** Current run data (active gameplay attempt) */
	FTelemetryRunData CurrentRunData;
	
	/** Frame counter for event ordering */
	int32 FrameCounter;

	/** HTTP request timeout in seconds */
	static constexpr float RequestTimeout = 5.0f;
};