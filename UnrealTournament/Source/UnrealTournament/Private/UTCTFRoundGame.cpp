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
	bCarryOwnFlag = true;
	bNoFlagReturn = true;
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
	bSlowFlagCarrier = false;
	EndOfMatchMessageDelay = 2.5f;
	bUseLevelTiming = true;
}

void AUTCTFRoundGame::PostLogin(APlayerController* NewPlayer)
{
	if (NewPlayer)
	{
		InitPlayerForRound(Cast<AUTPlayerState>(NewPlayer->PlayerState));
		AUTPlayerState* UTPS = Cast<AUTPlayerController>(NewPlayer) ? Cast<AUTPlayerController>(NewPlayer)->UTPlayerState : nullptr;
		if (UTPS && UTPS->Team && IsMatchInProgress() && UTGameState && !UTGameState->IsMatchIntermission())
		{
			NewPlayer->ClientReceiveLocalizedMessage(UUTCTFRoleMessage::StaticClass(), IsTeamOnDefense(UTPS->Team->TeamIndex) ? 2 : 1);
		}
	}
	Super::PostLogin(NewPlayer);
}

void AUTCTFRoundGame::CreateGameURLOptions(TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps)
{
	MenuProps.Empty();
	if (BotFillCount == 0)
	{
		BotFillCount = DefaultMaxPlayers;
	}
	MenuProps.Add(MakeShareable(new TAttributeProperty<int32>(this, &BotFillCount, TEXT("BotFill"))));
	MenuProps.Add(MakeShareable(new TAttributePropertyBool(this, &bBalanceTeams, TEXT("BalanceTeams"))));
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

	FString InOpt = UGameplayStatics::ParseOption(Options, TEXT("OwnFlag"));
	bCarryOwnFlag = EvalBoolOptions(InOpt, bCarryOwnFlag);

	InOpt = UGameplayStatics::ParseOption(Options, TEXT("FlagReturn"));
	bNoFlagReturn = EvalBoolOptions(InOpt, bNoFlagReturn);

	FlagPickupDelay = FMath::Max(1, UGameplayStatics::GetIntOption(Options, TEXT("FlagDelay"), FlagPickupDelay));

	InOpt = UGameplayStatics::ParseOption(Options, TEXT("SlowFC"));
	bSlowFlagCarrier = EvalBoolOptions(InOpt, bSlowFlagCarrier);
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

float AUTCTFRoundGame::AdjustNearbyPlayerStartScore(const AController* Player, const AController* OtherController, const ACharacter* OtherCharacter, const FVector& StartLoc, const APlayerStart* P)
{
	return 0.f;
}

void AUTCTFRoundGame::PlayEndOfMatchMessage()
{
	if (UTGameState && UTGameState->WinningTeam)
	{
		int32 IsFlawlessVictory = (UTGameState->WinningTeam->Score == 12);
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* Controller = Iterator->Get();
			if (Controller && Controller->IsA(AUTPlayerController::StaticClass()))
			{
				AUTPlayerController* PC = Cast<AUTPlayerController>(Controller);
				if (PC && Cast<AUTPlayerState>(PC->PlayerState))
				{
					if (bSecondaryWin)
					{
						PC->ClientReceiveLocalizedMessage(VictoryMessageClass, 4 + ((UTGameState->WinningTeam == Cast<AUTPlayerState>(PC->PlayerState)->Team) ? 1 : 0), UTGameState->WinnerPlayerState, PC->PlayerState, UTGameState->WinningTeam);
					}
					else
					{
						PC->ClientReceiveLocalizedMessage(VictoryMessageClass, 2 * IsFlawlessVictory + ((UTGameState->WinningTeam == Cast<AUTPlayerState>(PC->PlayerState)->Team) ? 1 : 0), UTGameState->WinnerPlayerState, PC->PlayerState, UTGameState->WinningTeam);
					}
				}
			}
		}
	}
}

void AUTCTFRoundGame::DefaultTimer()
{
	Super::DefaultTimer();

	//Super::DefaultTimer will never set the ReplayID because of our override of IsUTHandingReplays, 
	//check Super::IsUTHandlingReplays instead to see if we should set ReplayID.
	if (Super::UTIsHandlingReplays())
	{
		UDemoNetDriver* DemoNetDriver = GetWorld()->DemoNetDriver;
		if (DemoNetDriver != nullptr && DemoNetDriver->ReplayStreamer.IsValid())
		{
			UTGameState->ReplayID = DemoNetDriver->ReplayStreamer->GetReplayID();
		}
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

void AUTCTFRoundGame::HandleFlagCapture(AUTCharacter* HolderPawn, AUTPlayerState* Holder)
{
	FlagScorer = Holder;
	CheckScore(Holder);
	if (UTGameState && UTGameState->IsMatchInProgress())
	{
		Holder->AddCoolFactorEvent(400.0f);

		SetMatchState(MatchState::MatchIntermission);
	}
}

int32 AUTCTFRoundGame::IntermissionTeamToView(AUTPlayerController* PC)
{
	if (LastTeamToScore)
	{
		return LastTeamToScore->TeamIndex;
	}
	return Super::IntermissionTeamToView(PC);
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

void AUTCTFRoundGame::HandleExitingIntermission()
{
	CTFGameState->bStopGameClock = false;
	CTFGameState->HalftimeScoreDelay = 3.f;
	RemoveAllPawns();

	if (bFirstRoundInitialized)
	{
		IntermissionSwapSides();
	}
	else
	{
		CTFGameState->CTFRound = 0;
	}

	InitRound();
	if (!bFirstRoundInitialized)
	{
		bFirstRoundInitialized = true;
		if (Super::UTIsHandlingReplays() && GetGameInstance() != nullptr)
		{
			GetGameInstance()->StartRecordingReplay(TEXT(""), GetWorld()->GetMapName());
		}
	}

	//now respawn all the players
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = Iterator->Get();
		if (Controller->PlayerState != NULL && !Controller->PlayerState->bOnlySpectator)
		{
			RestartPlayer(Controller);

			// Reset group taunt
			AUTPlayerState* PS = Cast<AUTPlayerState>(Controller->PlayerState);
			if (PS)
			{
				PS->ActiveGroupTaunt = nullptr;
				PS->ClearRoundStats();
			}
		}
	}

	// Send all flags home..
	CTFGameState->ResetFlags();
	CTFGameState->bIsAtIntermission = false;
	CTFGameState->SetTimeLimit(TimeLimit);		// Reset the GameClock for the second time.
	SetMatchState(MatchState::InProgress);

	if (UTGameState->LineUpHelper)
	{
		UTGameState->LineUpHelper->CleanUp();
	}
}

void AUTCTFRoundGame::InitFlagForRound(AUTCarriedObject* Flag)
{
	if (Flag != nullptr)
	{
		Flag->AutoReturnTime = 8.f;
		Flag->bGradualAutoReturn = true;
		Flag->bDisplayHolderTrail = true;
		Flag->bShouldPingFlag = true;
		Flag->bSlowsMovement = bSlowFlagCarrier;
		Flag->ClearGhostFlags();
		Flag->bSendHomeOnScore = false;
		Flag->bEnemyCanPickup = !bCarryOwnFlag;
		Flag->bFriendlyCanPickup = bCarryOwnFlag;
		Flag->bTeamPickupSendsHome = !Flag->bFriendlyCanPickup && !bNoFlagReturn;
		Flag->bEnemyPickupSendsHome = !Flag->bEnemyCanPickup && !bNoFlagReturn;
	}
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

void AUTCTFRoundGame::InitRound()
{
	FlagScorer = nullptr;
	bFirstBloodOccurred = false;
	bLastManOccurred = false;
	bNeedFiveKillsMessage = true;
	InitGameStateForRound();
	ResetFlags();
	if (FlagPickupDelay > 0)
	{
		for (AUTCTFFlagBase* Base : CTFGameState->FlagBases)
		{
			if (Base != NULL && Base->MyFlag)
			{
				InitDelayedFlag(Base->MyFlag);
			}
		}
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTCTFRoundGame::FlagCountDown, 1.f*GetActorTimeDilation(), false);
	}
	else
	{
		InitFlags();
	}

	for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
	{
		AUTPlayerState* PS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
		InitPlayerForRound(PS);

	}
	CTFGameState->SetTimeLimit(TimeLimit);

	// re-initialize all AI squads, in case objectives have changed sides
	for (AUTTeamInfo* Team : Teams)
	{
		Team->ReinitSquads();
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

void AUTCTFRoundGame::ResetFlags()
{
	for (AUTCTFFlagBase* Base : CTFGameState->FlagBases)
	{
		if (Base != NULL && Base->MyFlag)
		{
			Base->MyFlag->SetActorHiddenInGame(true);
			Base->ClearDefenseEffect();
			if (IsTeamOnDefense(Base->MyFlag->GetTeamNum()))
			{
				Base->SpawnDefenseEffect();
			}
		}
	}
}

void AUTCTFRoundGame::InitPlayerForRound(AUTPlayerState* PS)
{
	if (PS)
	{
		PS->RoundKills = 0;
		PS->RoundDeaths = 0;
		PS->RoundKillAssists = 0;
		PS->bRallyActivated = false;
		PS->RespawnWaitTime = IsPlayerOnLifeLimitedTeam(PS) ? LimitedRespawnWaitTime : UnlimitedRespawnWaitTime;
		PS->SetRemainingBoosts(InitialBoostCount);
		PS->bSpecialTeamPlayer = false;
		PS->bSpecialPlayer = false;
		if (GetNetMode() != NM_DedicatedServer)
		{
			PS->OnRepSpecialPlayer();
			PS->OnRepSpecialTeamPlayer();
		}
		if (PS && (!PS->Team || PS->bOnlySpectator))
		{
			PS->RemainingLives = 0;
			PS->SetOutOfLives(true);
		}
		else if (PS)
		{
			PS->RemainingLives = RoundLives;
			PS->SetOutOfLives(false);
		}
		PS->ForceNetUpdate();
	}
}

void AUTCTFRoundGame::HandleRollingAttackerRespawn(AUTPlayerState* OtherPS)
{
	if (GetWorld()->GetTimeSeconds() - LastAttackerSpawnTime < 1.f)
	{
		OtherPS->RespawnWaitTime = 1.f;
	}
	else
	{
		if (GetWorld()->GetTimeSeconds() - RollingSpawnStartTime < RollingAttackerRespawnDelay)
		{
			OtherPS->RespawnWaitTime = RollingAttackerRespawnDelay - GetWorld()->GetTimeSeconds() + RollingSpawnStartTime;
		}
		else
		{
			RollingSpawnStartTime = GetWorld()->GetTimeSeconds();
			OtherPS->RespawnWaitTime = RollingAttackerRespawnDelay;
		}
		// if friendly hanging near flag base, respawn right away
		AUTCTFFlagBase* TeamBase = CTFGameState->FlagBases[OtherPS->Team->TeamIndex];
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			AUTCharacter* Teammate = Cast<AUTCharacter>(*It);
			if (Teammate && !Teammate->IsDead() && CTFGameState && CTFGameState->OnSameTeam(OtherPS, Teammate) && ((Teammate->GetActorLocation() - TeamBase->GetActorLocation()).SizeSquared() < 4000000.f))
			{
				OtherPS->RespawnWaitTime = 1.f;
				break;
			}
		}
	}
}

void AUTCTFRoundGame::ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	Super::ScoreKill_Implementation(Killer, Other, KilledPawn, DamageType);

	AUTPlayerState* OtherPS = Other ? Cast<AUTPlayerState>(Other->PlayerState) : nullptr;
	if (OtherPS && OtherPS->Team && IsTeamOnOffense(OtherPS->Team->TeamIndex) && bRollingAttackerSpawns)
	{
		HandleRollingAttackerRespawn(OtherPS);
		OtherPS->RespawnWaitTime += 0.01f * FMath::FRand();
		OtherPS->ForceNetUpdate();
		OtherPS->OnRespawnWaitReceived();
	}
	if (OtherPS && IsPlayerOnLifeLimitedTeam(OtherPS) && (OtherPS->RemainingLives > 0))
	{
		OtherPS->RemainingLives--;
		bool bEliminated = false;
		if (OtherPS->RemainingLives == 0)
		{
			// this player is out of lives
			OtherPS->SetOutOfLives(true);
			bEliminated = true;
			bool bFoundTeammate = false;
			for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
			{
				AUTPlayerState* TeamPS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
				if (TeamPS && (OtherPS->Team == TeamPS->Team) && !TeamPS->bOutOfLives && !TeamPS->bIsInactive)
				{
					// found a live teammate, so round isn't over - notify about termination though
					if (IsMatchInProgress() && (GetMatchState() != MatchState::MatchIntermission))
					{
						BroadcastLocalized(NULL, UUTShowdownRewardMessage::StaticClass(), 3, OtherPS);
					}
					bFoundTeammate = true;
					break;
				}
			}
			if (!bFoundTeammate)
			{
				BroadcastLocalized(NULL, UUTShowdownRewardMessage::StaticClass(), 4);
				CTFGameState->bStopGameClock = true;

				if (OtherPS->Team->TeamIndex == 0)
				{
					FTimerHandle TempHandle;
					GetWorldTimerManager().SetTimer(TempHandle, this, &AUTCTFRoundGame::ScoreBlueAlternateWin, 1.f);
				}
				else
				{
					FTimerHandle TempHandle;
					GetWorldTimerManager().SetTimer(TempHandle, this, &AUTCTFRoundGame::ScoreRedAlternateWin, 1.f);

				}
			}
		}
		else if (UTGameState && IsMatchInProgress() && (GetMatchState() != MatchState::MatchIntermission))
		{
			bool bFoundLiveTeammate = false;
			int32 TeamCount = 0;
			for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
			{
				AUTPlayerState* TeamPS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
				if (TeamPS && (OtherPS->Team == TeamPS->Team) && !TeamPS->bOutOfLives && !TeamPS->bIsInactive)
				{
					TeamCount++;
					if (TeamPS->GetUTCharacter() && !TeamPS->GetUTCharacter()->IsDead())
					{
						bFoundLiveTeammate = true;
						break;
					}
				}
			}
			if (!bFoundLiveTeammate && (TeamCount == 5) && (GetWorld()->GetTimeSeconds() - LastAceTime > 20.f))
			{
				LastAceTime = GetWorld()->GetTimeSeconds();
				BroadcastLocalized(NULL, UUTCTFRewardMessage::StaticClass(), 8);
			}
		}

		int32 RemainingDefenders = 0;
		int32 RemainingLives = 0;

		// check if just transitioned to last man
		bool bWasAlreadyLastMan = bLastManOccurred;
		AUTPlayerState* LastMan = nullptr;
		for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
		{
			AUTPlayerState* PS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
			if (PS && (OtherPS->Team == PS->Team) && !PS->bOutOfLives && !PS->bIsInactive)
			{
				RemainingDefenders++;
				RemainingLives += PS->RemainingLives;
				LastMan = PS;
			}
		}
		bLastManOccurred = (RemainingDefenders == 1);
		if (bLastManOccurred && !bWasAlreadyLastMan)
		{
			for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
			{
				AUTPlayerState* PS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
				AUTPlayerController* PC = PS ? Cast<AUTPlayerController>(PS->GetOwner()) : nullptr;
				if (PC)
				{
					int32 MessageType = (OtherPS->Team == PS->Team) ? 1 : 0;
					PC->ClientReceiveLocalizedMessage(UUTShowdownRewardMessage::StaticClass(), MessageType, LastMan, NULL, NULL);
				}
			}
		}
		else if (((RemainingDefenders == 3) && bEliminated) || (RemainingLives < 10))
		{
			// find player on other team to speak message
			AUTPlayerState* Speaker = nullptr;
			for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
			{
				AUTPlayerState* TeamPS = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
				if (TeamPS && TeamPS->Team && (OtherPS->Team != TeamPS->Team) && !TeamPS->bOutOfLives && !TeamPS->bIsInactive)
				{
					Speaker = TeamPS;
					break;
				}
			}
			if (Speaker != nullptr)
			{
				if ((RemainingDefenders == 3) && bEliminated)
				{
					Speaker->AnnounceStatus(StatusMessage::EnemyThreePlayers);
				}
				else if (RemainingLives == 9)
				{
					Speaker->AnnounceStatus(StatusMessage::EnemyLowLives);
				}
			}
		}

		if (OtherPS->RemainingLives > 0)
		{
			OtherPS->RespawnWaitTime = FMath::Max(1.f, float(RemainingDefenders));
			if (UTGameState && UTGameState->GetRemainingTime() > 150)
			{
				OtherPS->RespawnWaitTime = FMath::Min(OtherPS->RespawnWaitTime, 2.f);
			}
			OtherPS->RespawnWaitTime += 0.01f * FMath::FRand();
			OtherPS->ForceNetUpdate();
			OtherPS->OnRespawnWaitReceived();
		}
	}
}

void AUTCTFRoundGame::ScoreRedAlternateWin()
{
}

void AUTCTFRoundGame::ScoreBlueAlternateWin()
{
}

//Special markup for Analytics event so they show up properly in grafana. Should be eventually moved to UTAnalytics.
/*
* @EventName RCTFRoundResult
* @Trigger Sent when a round ends in an RCTF game through Score Alternate Win
* @Type Sent by the Server
* @EventParam FlagCapScore int32 Always 0, just shows that someone caped flag
* @Comments
*/

int32 AUTCTFRoundGame::GetDefenseScore()
{
	return 1;
}

bool AUTCTFRoundGame::IsTeamOnOffense(int32 TeamNumber) const
{
	return true;
}

bool AUTCTFRoundGame::IsTeamOnDefense(int32 TeamNumber) const
{
	return !IsTeamOnOffense(TeamNumber);
}

bool AUTCTFRoundGame::IsPlayerOnLifeLimitedTeam(AUTPlayerState* PlayerState) const
{
	return PlayerState && PlayerState->Team && IsTeamOnDefense(PlayerState->Team->TeamIndex);
}
