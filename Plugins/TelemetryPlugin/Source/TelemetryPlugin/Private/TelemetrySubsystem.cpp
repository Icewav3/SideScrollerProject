#include "TelemetrySubsystem.h"
#include "HttpModule.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"

void UTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MachineName = FPlatformProcess::ComputerName();
	FrameCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Initialized on: %s"), *MachineName);
}

void UTelemetrySubsystem::Deinitialize()
{
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

	// End previous session if exists
	if (!CurrentSessionID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Ending previous session before starting new one"));
		EndSession();
	}

	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	CurrentSessionID = FString::Printf(TEXT("%s_%s"), *MachineName, *Timestamp);
	FrameCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Session started: %s"), *CurrentSessionID);

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("session_start"), 0.0f);
	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::EndSession()
{
	if (CurrentSessionID.IsEmpty())
	{
		return;
	}
	
	if (ServerURL.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Cannot end session - server not configured"));
		CurrentSessionID.Empty();
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Telemetry] Session ended: %s"), *CurrentSessionID);

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("session_end"), 0.0f);
	SendTelemetryEvent(EventData);

	CurrentSessionID.Empty();
}

void UTelemetrySubsystem::SendPositionUpdate(FVector Position, float GameTime)
{
	if (ServerURL.IsEmpty() || CurrentSessionID.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("position"), GameTime);

	TSharedPtr<FJsonObject> PosObject = MakeShareable(new FJsonObject);
	PosObject->SetNumberField(TEXT("x"), Position.X);
	PosObject->SetNumberField(TEXT("y"), Position.Y);
	PosObject->SetNumberField(TEXT("z"), Position.Z);
	EventData->SetObjectField(TEXT("player_pos"), PosObject);

	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::SendPlayerInputAction(UInputAction* InputAction, float GameTime)
{
	//todo
}

void UTelemetrySubsystem::SendPlayerInputMappingContextUpdate(UInputMappingContext* InputMappingContext, float GameTime)
{
	//todo
}

void UTelemetrySubsystem::SendDamageEvent(
	float DamageAmount,
	float HealthBefore,
	float HealthAfter,
	const FString& DamageSource,
	FVector Position,
	float GameTime)
{
	if (ServerURL.IsEmpty() || CurrentSessionID.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("damage"), GameTime);

	EventData->SetNumberField(TEXT("damage"), DamageAmount);
	EventData->SetNumberField(TEXT("health_before"), HealthBefore);
	EventData->SetNumberField(TEXT("health_after"), HealthAfter);
	EventData->SetStringField(TEXT("damage_source"), DamageSource);

	TSharedPtr<FJsonObject> PosObject = MakeShareable(new FJsonObject);
	PosObject->SetNumberField(TEXT("x"), Position.X);
	PosObject->SetNumberField(TEXT("y"), Position.Y);
	PosObject->SetNumberField(TEXT("z"), Position.Z);
	EventData->SetObjectField(TEXT("player_pos"), PosObject);

	SendTelemetryEvent(EventData);
}

void UTelemetrySubsystem::SendDeathEvent(FVector Position, float GameTime)
{
	//Todo abstract into method
	if (ServerURL.IsEmpty() || CurrentSessionID.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> EventData = CreateBaseTelemetryObject(TEXT("death"), GameTime);

	TSharedPtr<FJsonObject> PosObject = MakeShareable(new FJsonObject);
	PosObject->SetNumberField(TEXT("x"), Position.X);
	PosObject->SetNumberField(TEXT("y"), Position.Y);
	PosObject->SetNumberField(TEXT("z"), Position.Z);
	EventData->SetObjectField(TEXT("player_pos"), PosObject);

	SendTelemetryEvent(EventData);
}

TSharedPtr<FJsonObject> UTelemetrySubsystem::CreateBaseTelemetryObject(const FString& EventType, float GameTime)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("machine_id"), MachineName);
	JsonObject->SetStringField(TEXT("session_id"), CurrentSessionID);
	JsonObject->SetStringField(TEXT("event_type"), EventType);
	JsonObject->SetNumberField(TEXT("frame"), FrameCounter++);
	JsonObject->SetNumberField(TEXT("game_time"), GameTime);

	return JsonObject;
}

void UTelemetrySubsystem::SendTelemetryEvent(const TSharedPtr<FJsonObject>& JsonData)
{
	if (ServerURL.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Telemetry] Not configured - call Configure() first"));
		return;
	}

	// Serialize JSON
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonData.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("[Telemetry] Failed to serialize JSON"));
		return;
	}

	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);
	Request->SetTimeout(RequestTimeout);

	// Send asynchronously (fire and forget)
	Request->ProcessRequest();
}