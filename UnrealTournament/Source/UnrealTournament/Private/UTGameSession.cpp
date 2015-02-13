// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTGameSession.h"
#include "Online.h"
#include "OnlineSubsystemTypes.h"
#include "UTOnlineGameSettingsBase.h"
#include "UTBaseGameMode.h"
#include "UTLobbyGameMode.h"

AUTGameSession::AUTGameSession(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void AUTGameSession::Destroyed()
{
	Super::Destroyed();
	CleanUpOnlineSubsystem();
}

void AUTGameSession::InitOptions( const FString& Options )
{
	Super::InitOptions(Options);

	// Cache the GameMode for later.
	UTGameMode = Cast<AUTBaseGameMode>(GetWorld()->GetAuthGameMode());
}

void AUTGameSession::ValidatePlayer(const FString& Address, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	UNetDriver* NetDriver = NULL;
	if (GetWorld())
	{
		NetDriver = GetWorld()->GetNetDriver();
	}

	FString LocalAddress = NetDriver->LowLevelGetNetworkNumber();

	if (!UniqueId.IsValid() && Address != TEXT("127.0.0.1") && !FParse::Param(FCommandLine::Get(), TEXT("AllowEveryone")) && Cast<AUTLobbyGameMode>(UTGameMode) == NULL)
	{
		ErrorMessage = TEXT("NOTLOGGEDIN");
	}
}


FString AUTGameSession::ApproveLogin(const FString& Options)
{
	if (UTGameMode)
	{
		if (!UTGameMode->HasOption(Options, TEXT("VersionCheck")) && (GetNetMode() != NM_Standalone) && !GetWorld()->IsPlayInEditor())
		{
			UE_LOG(UT, Warning, TEXT("********************************YOU MUST UPDATE TO A NEW VERSION %s"), *Options);
			return TEXT("You must update to a the latest version.  For more information, go to forums.unrealtournament.com");
		}
		if (UTGameMode->bRequirePassword && !UTGameMode->ServerPassword.IsEmpty())
		{
			FString Password = UTGameMode->ParseOption(Options, TEXT("Password"));
			if (Password.IsEmpty() || Password != UTGameMode->ServerPassword)
			{
				return TEXT("NEEDPASS");
			}
		}
	}

	return Super::ApproveLogin(Options);
}


void AUTGameSession::CleanUpOnlineSubsystem()
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
		}

	}
}

bool AUTGameSession::ProcessAutoLogin()
{
	if (GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		//RegisterServer();
		return true;
	}
	return Super::ProcessAutoLogin();
}


void AUTGameSession::RegisterServer()
{
	UE_LOG(UT,Verbose,TEXT("--------------[REGISTER SERVER]----------------"));

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			EOnlineSessionState::Type State = SessionInterface->GetSessionState(FName(TEXT("Game")));
			if (State == EOnlineSessionState::NoSession)
			{
				OnCreateSessionCompleteDelegate = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AUTGameSession::OnCreateSessionComplete));
			}
			else if (State == EOnlineSessionState::Ended)
			{
				StartMatch();
			}

			TSharedPtr<class FUTOnlineGameSettingsBase> OnlineGameSettings = MakeShareable(new FUTOnlineGameSettingsBase(false, false, 10000));
			if (OnlineGameSettings.IsValid() && UTGameMode)
			{
				InitHostBeacon(OnlineGameSettings.Get());
				OnlineGameSettings->ApplyGameSettings(UTGameMode);
				SessionInterface->CreateSession(0, GameSessionName, *OnlineGameSettings);
				return;
			}
		}
	}
}

void AUTGameSession::UnRegisterServer(bool bShuttingDown)
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	const auto SessionInterface = OnlineSub->GetSessionInterface();
	EOnlineSessionState::Type State = SessionInterface->GetSessionState(FName(TEXT("Game")));

	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		DestroyHostBeacon();
		if (bShuttingDown)
		{
			if (SessionInterface.IsValid())
			{
				OnDestroySessionCompleteDelegate = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &AUTGameSession::OnDestroySessionComplete));
				SessionInterface->DestroySession(GameSessionName);
			}
		}
	}
}

void AUTGameSession::StartMatch()
{

	UE_LOG(UT,Log,TEXT("--------------[MCP START MATCH] ----------------"));
	UE_LOG(UT,Log,TEXT("--------------[MCP START MATCH] ----------------"));

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			OnStartSessionCompleteDelegate = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &AUTGameSession::OnStartSessionComplete));
			SessionInterface->StartSession(GameSessionName);
		}
	}
}

void AUTGameSession::EndMatch()
{
	UE_LOG(UT,Log,TEXT("--------------[MCP END MATCH] ----------------"));
	UE_LOG(UT,Log,TEXT("--------------[MCP END MATCH] ----------------"));

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			OnEndSessionCompleteDelegate = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &AUTGameSession::OnEndSessionComplete));
			SessionInterface->EndSession(GameSessionName);
		}
	}
	
}


void AUTGameSession::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// If we were not successful, then clear the online game settings member and move on
	if (bWasSuccessful)
	{
		UpdateSessionJoinability(FName(TEXT("Game")), true, true, true, false);
		// Immediately start the online session
		StartMatch();
	}
	else
	{
		UE_LOG(UT,Log,TEXT("Failed to Create the session '%s' so this match will not be visible.  See the logs!"), *SessionName.ToString());
	}


	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
		}
	}

}

void AUTGameSession::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(UT,Log,TEXT("Failed to start the session '%s' so this match will not be visible.  See the logs!"), *SessionName.ToString());
	}
	else
	{
		// Our session has started, if we are a lobby instance, tell the lobby to go.  NOTE: We don't use the cached version of UTGameMode here
		AUTGameMode* GM = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (GM && GM->IsGameInstanceServer())
		{
			GM->NotifyLobbyGameIsReady();
		}
	
	}

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			UpdateGameState();		// Immediately perform an update so as to pickup any players that have joined since.
		}
	}


}

void AUTGameSession::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(UT,Log,TEXT("Failed to end the session '%s' so match stats will not save.  See the logs!"), *SessionName.ToString());
		IOnlineSubsystem::Get()->GetSessionInterface()->DumpSessionState();
	}

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
		}
	}
}

void AUTGameSession::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(UT,Log,TEXT("Failed to destroy the session '%s'!  Matchmaking will be broken until restart.  See the logs!"), *SessionName.ToString());
	}

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
		}
	}
}
void AUTGameSession::UpdateGameState()
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (UTGameMode && OnlineSub && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		const auto SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			EOnlineSessionState::Type State = SessionInterface->GetSessionState(GameSessionName);
			if (State != EOnlineSessionState::Creating && State != EOnlineSessionState::Ended && State != EOnlineSessionState::Ending && State != EOnlineSessionState::Destroying && State != EOnlineSessionState::NoSession )
			{
				FUTOnlineGameSettingsBase* OGS = (FUTOnlineGameSettingsBase*)SessionInterface->GetSessionSettings(GameSessionName);

				OGS->Set(SETTING_UTMAXPLAYERS, MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
				OGS->Set(SETTING_UTMAXSPECTATORS, MaxSpectators, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
				OGS->Set(SETTING_NUMMATCHES, UTGameMode->GetNumMatches(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
				OGS->Set(SETTING_PLAYERSONLINE, UTGameMode->GetNumPlayers(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
				OGS->Set(SETTING_SPECTATORSONLINE, UTGameMode->NumSpectators, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
				SessionInterface->UpdateSession(SessionName, *OGS, true);
			}
		}
	}
}


void AUTGameSession::InitHostBeacon(FOnlineSessionSettings* SessionSettings)
{
	UWorld* const World = GetWorld();
	check(!BeaconHost);

	// Always create a new beacon host
	BeaconHostListener = World->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
	check(BeaconHostListener);
	BeaconHost = World->SpawnActor<AUTServerBeaconHost>(AUTServerBeaconHost::StaticClass());
	check(BeaconHost);

	// Initialize beacon state, either new or from a seamless travel
	bool bBeaconInit = false;
	if (BeaconHostListener && BeaconHostListener->InitHost())
	{
		BeaconHostListener->RegisterHost(BeaconHost);
	}

	// Update the beacon port
	if (SessionSettings)
	{
		SessionSettings->Set(SETTING_BEACONPORT, BeaconHostListener->GetListenPort(), EOnlineDataAdvertisementType::ViaOnlineService);
	}
}

void AUTGameSession::DestroyHostBeacon()
{
	if (BeaconHost)
	{
		BeaconHostListener->UnregisterHost(BeaconHost->GetBeaconType());
		BeaconHost->Destroy();
		BeaconHost = NULL;

		BeaconHostListener->DestroyBeacon();
		BeaconHostListener = NULL;
	}
}