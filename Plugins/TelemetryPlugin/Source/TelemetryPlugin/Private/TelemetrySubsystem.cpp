#include "TelemetrySubsystem.h"
#include "HttpModule.h"
#include "InputAction.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"

void UTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MachineName = FPlatformProcess::ComputerName();
	UserName = FPlatformProcess::UserName();
	FrameCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Initialized on: %s under username: %s"), *MachineName, *UserName);
}

void UTelemetrySubsystem::Deinitialize()
{
	// Auto-cleanup: End run if active
	if (CurrentRunData.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Auto-ending active run on deinitialize"));
		EndRun(TEXT("shutdown"));
	}

	// Auto-cleanup: End session if active
	if (!CurrentSessionID.IsEmpty())
	{
		EndSession();
	}

	Super::Deinitialize();
}

void UTelemetrySubsystem::Configure(const FString& InServerURL)
{
	ServerURL = InServerURL.IsEmpty() ? TEXT("http://10.20.5.27:8080/telemetry") : InServerURL;
	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Configured server: %s"), *ServerURL);
}

void UTelemetrySubsystem::StartNewSession()
{
	if (ServerURL.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[Telemetry] Cannot start session - Configure() not called yet!"));
		return;
	}

	// Auto-end previous session if exists
	if (!CurrentSessionID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Ending previous session before starting new one"));
		EndSession();
	}

	// Generate unique session ID
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	CurrentSessionID = FString::Printf(TEXT("%s_%s"), *MachineName, *Timestamp);
	FrameCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Session started: %s"), *CurrentSessionID);

	// Send session_start event
	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("session_start"), 0.0f);
	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::EndSession()
{
	if (CurrentSessionID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Cannot end session - no active session"));
		return;
	}

	// Auto-end active run before ending session
	if (CurrentRunData.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Auto-ending active run before session end"));
		EndRun(TEXT("session_end"));
	}

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Session ended: %s"), *CurrentSessionID);

	// Send session_end event
	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("session_end"), 0.0f);
	SendTelemetryEvent(EventData);

	CurrentSessionID.Empty();
}

void UTelemetrySubsystem::StartRun()
{
	if (!IsSessionActive())
	{
		UE_LOG(LogTemp, Error, TEXT("[Telemetry] Cannot start run - no active session. Call StartNewSession() first."));
		return;
	}

	// Auto-end previous run if exists
	if (CurrentRunData.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Ending previous run before starting new one"));
		EndRun(TEXT("new_run_started"));
	}

	// Get current game time
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Generate unique run ID
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S_%f"));
	CurrentRunData.RunID = FString::Printf(TEXT("%s_run_%s"), *CurrentSessionID, *Timestamp);
	CurrentRunData.RunStartTime = CurrentTime;
	CurrentRunData.RunEndTime = 0.0f;  // 0 indicates active run
	CurrentRunData.RunTotalTime = 0.0f;
	CurrentRunData.EndReason.Empty();

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Run started: %s at time %.2f"), *CurrentRunData.RunID, CurrentTime);

	// Send run_start event
	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("run_start"), CurrentTime);
	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::EndRun(const FString& Reason)
{
	if (!CurrentRunData.IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Cannot end run - no active run"));
		return;
	}

	// Get current game time
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Finalize run data
	CurrentRunData.RunEndTime = CurrentTime;
	CurrentRunData.RunTotalTime = CurrentRunData.RunEndTime - CurrentRunData.RunStartTime;
	CurrentRunData.EndReason = Reason;

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Run ended: %s | Reason: %s | Duration: %.2fs | Rooms: %d"), 
		*CurrentRunData.RunID, *Reason, CurrentRunData.RunTotalTime, CurrentRunData.RoomsCleared);

	// Send run_end event (includes final run data)
	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("run_end"), CurrentTime);
	SendTelemetryEvent(EventData);

	// Clear run data
	CurrentRunData = FTelemetryRunData();
}

void UTelemetrySubsystem::SendPositionUpdate(FVector Position, float GameTime)
{
	if (!IsTelemetryReady())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("position"), GameTime);
	EventData->SetObjectField(TEXT("player_pos"), CreatePositionObject(Position));

	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::SendPlayerInputAction(UInputAction* InputAction, float GameTime)
{
	if (!IsTelemetryReady())
	{
		return;
	}

	if (!InputAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] SendPlayerInputAction called with null InputAction"));
		return;
	}
	
	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("input_received"), GameTime);

	TSharedPtr<FJsonObject> IAObject = MakeShareable(new FJsonObject);
	IAObject->SetStringField(TEXT("action_name"), InputAction->GetName());
	EventData->SetObjectField(TEXT("input_action"), IAObject);

	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::SendDamageEvent(
	float DamageAmount,
	float HealthBefore,
	float HealthAfter,
	const FString& DamageSource,
	FVector Position,
	float GameTime)
{
	if (!IsTelemetryReady())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("damage"), GameTime);

	EventData->SetNumberField(TEXT("damage"), DamageAmount);
	EventData->SetNumberField(TEXT("health_before"), HealthBefore);
	EventData->SetNumberField(TEXT("health_after"), HealthAfter);
	EventData->SetStringField(TEXT("damage_source"), DamageSource);
	EventData->SetObjectField(TEXT("player_pos"), CreatePositionObject(Position));

	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::SendDeathEvent(const FString& Cause, FVector Position, float GameTime)
{
	if (!IsTelemetryReady())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("death"), GameTime);
	EventData->SetObjectField(TEXT("player_pos"), CreatePositionObject(Position));

	SendTelemetryEvent(EventData);
}

TSharedPtr<FJsonObject> UTelemetrySubsystem::CreateBaseTelemetryObject(const FString& EventType, float GameTime)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Base fields
	JsonObject->SetStringField(TEXT("machine_id"), MachineName);
	JsonObject->SetStringField(TEXT("session_id"), CurrentSessionID);
	JsonObject->SetStringField(TEXT("event_type"), EventType);
	JsonObject->SetNumberField(TEXT("frame"), FrameCounter++);
	JsonObject->SetNumberField(TEXT("game_time"), GameTime);

	return JsonObject;
}

TSharedPtr<FJsonObject> UTelemetrySubsystem::CreatePositionObject(FVector Position)
{
	TSharedPtr<FJsonObject> PosObject = MakeShareable(new FJsonObject);
	PosObject->SetNumberField(TEXT("x"), Position.X);
	PosObject->SetNumberField(TEXT("y"), Position.Y);
	PosObject->SetNumberField(TEXT("z"), Position.Z);
	return PosObject;
}

bool UTelemetrySubsystem::IsTelemetryReady() const
{
	if (ServerURL.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[Telemetry] Cannot send events - server URL not configured"));
		return false;
	}

	if (CurrentSessionID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Cannot send events - no active session"));
		return false;
	}

	return true;
}

void UTelemetrySubsystem::SendTelemetryEvent(const TSharedPtr<FJsonObject>& JsonData) const
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("[Telemetry] Failed to serialize JSON"));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);
	Request->SetTimeout(RequestTimeout);
	Request->ProcessRequest();
}
