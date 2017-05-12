// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTTeamGameMode.h"
#include "UTHUD_CTF.h"
#include "UTCTFRoundGame.h"
#include "UTCTFGameMessage.h"
#include "UTCTFRoleMessage.h"
#include "UTCTFRewardMessage.h"
#include "UTCTFMajorMessage.h"
#include "UTFirstBloodMessage.h"
#include "UTPickup.h"
#include "UTGameMessage.h"
#include "UTMutator.h"
#include "UTCTFSquadAI.h"
#include "UTWorldSettings.h"
#include "Widgets/SUTTabWidget.h"
#include "Dialogs/SUTPlayerInfoDialog.h"
#include "StatNames.h"
#include "Engine/DemoNetDriver.h"
#include "UTCTFScoreboard.h"
#include "UTShowdownGameMessage.h"
#include "UTShowdownRewardMessage.h"
#include "UTShowdownStatusMessage.h"
#include "UTPlayerStart.h"
#include "UTTimedPowerup.h"
#include "UTPlayerState.h"
#include "UTFlagRunHUD.h"
#include "UTGhostFlag.h"
#include "UTCTFRoundGameState.h"
#include "UTAsymCTFSquadAI.h"
#include "UTAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Analytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"
#include "UTLineUpHelper.h"
#include "UTLineUpZone.h"
#include "UTProjectile.h"
#include "UTRemoteRedeemer.h"

AUTCTFRoundGame::AUTCTFRoundGame(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	TimeLimit = 5;
	IntermissionDuration = 28.f;
	RoundLives = 5;
	bNeedFiveKillsMessage = true;
	FlagCapScore = 1;
	UnlimitedRespawnWaitTime = 2.f;
	bForceRespawn = true;
	bFirstRoundInitialized = false;
	FlagPickupDelay = 10;
	HUDClass = AUTFlagRunHUD::StaticClass();
	GameStateClass = AUTCTFRoundGameState::StaticClass();
	SquadType = AUTAsymCTFSquadAI::StaticClass();
	NumRounds = 6;
	bHideInUI = true;

	InitialBoostCount = 0;
	MaxTimeScoreBonus = 180;

	bGameHasTranslocator = false;

	RollingAttackerRespawnDelay = 5.f;
	LastAttackerSpawnTime = 0.f;
	RollingSpawnStartTime = 0.f;
	bRollingAttackerSpawns = true;
	ForceRespawnTime = 0.1f;
	LimitedRespawnWaitTime = 6.f;
	MatchSummaryDelay = 20.f;

	bSitOutDuringRound = false;
	EndOfMatchMessageDelay = 2.5f;
	bUseLevelTiming = true;
}

void AUTCTFRoundGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!UGameplayStatics::HasOption(Options, TEXT("TimeLimit")) || (TimeLimit <= 0))
	{
		AUTWorldSettings* WorldSettings = bUseLevelTiming ? Cast<AUTWorldSettings>(GetWorldSettings()) : nullptr;
		TimeLimit = WorldSettings ? WorldSettings->DefaultRoundLength : TimeLimit;
	}

	// key options are ?RoundLives=xx?Dash=xx?Asymm=xx?PerPlayerLives=xx?OffKillsForPowerup=xx?DefKillsForPowerup=xx?DelayRally=xxx?Boost=xx
	RoundLives = FMath::Max(1, UGameplayStatics::GetIntOption(Options, TEXT("RoundLives"), RoundLives));

	FlagPickupDelay = FMath::Max(1, UGameplayStatics::GetIntOption(Options, TEXT("FlagDelay"), FlagPickupDelay));

}

void AUTCTFRoundGame::BeginGame()
{
	UE_LOG(UT, Log, TEXT("BEGIN GAME GameType: %s"), *GetNameSafe(this));
	UE_LOG(UT, Log, TEXT("Difficulty: %f GoalScore: %i TimeLimit (sec): %i"), GameDifficulty, GoalScore, TimeLimit);

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* TestActor = *It;
		if (TestActor && !TestActor->IsPendingKill() && TestActor->IsA<AUTPlayerState>())
		{
			Cast<AUTPlayerState>(TestActor)->StartTime = 0;
			Cast<AUTPlayerState>(TestActor)->bSentLogoutAnalytics = false;
		}
		else if (TestActor && !TestActor->IsPendingKill() && TestActor->IsA<AUTProjectile>())
		{
			TestActor->Destroy();
		}
	}
	if (CTFGameState)
	{
		CTFGameState->ElapsedTime = 0;
	}

	//Let the game session override the StartMatch function, in case it wants to wait for arbitration
	if (GameSession->HandleStartMatchRequest())
	{
		return;
	}
	if (CTFGameState)
	{
		CTFGameState->CTFRound = 1;
		CTFGameState->NumRounds = NumRounds;
		CTFGameState->HalftimeScoreDelay = 0.5f;
	}
	for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
	{
		AUTPlayerState* UTPlayerState = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
		if (UTPlayerState)
		{
			UTPlayerState->NotIdle();
		}
	}

	if (GetNetMode() == NM_Standalone)
	{
		IntermissionDuration = 15.f;
	}

	float RealIntermissionDuration = IntermissionDuration;
	IntermissionDuration = bBasicTrainingGame ? 12.f : 6.f;
	SetMatchState(MatchState::MatchIntermission);
	IntermissionDuration = RealIntermissionDuration;
	
	if ((!GetWorld() || !GetWorld()->GetGameState<AUTGameState>() || !GetWorld()->GetGameState<AUTGameState>()->LineUpHelper || !GetWorld()->GetGameState<AUTGameState>()->LineUpHelper->bIsActive))
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(It->Get());
			if (PC)
			{
				PC->ViewStartSpot();
			}
		}
	}
}

AActor* AUTCTFRoundGame::SetIntermissionCameras(uint32 TeamToWatch)
{
	return CTFGameState->FlagBases[TeamToWatch];
}

void AUTCTFRoundGame::HandleMatchIntermission()
{
	if (bFirstRoundInitialized)
	{
		// kick idlers
		if (UTGameState && GameSession && !bIgnoreIdlePlayers && !bIsLANGame)
		{
			for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
			{
				AUTPlayerState* UTPlayerState = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
				if (UTPlayerState && IsPlayerIdle(UTPlayerState) && Cast<APlayerController>(UTPlayerState->GetOwner()))
				{
					Cast<APlayerController>(UTPlayerState->GetOwner())->ClientWasKicked(NSLOCTEXT("General", "IdleKick", "You were kicked for being idle."));
					UTPlayerState->GetOwner()->Destroy();
				}
			}
		}

		// view defender base, with last team to score around it
		int32 TeamToWatch = IntermissionTeamToView(nullptr);

		if ((CTFGameState == NULL) || (TeamToWatch >= CTFGameState->FlagBases.Num()) || (CTFGameState->FlagBases[TeamToWatch] == NULL))
		{
			return;
		}

		UTGameState->PrepareForIntermission();

		AActor* IntermissionFocus = SetIntermissionCameras(TeamToWatch);
		// Tell the controllers to look at defender base
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
			if (PC != NULL)
			{
				PC->ClientPrepareForIntermission();
				PC->SetViewTarget(IntermissionFocus);
			}
		}

		if (UTGameState->LineUpHelper)
		{
			UTGameState->LineUpHelper->HandleLineUp(LineUpTypes::Intermission);
		}
	}

	if (CTFGameState)
	{
		CTFGameState->bIsAtIntermission = true;
		CTFGameState->bStopGameClock = true;
		CTFGameState->IntermissionTime = IntermissionDuration;
	}
}

void AUTCTFRoundGame::EndTeamGame(AUTTeamInfo* Winner, FName Reason)
{
	// Dont ever end the game in PIE
	if (GetWorld()->WorldType == EWorldType::PIE) return;

	UTGameState->WinningTeam = Winner;
	EndTime = GetWorld()->TimeSeconds;

	APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(GetWorld());
	UUTLocalPlayer* LP = LocalPC ? Cast<UUTLocalPlayer>(LocalPC->Player) : NULL;
	if (LP)
	{
		LP->EarnedStars = 0;
		LP->RosterUpgradeText = FText::GetEmpty();
		if (bOfflineChallenge && PlayerWonChallenge())
		{
			LP->ChallengeCompleted(ChallengeTag, ChallengeDifficulty + 1);
		}
	}

	if (IsGameInstanceServer() && LobbyBeacon)
	{
		FString MatchStats = FString::Printf(TEXT("%i"), CTFGameState->ElapsedTime);

		FMatchUpdate MatchUpdate;
		MatchUpdate.GameTime = UTGameState->ElapsedTime;
		MatchUpdate.NumPlayers = NumPlayers;
		MatchUpdate.NumSpectators = NumSpectators;
		MatchUpdate.MatchState = MatchState;
		MatchUpdate.bMatchHasBegun = HasMatchStarted();
		MatchUpdate.bMatchHasEnded = HasMatchEnded();

		UpdateLobbyScore(MatchUpdate);
		LobbyBeacon->EndGame(MatchUpdate);
	}

	// SETENDGAMEFOCUS
	EndMatch();
	AActor* EndMatchFocus = SetIntermissionCameras(Winner->TeamIndex);

	AUTCTFFlagBase* WinningBase =Cast<AUTCTFFlagBase>(EndMatchFocus);
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* Controller = Cast<AUTPlayerController>(*Iterator);
		if (Controller && Controller->UTPlayerState)
		{
			AUTCTFFlagBase* BaseToView = WinningBase;
			// If we don't have a winner, view my base
			if (BaseToView == NULL)
			{
				AUTTeamInfo* MyTeam = Controller->UTPlayerState->Team;
				if (MyTeam)
				{
					BaseToView = CTFGameState->FlagBases[MyTeam->GetTeamNum()];
				}
			}

			if (BaseToView)
			{
				if (UTGameState->LineUpHelper && UTGameState->LineUpHelper->bIsActive)
				{
					Controller->GameHasEnded(Controller->GetPawn(), (Controller->UTPlayerState->Team && (Controller->UTPlayerState->Team->TeamIndex == Winner->TeamIndex)));
				}
				else
				{
					Controller->GameHasEnded(BaseToView, (Controller->UTPlayerState->Team && (Controller->UTPlayerState->Team->TeamIndex == Winner->TeamIndex)));
				}
			}
		}
	}

	// Allow replication to happen before reporting scores, stats, etc.
	FTimerHandle TempHandle;
	GetWorldTimerManager().SetTimer(TempHandle, this, &AUTGameMode::HandleMatchHasEnded, 1.5f);
	bGameEnded = true;

	// Setup a timer to continue to the next map.  Need enough time for match summaries
	EndTime = GetWorld()->TimeSeconds;
	float TravelDelay = GetTravelDelay();
	FTimerHandle TempHandle3;
	GetWorldTimerManager().SetTimer(TempHandle3, this, &AUTGameMode::TravelToNextMap, TravelDelay*GetActorTimeDilation());

	FTimerHandle TempHandle4;
	float EndReplayDelay = TravelDelay - 10.f;
	GetWorldTimerManager().SetTimer(TempHandle4, this, &AUTCTFRoundGame::StopRCTFReplayRecording, EndReplayDelay);

	SendEndOfGameStats(Reason);
}

void AUTCTFRoundGame::StopRCTFReplayRecording()
{
	if (Super::UTIsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StopRecordingReplay();
	}
}

void AUTCTFRoundGame::IntermissionSwapSides()
{
	// swap sides, if desired
	AUTWorldSettings* Settings = Cast<AUTWorldSettings>(GetWorld()->GetWorldSettings());
	if (Settings != NULL && Settings->bAllowSideSwitching)
	{
		CTFGameState->ChangeTeamSides(1);
	}
}

void AUTCTFRoundGame::InitGameState()
{
	Super::InitGameState();

	CTFGameState->CTFRound = 1;
	CTFGameState->NumRounds = NumRounds;
}

void AUTCTFRoundGame::InitFlagForRound(AUTCarriedObject* Flag)
{
}

void AUTCTFRoundGame::InitFlags()
{
	for (AUTCTFFlagBase* Base : CTFGameState->FlagBases)
	{
		if (Base != NULL && Base->MyFlag)
		{
			InitFlagForRound(Base->MyFlag);

			// check for flag carrier already here waiting
			TArray<AActor*> Overlapping;
			Base->MyFlag->GetOverlappingActors(Overlapping, AUTCharacter::StaticClass());
			// try humans first, then bots
			for (AActor* A : Overlapping)
			{
				AUTCharacter* Character = Cast<AUTCharacter>(A);
				if (Character != nullptr && Cast<APlayerController>(Character->Controller) != nullptr)
				{
					if (!GetWorld()->LineTraceTestByChannel(Character->GetActorLocation(), Base->MyFlag->GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams))
					{
						Base->MyFlag->TryPickup(Character);
						if (Base->MyFlag->ObjectState == CarriedObjectState::Held)
						{
							return;
						}
					}
				}
			}
			for (AActor* A : Overlapping)
			{
				AUTCharacter* Character = Cast<AUTCharacter>(A);
				if (Character != nullptr && Cast<APlayerController>(Character->Controller) == nullptr)
				{
					if (!GetWorld()->LineTraceTestByChannel(Character->GetActorLocation(), Base->MyFlag->GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams))
					{
						Base->MyFlag->TryPickup(Character);
						if (Base->MyFlag->ObjectState == CarriedObjectState::Held)
						{
							return;
						}
					}
				}
			}
		}
	}
}

void AUTCTFRoundGame::FlagCountDown()
{
	AUTCTFRoundGameState* RCTFGameState = Cast<AUTCTFRoundGameState>(CTFGameState);
	if (RCTFGameState && IsMatchInProgress() && (MatchState != MatchState::MatchIntermission))
	{
		RCTFGameState->RemainingPickupDelay--;
		if (RCTFGameState->RemainingPickupDelay > 0)
		{
			FTimerHandle TempHandle;
			GetWorldTimerManager().SetTimer(TempHandle, this, &AUTCTFRoundGame::FlagCountDown, 1.f*GetActorTimeDilation(), false);
		}
		else
		{
			FlagsAreReady();
		}
	}
}

void AUTCTFRoundGame::FlagsAreReady()
{
	BroadcastLocalized(this, UUTCTFMajorMessage::StaticClass(), 21, NULL, NULL, NULL);
	InitFlags();
}

void AUTCTFRoundGame::InitGameStateForRound()
{
	AUTCTFRoundGameState* RCTFGameState = Cast<AUTCTFRoundGameState>(CTFGameState);
	if (RCTFGameState)
	{
		RCTFGameState->CTFRound++;
		RCTFGameState->RemainingPickupDelay = FlagPickupDelay;
	}
}

void AUTCTFRoundGame::InitDelayedFlag(AUTCarriedObject* Flag)
{
	if (Flag != nullptr)
	{
		Flag->bEnemyCanPickup = false;
		Flag->bFriendlyCanPickup = false;
		Flag->bTeamPickupSendsHome = false;
		Flag->bEnemyPickupSendsHome = false;
	}
}

//Special markup for Analytics event so they show up properly in grafana. Should be eventually moved to UTAnalytics.
/*
* @EventName RCTFRoundResult
* @Trigger Sent when a round ends in an RCTF game through Score Alternate Win
* @Type Sent by the Server
* @EventParam FlagCapScore int32 Always 0, just shows that someone caped flag
* @Comments
*/

