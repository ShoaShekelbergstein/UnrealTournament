// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLineUpHelper.h"
#include "UTLineUpZone.h"
#include "Net/UnrealNetwork.h"
#include "UTWeaponAttachment.h"
#include "UTHUD.h"
#include "UTCTFGameState.h"
#include "UTCTFRoundGameState.h"
#include "UTCTFRoundGame.h"
#include "UTCTFGameMode.h"
#include "UTFlagRunGameState.h"
#include "UTPlayerState.h"

AUTLineUpHelper::AUTLineUpHelper(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsPlacingPlayers = false;
	bAlwaysRelevant = true;

	TimerDelayForIntro = 0.f;
	TimerDelayForIntermission = 9.f;
	TimerDelayForEndMatch = 9.f;
}

void AUTLineUpHelper::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTLineUpHelper, bIsActive);
	DOREPLIFETIME(AUTLineUpHelper, LastActiveType);
}

void AUTLineUpHelper::HandleLineUp(LineUpTypes ZoneType)
{
	AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
	if (UTGS && (UTGS->GetAppropriateSpawnList(ZoneType) != nullptr))
	{
		LastActiveType = ZoneType;

		if (GetWorld())
		{
			if (ZoneType == LineUpTypes::Intro)
			{
				HandleIntro(ZoneType);
			}
			else if (ZoneType == LineUpTypes::Intermission)
			{
				HandleIntermission(ZoneType);
			}
			else if (ZoneType == LineUpTypes::PostMatch)
			{
				HandleEndMatchSummary(ZoneType);
			}
		}
	}
}

void AUTLineUpHelper::HandleIntro(LineUpTypes IntroType)
{
	bIsActive = true;
	MovePlayersDelayed(IntroType, IntroHandle, TimerDelayForIntro);
}

void AUTLineUpHelper::CleanUp()
{
	if (bIsActive)
	{
		if (SelectedCharacter.IsValid())
		{
			SelectedCharacter.Reset();
		}

		bIsActive = false;
		LastActiveType = LineUpTypes::Invalid;

		if (GetWorld())
		{
			for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
			{
				AUTPlayerController* UTPC = Cast<AUTPlayerController>(*Iterator);
				if (UTPC)
				{
					CleanUpPlayerAfterLineUp(UTPC);
				}
			}

			AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
			if (UTGS)
			{
				UTGS->LeadLineUpPlayer = nullptr;

				//If we are in the end game / map vote we don't need to destroy our spawned clones and should 
				//let them stick around and look fancy while voting / stats are being displayed
				if ((UTGS->GetMatchState() != MatchState::WaitingPostMatch) && (UTGS->GetMatchState() != MatchState::MapVoteHappening))
				{
					DestroySpawnedClones();
				}
			}
		}
	}
}
		
void AUTLineUpHelper::CleanUpPlayerAfterLineUp(AUTPlayerController* UTPC)
{
	if (UTPC != nullptr)
	{
		//Clear any active taunts
		AUTCharacter* UTChar = UTPC->GetUTCharacter();
		if (UTChar && UTChar->CurrentTaunt && UTChar->GetMesh())
		{
			UAnimInstance* AnimInstance = UTChar->GetMesh()->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Stop(0.0f, UTChar->CurrentTaunt);
			}
		}

		if (UTPC->UTPlayerState)
		{
			UTPC->UTPlayerState->EmoteReplicationInfo.EmoteCount = 0;
		}
		
		UTPC->SetEmoteSpeed(1.0f);
		UTPC->FlushPressedKeys();
	}
}

void AUTLineUpHelper::DestroySpawnedClones()
{
	if (PlayerPreviewCharacters.Num() > 0)
	{
		for (int index = 0; index < PlayerPreviewCharacters.Num(); ++index)
		{
			if (PlayerPreviewCharacters[index])
			{
				PlayerPreviewCharacters[index]->Destroy();
			}
		}
		PlayerPreviewCharacters.Empty();
	}

	if (PreviewWeapons.Num() > 0)
	{
		for (int index = 0; index < PreviewWeapons.Num(); ++index)
		{
			if (PreviewWeapons[index])
			{
				PreviewWeapons[index]->Destroy();
			}
		}
		PreviewWeapons.Empty();
	}
}

void AUTLineUpHelper::HandleIntermission(LineUpTypes IntermissionType)
{
	MovePlayersDelayed(IntermissionType, IntermissionHandle, TimerDelayForIntermission);
}

void AUTLineUpHelper::MovePlayersDelayed(LineUpTypes ZoneType, FTimerHandle& TimerHandleToStart, float TimeDelay)
{
	GetWorld()->GetTimerManager().ClearTimer(IntroHandle);
	GetWorld()->GetTimerManager().ClearTimer(IntermissionHandle);
	GetWorld()->GetTimerManager().ClearTimer(MatchSummaryHandle);
	
	AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());

	if ((TimeDelay > SMALL_NUMBER))
	{
		SetupDelayedLineUp();
		GetWorld()->GetTimerManager().SetTimer(TimerHandleToStart, FTimerDelegate::CreateUObject(this, &AUTLineUpHelper::MovePlayers, ZoneType), TimeDelay, false);
	}
	else
	{
		MovePlayers(ZoneType);
	}
}

void AUTLineUpHelper::SetupDelayedLineUp()
{
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>(*Iterator);
		if (UTPC)
		{
			AUTCharacter* UTChar = Cast<AUTCharacter>(UTPC->GetPawn());
			if (UTChar)
			{
				UTChar->TurnOff();
				ForceCharacterAnimResetForLineUp(UTChar);
			}

			UTPC->FlushPressedKeys();

			UTPC->ClientPrepareForLineUp();
		}
	}
}

void AUTLineUpHelper::OnRep_LineUpInfo()
{
	//Make sure that we have received both bIsActive and LastActiveType before calling ClientSetActiveLineUp
	//This means that we either have a valid LastActiveType and bIsActive is true, or an Invalid LastActiveType and bIsActive is false
	if ( GetWorld() && ((bIsActive && (LastActiveType != LineUpTypes::Invalid) && (LastActiveType != LineUpTypes::None)) || (!bIsActive && LastActiveType == LineUpTypes::Invalid)) ) 
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>(GetWorld()->GetFirstPlayerController());
		if (UTPC)
		{
			UTPC->ClientSetActiveLineUp();
		}
	}
}

void AUTLineUpHelper::MovePlayers(LineUpTypes ZoneType)
{	
	static const FName NAME_LineUpCam = FName(TEXT("LineUpCam"));
	bIsPlacingPlayers = true;
	bIsActive = true;

	if (GetWorld() && GetWorld()->GetAuthGameMode())
	{
		AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
		
		//Setup LineUp weapon. Favorite weapon if set, current weapon otherwise
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* UTPC = Cast<AUTPlayerController>(*Iterator);
			if (UTPC)
			{
				AUTPlayerState* UTPS = Cast<AUTPlayerState>(UTPC->PlayerState);
				AUTCharacter* UTChar = Cast<AUTCharacter>(UTPC->GetPawn());
				if (UTPS && UTChar)
				{
					UTPS->LineUpWeapon = (UTPS->FavoriteWeapon != NULL)? UTPS->FavoriteWeapon : UTChar->GetWeaponClass();
				}
			}
		}

		if (UTGM)
		{
			UTGM->RemoveAllPawns();
		}

		//Go through all controllers and spawn/respawn pawns
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* C = Cast<AController>(*Iterator);
			if (C)
			{
				AUTCharacter* UTChar = Cast<AUTCharacter>(C->GetPawn());
				if (!UTChar || UTChar->IsDead() || UTChar->IsRagdoll())
				{
					if (C->GetPawn())
					{
						C->UnPossess();
					}
					UTGM->RestartPlayer(C);
					if (C->GetPawn())
					{
						UTChar = Cast<AUTCharacter>(C->GetPawn());
					}
				}
					
				if (UTChar && !UTChar->IsDead())
				{
					PlayerPreviewCharacters.Add(UTChar);
				}
				
				AUTPlayerController* UTPC = Cast<AUTPlayerController>(C);
				if (UTPC)
				{
					UTPC->SetCameraMode(NAME_LineUpCam);
				}
			}
		}

		FlagFixUp();
		SortPlayers();
		MovePreviewCharactersToLineUpSpawns(ZoneType);

		//Setup LeadPlayer, not done in Sort because players will be deleted as part of MovePreivewCharactersToLineUpSpawns
		if (PlayerPreviewCharacters.Num() > 0)
		{
			AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
			if (UTGS)
			{
				UTGS->LeadLineUpPlayer = Cast<AUTPlayerState>(PlayerPreviewCharacters[0]->PlayerState);
			}
		}

		//Go back through characters now that they are moved and turn them off
		for (AUTCharacter* UTChar : PlayerPreviewCharacters)
		{
			UTChar->TurnOff();
			UTChar->DeactivateSpawnProtection();
			ForceCharacterAnimResetForLineUp(UTChar);
			SpawnPlayerWeapon(UTChar);
		}
	}

	bIsPlacingPlayers = false;
}

void AUTLineUpHelper::FlagFixUp()
{
	if (GetWorld() && GetWorld()->GetAuthGameMode())
	{
		AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
		if (UTGM)
		{
			AUTCTFFlag* OffenseFlag = nullptr;
			AUTFlagRunGameState* UTFRGS = UTGM->GetGameState<AUTFlagRunGameState>();
			if (UTFRGS)
			{
				OffenseFlag = UTFRGS->GetOffenseFlag();
			}

			if (OffenseFlag)
			{
				AController* FlagController = nullptr;
				if (OffenseFlag->Holder)
				{
					FlagController = Cast<AController>(OffenseFlag->Holder->GetOwner());
				}
				else if (OffenseFlag->LastHolder)
				{
					FlagController = Cast<AController>(OffenseFlag->LastHolder->GetOwner());
				}

				if (FlagController && FlagController->GetPawn())
				{
					OffenseFlag->SetHolder(Cast<AUTCharacter>(FlagController->GetPawn()));
				}
				else
				{
					OffenseFlag->Destroy();
				}
			}
		}
	}
}

void AUTLineUpHelper::SpawnPlayerWeapon(AUTCharacter* UTChar)
{
	//If we already have a weapon attachment, keep that
	if (UTChar)
	{
		AUTPlayerState* UTPS = Cast<AUTPlayerState>(UTChar->PlayerState);
		TSubclassOf<AUTWeapon> WeaponClass = NULL;

		if (UTPS)
		{
			WeaponClass = UTPS->LineUpWeapon ? UTPS->LineUpWeapon->GetDefaultObject<AUTWeapon>()->GetClass() : NULL;
		}

		//Default to Link Gun
		if (!WeaponClass)
		{
			WeaponClass = LoadClass<AUTWeapon>(NULL, TEXT("/Game/RestrictedAssets/Weapons/LinkGun/BP_LinkGun.BP_LinkGun_C"), NULL, LOAD_None, NULL);
		}
		
		//Remove all inventory so that when we add this weapon in, it is equipped.
		UTChar->DiscardAllInventory();
		
		FActorSpawnParameters WeaponSpawnParams;
		WeaponSpawnParams.Instigator = UTChar;
		WeaponSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		WeaponSpawnParams.bNoFail = true;

		AUTWeapon* PreviewWeapon = GetWorld()->SpawnActor<AUTWeapon>(WeaponClass, FVector(0, 0, 0), FRotator(0, 0, 0), WeaponSpawnParams);
		if (PreviewWeapon)
		{
			PreviewWeapons.Add(PreviewWeapon);
			
			PreviewWeapon->bAlwaysRelevant = true;
			PreviewWeapon->SetReplicates(true);
			UTChar->AddInventory(PreviewWeapon, true);
			
			//Bots will not auto-switch to new weapon
			AUTBot* BotController = Cast<AUTBot>(UTChar->Controller);
			if (BotController)
			{
				BotController->SwitchToBestWeapon();
			}
		}
	}
}


void AUTLineUpHelper::ForceCharacterAnimResetForLineUp(AUTCharacter* UTChar)
{
	if (UTChar && UTChar->GetMesh())
	{
		//Want to still update the animations and bones even though we have turned off the Pawn, so re-enable those.
		UTChar->GetMesh()->bPauseAnims = false;
		UTChar->GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	}
}

void AUTLineUpHelper::MovePreviewCharactersToLineUpSpawns(LineUpTypes LineUpType)
{
	AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
	AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());

	if (UTGS && UTGM)
	{
		AUTTeamGameMode* TeamGM = Cast<AUTTeamGameMode>(UTGM);
		AUTCTFRoundGame* CTFGM = Cast<AUTCTFRoundGame>(UTGM);
		AUTCTFGameState* CTFGS = Cast<AUTCTFGameState>(UTGS);

		TArray<FTransform> RedOrWinnerSpawns;
		TArray<FTransform> BlueOrLoserSpawns;
		TArray<FTransform> FFASpawns;

		int RedOrWinIndex = 0;
		int BlueOrLoseIndex = 0;
		int FFAIndex = 0;

		int RedOrWinningTeamNumber = 0;
		int BlueOrLosingTeamNumber = 1;

		//Spawn using Winning / Losing teams instead of team color based teams. This means the red list = winning team and blue list = losing team.
		if (TeamGM && TeamGM->UTGameState && (LineUpType == LineUpTypes::PostMatch || LineUpType == LineUpTypes::Intermission))
		{
			if (TeamGM->UTGameState->WinningTeam)
			{
				RedOrWinningTeamNumber = TeamGM->UTGameState->WinningTeam->GetTeamNum();
				BlueOrLosingTeamNumber = 1 - RedOrWinningTeamNumber;
			}
			else if (TeamGM->UTGameState->ScoringPlayerState)
			{
				RedOrWinningTeamNumber = TeamGM->UTGameState->ScoringPlayerState->GetTeamNum();
				BlueOrLosingTeamNumber = 1 - RedOrWinningTeamNumber;
			}
			else if (CTFGM && CTFGM->FlagScorer)
			{
				RedOrWinningTeamNumber = CTFGM->FlagScorer->GetTeamNum();
				BlueOrLosingTeamNumber = 1 - RedOrWinningTeamNumber;
			}
			else if (CTFGS && CTFGS->GetScoringPlays().Num() > 0)
			{
				const TArray<const FCTFScoringPlay>& ScoringPlays = CTFGS->GetScoringPlays();
				const FCTFScoringPlay& WinningPlay = ScoringPlays.Last();

				if (WinningPlay.Team)
				{
					RedOrWinningTeamNumber = WinningPlay.Team->GetTeamNum();
					BlueOrLosingTeamNumber = 1 - RedOrWinningTeamNumber;
				}
			}
		}

		TArray<AUTCharacter*> PreviewsMarkedForDestroy;
		bool bSkipSpawnPlacement = false;

		AUTLineUpZone* SpawnList = UTGS->GetAppropriateSpawnList(LineUpType);
		if (SpawnList)
		{
			RedOrWinnerSpawns = SpawnList->RedAndWinningTeamSpawnLocations;
			BlueOrLoserSpawns = SpawnList->BlueAndLosingTeamSpawnLocations;
			FFASpawns = SpawnList->FFATeamSpawnLocations;
		
			for (AUTCharacter* PreviewChar : PlayerPreviewCharacters)
			{
				FTransform SpawnTransform = SpawnList->GetActorTransform();

				if ((PreviewChar->GetTeamNum() == RedOrWinningTeamNumber) && (RedOrWinnerSpawns.Num() > RedOrWinIndex))
				{
					SpawnTransform = RedOrWinnerSpawns[RedOrWinIndex] * SpawnTransform;
					++RedOrWinIndex;
				}
				else if ((PreviewChar->GetTeamNum() == BlueOrLosingTeamNumber) && (BlueOrLoserSpawns.Num() > BlueOrLoseIndex))
				{
					SpawnTransform = BlueOrLoserSpawns[BlueOrLoseIndex] * SpawnTransform;
					++BlueOrLoseIndex;
				}
				else if (FFASpawns.Num() > FFAIndex)
				{
					SpawnTransform = FFASpawns[FFAIndex] * SpawnTransform;
					++FFAIndex;
				}
				//If they are not part of the line up display... remove them
				else
				{
					PreviewsMarkedForDestroy.Add(PreviewChar);
				}

				PreviewChar->bIsTranslocating = true; // Hack to get rid of teleport effect

				PreviewChar->TeleportTo(SpawnTransform.GetTranslation(), SpawnTransform.GetRotation().Rotator(), false, true);
				PreviewChar->Controller->SetControlRotation(SpawnTransform.GetRotation().Rotator());
				PreviewChar->Controller->ClientSetRotation(SpawnTransform.GetRotation().Rotator());

				PreviewChar->bIsTranslocating = false;
			}
		}
		else
		{
			//UTGM->HandleDefaultLineupSpawns(LineUpType, PlayerPreviewCharacters, PreviewsMarkedForDestroy);
		}

		for (AUTCharacter* DestroyCharacter : PreviewsMarkedForDestroy)
		{
			//Destroy any carried objects before destroying pawn.
			if (DestroyCharacter->GetCarriedObject())
			{
				DestroyCharacter->GetCarriedObject()->Destroy();
			}

			AUTPlayerController* UTPC = Cast<AUTPlayerController>(DestroyCharacter->GetController());
			if (UTPC)
			{
				UTPC->UnPossess();
			}

			PlayerPreviewCharacters.Remove(DestroyCharacter);
			DestroyCharacter->Destroy();
		}
		PreviewsMarkedForDestroy.Empty();
	}
}

LineUpTypes AUTLineUpHelper::GetLineUpTypeToPlay(UWorld* World)
{
	LineUpTypes ReturnZoneType = LineUpTypes::Invalid;

	AUTGameState* UTGS = Cast<AUTGameState>(World->GetGameState());
	if (UTGS == nullptr)
	{
		return ReturnZoneType;
	}

	AUTCTFGameState* UTCTFGS = Cast<AUTCTFGameState>(UTGS);
	AUTCTFRoundGameState* UTCTFRoundGameGS = Cast<AUTCTFRoundGameState>(UTGS);

	AUTCTFRoundGame* CTFGM = Cast<AUTCTFRoundGame>(World->GetAuthGameMode());

	//The first intermission of CTF Round Game is actually an intro
	if ((UTGS->GetMatchState() == MatchState::CountdownToBegin) || (UTGS->GetMatchState() == MatchState::PlayerIntro) || ((UTGS->GetMatchState() == MatchState::MatchIntermission) && UTCTFRoundGameGS && (UTCTFRoundGameGS->CTFRound == 1) && (!UTCTFRoundGameGS || UTCTFRoundGameGS->GetScoringPlays().Num() == 0)))
	{
		ReturnZoneType = LineUpTypes::Intro;
	}

	else if (UTGS->GetMatchState() == MatchState::MatchIntermission)
	{
		ReturnZoneType = LineUpTypes::Intermission;
	}

	else if (UTGS->GetMatchState() == MatchState::WaitingPostMatch)
	{
		ReturnZoneType = LineUpTypes::PostMatch;
	}

	return ReturnZoneType;
}


static int32 WeaponIndex = 0;

void AUTLineUpHelper::HandleEndMatchSummary(LineUpTypes SummaryType)
{
	MovePlayersDelayed(SummaryType, MatchSummaryHandle, TimerDelayForEndMatch);
}

void AUTLineUpHelper::SortPlayers()
{
	bool(*SortFunc)(const AUTCharacter&, const AUTCharacter&);
	SortFunc = [](const AUTCharacter& A, const AUTCharacter& B)
	{
		AUTPlayerState* PSA = Cast<AUTPlayerState>(A.PlayerState);
		AUTPlayerState* PSB = Cast<AUTPlayerState>(B.PlayerState);

		AUTCTFFlag* AUTFlagA = nullptr;
		AUTCTFFlag* AUTFlagB = nullptr;
		if (PSA)
		{
			AUTFlagA = Cast<AUTCTFFlag>(PSA->CarriedObject);
		}
		if (PSB)
		{
			AUTFlagB = Cast<AUTCTFFlag>(PSB->CarriedObject);
		}

		return !PSB || (AUTFlagA) || (PSA && (PSA->Score > PSB->Score) && !AUTFlagB);
	};
	PlayerPreviewCharacters.Sort(SortFunc);
}

void AUTLineUpHelper::OnPlayerChange()
{
	if (GetWorld())
	{
		AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
		if (UTGM && UTGM->UTGameState)
		{
			if (UTGM->UTGameState->GetMatchState() == MatchState::WaitingToStart)
			{
				ClientUpdatePlayerClones();
			}
		}
	}
}

void AUTLineUpHelper::ClientUpdatePlayerClones()
{

}

bool AUTLineUpHelper::CanInitiateGroupTaunt(AUTPlayerState* PlayerToCheck)
{
	AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
	return (UTGS &&
		    PlayerToCheck &&
			bIsActive &&
			(LastActiveType != LineUpTypes::Invalid) &&
			(LastActiveType != LineUpTypes::None) &&
			(LastActiveType != LineUpTypes::Intro) && //no group taunts during intro line ups
			(PlayerToCheck == UTGS->LeadLineUpPlayer));
}