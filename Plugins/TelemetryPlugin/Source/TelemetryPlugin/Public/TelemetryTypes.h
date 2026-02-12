// TelemetryTypes.h
#pragma once

#include "CoreMinimal.h"
#include "TelemetryTypes.generated.h"

/**
 * Run data that tracks individual gameplay attempts within a session
 * Embedded in all telemetry events to provide run context
 */
USTRUCT(BlueprintType)
struct TELEMETRYPLUGIN_API FTelemetryRunData
{
	GENERATED_BODY()
	
	/** Unique identifier for this run */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString RunID;
	
	/** When this run started (game time) */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float RunStartTime;
	
	/** When this run ended (game time) - 0 if still active */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float RunEndTime;
	
	/** Total duration of the run */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float RunTotalTime;
	
	/** Why the run ended (e.g., "death", "quit", "victory") */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString EndReason;
	

	FTelemetryRunData()
		: RunStartTime(0.0f)
		, RunEndTime(0.0f)
		, RunTotalTime(0.0f)
		, EndReason(TEXT(""))
	{
	}

	/** Convert to JSON object for embedding in telemetry events */
	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("run_id"), RunID);
		JsonObject->SetNumberField(TEXT("run_start_time"), RunStartTime);
		JsonObject->SetNumberField(TEXT("run_end_time"), RunEndTime);
		JsonObject->SetNumberField(TEXT("run_total_time"), RunTotalTime);
		JsonObject->SetStringField(TEXT("end_reason"), EndReason);
		return JsonObject;
	}

	/** Check if run is currently active */
	bool IsActive() const
	{
		return !RunID.IsEmpty() && RunEndTime == 0.0f;
	}
};

/**
 * Context object that captures common telemetry data automatically
 * Reduces parameter duplication across telemetry calls
 */
USTRUCT(BlueprintType)
struct TELEMETRYPLUGIN_API FTelemetryContext
{
	GENERATED_BODY()
	
	/** Position where event occurred */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FVector Position;
	
	/** Game time when event occurred */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float GameTime;
	
	/** Actor that triggered the event */
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	AActor* SourceActor;

	FTelemetryContext()
		: Position(FVector::ZeroVector)
		, GameTime(0.0f)
		, SourceActor(nullptr)
	{
	}

	/** 
	 * Auto-populate context from an actor
	 * Automatically extracts position and game time
	 */
	static FTelemetryContext FromActor(AActor* Actor)
	{
		FTelemetryContext Context;
		if (Actor)
		{
			Context.Position = Actor->GetActorLocation();
			Context.SourceActor = Actor;
			
			if (UWorld* World = Actor->GetWorld())
			{
				Context.GameTime = World->GetTimeSeconds();
			}
		}
		return Context;
	}

	/** Check if context is valid */
	bool IsValid() const
	{
		return SourceActor != nullptr;
	}
};