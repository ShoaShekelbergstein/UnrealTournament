// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTTeamGameMode.h"
#include "UTCTFGameMode.h"
#include "UTGauntletGame.h"
#include "UTGauntletFlagDispenser.h"
#include "UTGauntletFlag.h"
#include "UTGauntletGameState.h"
#include "UTGauntletGameMessage.h"
#include "UTCTFGameMessage.h"
#include "UTCTFRewardMessage.h"
#include "UTFirstBloodMessage.h"
#include "UTCountDownMessage.h"
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
#include "UTShowdownRewardMessage.h"
#include "UTShowdownGameMessage.h"
#include "UTDroppedAmmoBox.h"
#include "UTDroppedLife.h"
#include "UTTimedPowerup.h"
#include "UTGauntletHUD.h"
#include "UTIntermissionBeginInterface.h"

namespace MatchState
{
	const FName GauntletScoreSummary = FName(TEXT("GauntletScoreSummary"));
	const FName GauntletFadeToBlack = FName(TEXT("GauntletFadeToBlack"));
	const FName GauntletRoundAnnounce = FName(TEXT("GauntletRoundAnnounce"));
}

AUTGauntletGame::AUTGauntletGame(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	GoalScore = 3;
	TimeLimit = 0;
	InitialBoostCount = 0;
	DisplayName = NSLOCTEXT("UTGauntletGame", "GauntletDisplayName", "Gauntlet");

	GameStateClass = AUTGauntletGameState::StaticClass();
	HUDClass = AUTGauntletHUD::StaticClass();
	SquadType = AUTCTFSquadAI::StaticClass();
	RoundLives=5;
	bPerPlayerLives = true;
	FlagSwapTime=5;
	FlagPickupDelay=15;
	MapPrefix = TEXT("CTF");
	bHideInUI = true;
	bRollingAttackerSpawns = false;
	bWeaponStayActive = false;
	bCarryOwnFlag = true;
	bNoFlagReturn = true;
	UnlimitedRespawnWaitTime = 3.0f;
	bGameHasTranslocator = false;
	bSitOutDuringRound = false;

	ScoreSummaryDuration = 6.0f;
	FadeToBlackDuration = 1.5f;
	RoundAnnounceDuration = 3.5f;
}

void AUTGauntletGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bForceRespawn = false;
	GoalScore = 3;	
	FlagSwapTime = FMath::Max(0, UGameplayStatics::GetIntOption(Options, TEXT("FlagSwapTime"), FlagSwapTime));

	TAssetSubclassOf<AUTWeapon> WeaponObj;

	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/LinkGun/BP_LinkGun.BP_LinkGun_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/BioRifle/BP_BioRifle.BP_BioRifle_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/ShockRifle/ShockRifle.ShockRifle_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/Minigun/BP_Minigun.BP_Minigun_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/Flak/BP_FlakCannon.BP_FlakCannon_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	WeaponObj = FStringAssetReference(TEXT("/Game/RestrictedAssets/Weapons/RocketLauncher/BP_RocketLauncher.BP_RocketLauncher_C"));
	DefaultWeaponLoadoutObjects.Add(WeaponObj);
	bFirstRoundInitialized = true;
}

void AUTGauntletGame::InitGameState()
{
	Super::InitGameState();

	GauntletGameState = Cast<AUTGauntletGameState>(UTGameState);
	GauntletGameState->FlagSwapTime = FlagSwapTime;
	GauntletGameState->CTFRound = 0;

	// Turn off weapon stay
	bWeaponStayActive = false;

}

void AUTGauntletGame::GiveDefaultInventory(APawn* PlayerPawn)
{
	Super::GiveDefaultInventory(PlayerPawn);
	AUTCharacter* UTCharacter = Cast<AUTCharacter>(PlayerPawn);
	if (PlayerPawn)
	{
		// Players start with all weapons except the sniper.  TODO: Reduce the starting ammo
		for (int32 i = 0; i < DefaultWeaponLoadoutObjects.Num(); i++)
		{
			TSubclassOf<AUTWeapon> InvClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *DefaultWeaponLoadoutObjects[i].ToStringReference().ToString(), NULL, LOAD_NoWarn));
			AUTWeapon* Weapon = UTCharacter->CreateInventory(InvClass,true);
			if (Weapon != nullptr)
			{
				Weapon->Ammo *= 0.5;
			}
		}
	}
}

void AUTGauntletGame::ResetFlags() 
{
	Super::ResetFlags();
	if (GauntletGameState->FlagDispenser)
	{
		GauntletGameState->FlagDispenser->Reset();
	}
} 

void AUTGauntletGame::InitRound()
{
	Super::InitRound();

	if (FlagPickupDelay > 0)
	{
		if (GauntletGameState->FlagDispenser)
		{
			GauntletGameState->FlagDispenser->InitRound();
		}
	}

	GauntletGameState->WinningTeam = nullptr;
	GauntletGameState->WinnerPlayerState = nullptr;
}

void AUTGauntletGame::InitFlagForRound(class AUTCarriedObject* Flag)
{
	Super::InitFlagForRound(Flag);
	if (Flag != nullptr)
	{
		Flag->bGradualAutoReturn = false;	
	}
}

void AUTGauntletGame::InitFlags() 
{
	if (GauntletGameState->FlagDispenser)
	{
		AUTCarriedObject* Flag = GauntletGameState->FlagDispenser->MyFlag;
		Flag->AutoReturnTime = 8.f;
		Flag->bGradualAutoReturn = false;
		Flag->bDisplayHolderTrail = true;
		Flag->bShouldPingFlag = true;
		Flag->bSlowsMovement = bSlowFlagCarrier;
		Flag->bSendHomeOnScore = false;
		Flag->SetActorHiddenInGame(false);
		Flag->bAnyoneCanPickup = true;
		Flag->bTeamPickupSendsHome = false;
		Flag->bEnemyPickupSendsHome = false;

		// check for flag carrier already here waiting
		TArray<AActor*> Overlapping;
		Flag->GetOverlappingActors(Overlapping, AUTCharacter::StaticClass());
		for (AActor* A : Overlapping)
		{
			AUTCharacter* Character = Cast<AUTCharacter>(A);
			if (Character != NULL)
			{
				if (!GetWorld()->LineTraceTestByChannel(Character->GetActorLocation(), Flag->GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams))
				{
					Flag->TryPickup(Character);
				}
			}
		}
	}
}


void AUTGauntletGame::FlagTeamChanged(uint8 NewTeamIndex)
{
	if (GauntletGameState->Flag->GetTeamNum() != 255)
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
			if (PC)
			{
				PC->ClientReceiveLocalizedMessage(UUTGauntletGameMessage::StaticClass(), PC->GetTeamNum() == NewTeamIndex ? 2 : 3, nullptr, nullptr, nullptr);
			}
		}
	}

}

// Looks to see if a given team has a chance to keep playing
bool AUTGauntletGame::IsTeamStillAlive(uint8 TeamNum)
{
/*
	// Look to see if anyone else is alive on this team...
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerState* PlayerState = Cast<AUTPlayerState>((*Iterator)->PlayerState);
		if (PlayerState && PlayerState->GetTeamNum() == TeamNum)
		{
			AUTCharacter* UTChar = Cast<AUTCharacter>((*Iterator)->GetPawn());
			if (!PlayerState->bOutOfLives || (UTChar && !UTChar->IsDead()))
			{
				return true;
			}
		}
	}
*/
	return true;
}

bool AUTGauntletGame::CanFlagTeamSwap(uint8 NewTeamNum)
{
	return IsTeamStillAlive(NewTeamNum);
}


void AUTGauntletGame::GameObjectiveInitialized(AUTGameObjective* Obj)
{
	// Convert all existing non-dispensers in to score only bases
	AUTGauntletFlagDispenser* FlagDispenser = Cast<AUTGauntletFlagDispenser>(Obj);
	if (FlagDispenser == nullptr)
	{
		AUTCTFFlagBase* FlagBase = Cast<AUTCTFFlagBase>(Obj);
		if (FlagBase)
		{
			FlagBase->bScoreOnlyBase = true;
		}
	}
	else
	{
		GauntletGameState->FlagDispenser = FlagDispenser;
	}

	Super::GameObjectiveInitialized(Obj);
	Obj->InitializeObjective();
}

void AUTGauntletGame::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	if (GauntletGameState->FlagDispenser == nullptr)
	{
		AUTCTFFlagBase* RedBase = nullptr;
		AUTCTFFlagBase* BlueBase = nullptr;

		for (int32 i=0; i < GauntletGameState->FlagBases.Num(); i++)
		{
			if (GauntletGameState->FlagBases[i])
			{
				if (GauntletGameState->FlagBases[i]->GetTeamNum() == 0 ) RedBase = GauntletGameState->FlagBases[i];
				else if (GauntletGameState->FlagBases[i]->GetTeamNum() == 1) BlueBase = GauntletGameState->FlagBases[i];
			}
		}

		if (RedBase && BlueBase)
		{
			// Find the mid point between the bases..
			FVector Direction = RedBase->GetActorLocation() - BlueBase->GetActorLocation();
			FVector MidPoint = BlueBase->GetActorLocation() + Direction.GetSafeNormal() * (Direction.Size() * 0.5);

			// Now find the powerup closest to the mid point.

			float BestDist = 0.0f;
			AUTPickupInventory* BestInventory = nullptr;

			// Find the powerup closest to the middle of the playing field
			for (TActorIterator<AUTPickupInventory> ObjIt(GetWorld()); ObjIt; ++ObjIt)
			{
				float Dist = (ObjIt->GetActorLocation() - MidPoint).SizeSquared();
				if (BestInventory == nullptr || Dist < BestDist)
				{
					BestInventory = *ObjIt;
					BestDist = Dist;
				}
			}
			if (BestInventory)
			{
				FVector SpawnLocation = BestInventory->GetActorLocation();
				static FName NAME_PlacementTrace(TEXT("PlacementTrace"));
				FCollisionQueryParams QueryParams(NAME_PlacementTrace, false, BestInventory);
				QueryParams.bTraceAsyncScene = true;
				FHitResult Hit(1.f);
				bool bHitWorld = GetWorld()->LineTraceSingleByChannel(Hit, SpawnLocation, SpawnLocation + FVector(0,0,-200),ECollisionChannel::ECC_Pawn, QueryParams);
				if (bHitWorld)
				{
					SpawnLocation = Hit.Location;
				}

				AUTGauntletFlagDispenser* FlagDispenser = GetWorld()->SpawnActor<AUTGauntletFlagDispenser>(AUTGauntletFlagDispenser::StaticClass(), SpawnLocation, Direction.Rotation());
				if (FlagDispenser != nullptr)
				{
					GauntletGameState->FlagDispenser = FlagDispenser;
					BestInventory->Destroy();
				}
			}
			else
			{
				UE_LOG(UT, Error, TEXT("Gauntlet needs at least a powerup to use as an anchor for the dispenser."));
			}
		}
		else
		{
			UE_LOG(UT,Error, TEXT("There has to be both a red and a blue base in Gauntlet."));
		}
	}
}

void AUTGauntletGame::FlagsAreReady()
{
	// Send out own message here.

	if (GauntletGameState->FlagDispenser)
	{
		GauntletGameState->FlagDispenser->CreateFlag();
		// Remove the defense effect from all of the flag bases.  It will get readded as the flag changes hands.
		for (int32 i=0; i < GauntletGameState->FlagBases.Num(); i++)
		{
			GauntletGameState->FlagBases[i]->ClearDefenseEffect();
		}
	}

	InitFlags();
}

bool AUTGauntletGame::CheckScore_Implementation(AUTPlayerState* Scorer)
{
	// Skip the round based version as we use goal score
	// We will check the score against Goal score after the GauntletScoreSummary is finished
	return false;
}

void AUTGauntletGame::HandleExitingIntermission()
{
	Super::HandleExitingIntermission();
	GauntletGameState->bFirstRoundInitialized = bFirstRoundInitialized;
}

void AUTGauntletGame::CheckGameTime()
{
	AUTCTFRoundGameState* RCTFGameState = Cast<AUTCTFRoundGameState>(CTFGameState);

	if (CTFGameState->IsMatchIntermission())
	{
		if (RCTFGameState && (RCTFGameState->IntermissionTime == 5))
		{
			int32 MessageIndex = bFirstRoundInitialized ? RCTFGameState->CTFRound + 2001 : 2001;
			BroadcastLocalized(this, UUTCountDownMessage::StaticClass(), MessageIndex, NULL, NULL, NULL);
		}
		if (RCTFGameState && (RCTFGameState->IntermissionTime <= 0))
		{
			SetMatchState(MatchState::MatchExitingIntermission);
		}
	}
	else
	{
		Super::CheckGameTime();
	}
}

void AUTGauntletGame::BeginGame()
{
	UE_LOG(UT, Warning, TEXT("BEGIN GAME GameType: %s"), *GetNameSafe(this));
	UE_LOG(UT, Warning, TEXT("Difficulty: %f GoalScore: %i TimeLimit (sec): %i"), GameDifficulty, GoalScore, TimeLimit);

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* TestActor = *It;
		if (TestActor && !TestActor->IsPendingKill() && TestActor->IsA<AUTPlayerState>())
		{
			Cast<AUTPlayerState>(TestActor)->StartTime = 0;
			Cast<AUTPlayerState>(TestActor)->bSentLogoutAnalytics = false;
		}
	}

	//Let the game session override the StartMatch function, in case it wants to wait for arbitration
	if (GameSession->HandleStartMatchRequest())
	{
		return;
	}
	if (GauntletGameState)
	{
		GauntletGameState->CTFRound = 0;
		GauntletGameState->NumRounds = NumRounds;
		GauntletGameState->HalftimeScoreDelay = 0.5f;
	}

	float RealIntermissionDuration = IntermissionDuration;
	IntermissionDuration = 10.f;
	SetMatchState(MatchState::GauntletFadeToBlack);
	IntermissionDuration = RealIntermissionDuration;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(It->Get());
		if (PC)
		{
			PC->ViewStartSpot();
		}
	}

	if (Super::UTIsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(TEXT(""), GetWorld()->GetMapName());
	}
}

void AUTGauntletGame::SetMatchState(FName NewState)
{
	if (MatchState == NewState) return;
	Super::SetMatchState(NewState);

	if (MatchState == MatchState::GauntletScoreSummary)
	{
		ForceEndOfRound();
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTGauntletGame::EndScoreSummary, ScoreSummaryDuration, false);
		if (GauntletGameState->CTFRound > 0)
		{
			//BroadcastLocalized(this, UUTGauntletGameMessage::StaticClass(), 4, GauntletGameState->WinnerPlayerState, nullptr, nullptr);
		}
	}
	else if (MatchState == MatchState::GauntletFadeToBlack)
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTGauntletGame::EndFadeToBlack, FadeToBlackDuration, false);
	}
	else if (MatchState == MatchState::GauntletRoundAnnounce)
	{
		BroadcastLocalized(this, UUTGauntletGameMessage::StaticClass(), 7, nullptr, nullptr, nullptr);

		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTGauntletGame::EndRoundAnnounce, RoundAnnounceDuration, false);
	}

}

void AUTGauntletGame::EndScoreSummary()
{
	AUTTeamInfo* WinningTeam = nullptr;
	for (int32 i=0; i < GauntletGameState->Teams.Num(); i++)
	{
		if (GauntletGameState->Teams[i]->Score >= GoalScore)
		{
			WinningTeam = GauntletGameState->Teams[i];
			break;		
		}
	}

	if (WinningTeam == nullptr)
	{
		// Start the transition to the next round
		SetMatchState(MatchState::GauntletFadeToBlack);
	}
	else
	{
		// Typically, find the best player on this team but for now focus on the last cap

		EndGame(GauntletGameState->LastScorer, FName(TEXT("scorelimit")));
	}
}

void AUTGauntletGame::EndFadeToBlack()
{
	// Kill all pawns in the world.
	RemoveAllPawns();
	SetMatchState(MatchState::GauntletRoundAnnounce);
}

void AUTGauntletGame::EndRoundAnnounce()
{
	SetMatchState(MatchState::MatchExitingIntermission);
}


void AUTGauntletGame::ScoreObject_Implementation(AUTCarriedObject* GameObject, AUTCharacter* HolderPawn, AUTPlayerState* Holder, FName Reason)
{
	Super::ScoreObject_Implementation(GameObject, HolderPawn, Holder, Reason);

	// Clear the defense effects

	AUTCTFFlagBase* DestinationBase = GauntletGameState->GetFlagBase(0);
	DestinationBase->ClearDefenseEffect();
	DestinationBase = GauntletGameState->GetFlagBase(1);
	DestinationBase->ClearDefenseEffect();


	GauntletGameState->LastScorer = Holder;

	DestinationBase = GauntletGameState->GetFlagBase(1 - GameObject->GetTeamNum());

	// Place the flag at the new base..
	GauntletGameState->Flag->FinalRestingPlace(DestinationBase);

	// We need to add a flag capture emote
	if (HolderPawn != nullptr)
	{
		FVector BaseLocation = DestinationBase->GetActorLocation();
		FVector PawnLocation = HolderPawn->GetActorLocation();
		FRotator NewPawnRotation = FRotator(0.0f,0.0f,0.0f);

		NewPawnRotation.Yaw =  (BaseLocation - PawnLocation).ToOrientationRotator().Yaw;

		HolderPawn->SetActorLocationAndRotation(BaseLocation + FVector(0.0f,0.0f,96.0f), NewPawnRotation);
		Holder->PlayTauntByIndex(0);
	}

}

void AUTGauntletGame::HandleFlagCapture(AUTCharacter* HolderPawn, AUTPlayerState* Holder)
{
	FlagScorer = Holder;
	CheckScore(Holder);
	if (GauntletGameState && GauntletGameState->IsMatchInProgress())
	{

		GauntletGameState->WinningTeam = Holder->Team;
		GauntletGameState->WinnerPlayerState = Holder;

		Holder->AddCoolFactorEvent(400.0f);
		PickMostCoolMoments(true);
		SetMatchState(MatchState::GauntletScoreSummary);
	}
}

void AUTGauntletGame::ForceEndOfRound()
{
	// Tell the controllers to look at own team flag
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
		if (PC != NULL)
		{
			int32 TeamToWatch = IntermissionTeamToView(PC);
			PC->SetViewTarget(CTFGameState->FlagBases[1-TeamToWatch]);
		}
	}

	// Freeze all of the pawns
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		AUTCharacter* UTCharacter = Cast<AUTCharacter>(It->Get());
		if (UTCharacter)
		{
			UTCharacter->TurnOff();
		}
	}

	GauntletGameState->bIsAtIntermission = true;

	// inform actors of intermission start
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		if (It->GetClass()->ImplementsInterface(UUTIntermissionBeginInterface::StaticClass()))
		{
			IUTIntermissionBeginInterface::Execute_IntermissionBegin(*It);
		}
	}
}

void AUTGauntletGame::BroadcastCTFScore(APlayerState* ScoringPlayer, AUTTeamInfo* ScoringTeam, int32 OldScore)
{
	BroadcastLocalized(this, UUTGauntletGameMessage::StaticClass(), 4, ScoringPlayer, NULL, ScoringTeam);
}
	
bool AUTGauntletGame::IsPlayerOnLifeLimitedTeam(AUTPlayerState* PlayerState) const
{
	return true;
}

void AUTGauntletGame::ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	Super::ScoreKill_Implementation(Killer, Other, KilledPawn, DamageType);
	AUTPlayerState* KillerPlayerState = Killer ? Cast<AUTPlayerState>(Killer->PlayerState) : nullptr;
	AUTPlayerState* VictimPlayerState = KilledPawn ? Cast<AUTPlayerState>(KilledPawn->PlayerState) : nullptr;
	
	if (KillerPlayerState && VictimPlayerState && !GauntletGameState->OnSameTeam(KillerPlayerState, VictimPlayerState))
	{
		AUTPlayerState* OldestPlayer = nullptr;

		uint8 KillerTeamNum = KillerPlayerState->GetTeamNum();
		for (APlayerState* PS : GauntletGameState->PlayerArray)
		{
			AUTPlayerState* UTPlayerState = Cast<AUTPlayerState>(PS);

			if (UTPlayerState && GauntletGameState->OnSameTeam(KillerPlayerState, UTPlayerState) && !UTPlayerState->bIsInactive && UTPlayerState->bOutOfLives)
			{
				float TimeSeconds = GetWorld()->GetTimeSeconds();
				if (OldestPlayer == nullptr || ( TimeSeconds - OldestPlayer->LastSpawnTime < TimeSeconds - UTPlayerState->LastSpawnTime))
				{
					OldestPlayer = UTPlayerState;
				}
			}
		}

		if (OldestPlayer != nullptr)
		{

			OldestPlayer->RemainingLives++;
			if (OldestPlayer->bOutOfLives)
			{
				OldestPlayer->SetOutOfLives(false);
				OldestPlayer->ForceNetUpdate();

				RestartPlayer(Cast<AController>(OldestPlayer->GetOwner()));
				AUTPlayerController* UTPC = Cast<AUTPlayerController>(OldestPlayer->GetOwner());
				UTPC->ClientReceiveLocalizedMessage(UUTGauntletGameMessage::StaticClass(), 6, nullptr, nullptr, nullptr);			
			}
		}
	}
}


void AUTGauntletGame::PlayEndOfMatchMessage()
{
	if (UTGameState && UTGameState->WinningTeam)
	{
		BroadcastLocalized(this, UUTGauntletGameMessage::StaticClass(), 5, GauntletGameState->LastScorer, NULL, UTGameState->WinningTeam);
	}
}