// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLineUpHelper.h"
#include "UTLineUpZone.h"
#include "Net/UnrealNetwork.h"
#include "UTWeaponAttachment.h"
#include "UTHUD.h"
#include "UTCTFGameState.h"
#include "UTCTFGameMode.h"
#include "UTFlagRunGameState.h"
#include "UTPlayerState.h"
#include "UTCustomMovementTypes.h"
#include "UTPickupWeapon.h"
#include "UTSupplyChest.h"
#include "UTMutator.h"
#include "UTWeap_Redeemer.h"
#include "UTWeap_Enforcer.h"
#include "UTWeap_Translocator.h"

DEFINE_LOG_CATEGORY_STATIC(LogUTLineUp, Log, All);

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

void AUTLineUpHelper::BeginPlay()
{
	Super::BeginPlay();
	
	BuildMapWeaponList();
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
				AUTPlayerState* UTPS = Cast<AUTPlayerState>((*Iterator)->PlayerState);
				if (UTPS)
				{
					UTPS->LineUpLocation = INDEX_NONE;
				}

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

		UTPC->SetViewTarget(UTPC->GetPawn());
		UTPC->SetCountdownCam();
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
			UTPC->FlushPressedKeys();
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
	bIsPlacingPlayers = true;
	bIsActive = true;

	if (GetWorld() && GetWorld()->GetAuthGameMode())
	{
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* UTPC = Cast<AUTPlayerController>(*Iterator);
			if (UTPC)
			{
				UTPC->FlushPressedKeys();
				UTPC->ClientPrepareForLineUp();
			}
		}

		AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
		
		//Setup LineUp weapon. Favorite weapon if set, current weapon otherwise
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* C = Cast<AController>(*Iterator);
			if (C)
			{
				AUTPlayerState* UTPS = Cast<AUTPlayerState>(C->PlayerState);
				if (UTPS)
				{
					UTPS->LineUpWeapon = (UTPS->FavoriteWeapon != NULL)? UTPS->FavoriteWeapon : NULL;
					
					//Either we didn't have an existing favorite weapon, or its not valid in this map
					if (UTPS->LineUpWeapon == NULL || (!MapWeaponTypeList.Contains(UTPS->LineUpWeapon)))
					{
						AUTCharacter* UTChar = Cast<AUTCharacter>(C->GetPawn());
						UTPS->LineUpWeapon = (UTChar) ? UTChar->GetWeaponClass() : nullptr;
					}
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
		for (int32 PlayerIndex = 0; PlayerIndex < PlayerPreviewCharacters.Num(); ++PlayerIndex)
		{
			AUTCharacter* UTChar = PlayerPreviewCharacters[PlayerIndex];
			if (UTChar)
			{
				UTChar->TurnOff();
				UTChar->DeactivateSpawnProtection();
				ForceCharacterAnimResetForLineUp(UTChar);
				SpawnPlayerWeapon(UTChar);

				//Set each player's position in the line up
				AUTPlayerState* UTPlayerState = Cast<AUTPlayerState>(UTChar->PlayerState);
				if (UTPlayerState)
				{
					UTPlayerState->LineUpLocation = PlayerIndex;
				}
			}
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
	if (UTChar)
	{
		AUTPlayerState* UTPS = Cast<AUTPlayerState>(UTChar->PlayerState);
		TSubclassOf<AUTWeapon> WeaponClass = NULL;

		if (UTPS)
		{
			WeaponClass = UTPS->LineUpWeapon ? UTPS->LineUpWeapon->GetDefaultObject<AUTWeapon>()->GetClass() : NULL;
		}

		//If we already have a weapon attachment, keep that
		if (!WeaponClass)
		{
			//Try and pick a random weapon available on the map for pickup
			if (MapWeaponTypeList.Num() > 0)
			{
				int32 WeaponIndex = FMath::RandHelper(MapWeaponTypeList.Num());

				if (MapWeaponTypeList[WeaponIndex] != NULL)
				{
					WeaponClass = MapWeaponTypeList[WeaponIndex];
				}
			}
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

void AUTLineUpHelper::BuildMapWeaponList()
{
	//All weapon spawning is on the server, so Clients don't need a MapWeaponList
	if (GetNetMode() != NM_Client)
	{
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			//Weapon Pickups
			AUTPickupWeapon* Pickup = Cast<AUTPickupWeapon>(*It);
			if (Pickup && (Pickup->WeaponType != NULL))
			{
				MapWeaponTypeList.AddUnique(Pickup->WeaponType);
			}

			//Supply Chest Weapons
			AUTSupplyChest* UTSupplyChest = Cast<AUTSupplyChest>(*It);
			if (UTSupplyChest && UTSupplyChest->bIsActive && (UTSupplyChest->Weapons.Num() > 0))
			{
				for (TSubclassOf<AUTWeapon>& Weapon : UTSupplyChest->Weapons)
				{
					if (Weapon != NULL)
					{
						MapWeaponTypeList.AddUnique(Weapon);
					}
				}
			}
		}

		//Default Inventory Weapons
		AUTGameMode* GameMode = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (GameMode)
		{
			//From Character BP
			AUTCharacter* UTChar = Cast<AUTCharacter>(GameMode->DefaultPawnClass->GetDefaultObject());
			if (UTChar)
			{
				for (TSubclassOf<AUTInventory>& Item : UTChar->DefaultCharacterInventory)
				{
					TSubclassOf<AUTWeapon> Weapon = Item.Get();
					if (Weapon != NULL)
					{
						MapWeaponTypeList.AddUnique(Weapon);
					}
				}
			}

			//From GameMode
			for (TSubclassOf<AUTInventory>& Item : GameMode->DefaultInventory)
			{
				TSubclassOf<AUTWeapon> Weapon = Item.Get();
				if (Weapon != NULL)
				{
					MapWeaponTypeList.AddUnique(Weapon);
				}
			}
		}

		//Remove invalid weapons for line-ups
		AUTGameState* UTGS = GetWorld()->GetGameState<AUTGameState>();
		if (GameMode)
		{
			TArray<TSubclassOf<AUTWeapon>> InvalidWeaponsToRemove;
			for (TSubclassOf<AUTWeapon>& Weapon : MapWeaponTypeList)
			{
				//Remove anything mutators won't allow
				if ((GameMode->BaseMutator) && (!GameMode->BaseMutator->CheckRelevance(Weapon->GetDefaultObject<AActor>())))
				{
					InvalidWeaponsToRemove.Add(Weapon);
				}
				//Translocator
				else if (Weapon->IsChildOf(AUTWeap_Translocator::StaticClass()))
				{
					InvalidWeaponsToRemove.Add(Weapon);
				}
				//Enforcer
				else if (Weapon->IsChildOf(AUTWeap_Enforcer::StaticClass()))
				{
					InvalidWeaponsToRemove.Add(Weapon);
				}
				//Redeemer
				else if (Weapon->IsChildOf(AUTWeap_Redeemer::StaticClass()))
				{
					InvalidWeaponsToRemove.Add(Weapon);
				}
			}

			//If we are about to delete all the weapons, try and keep 1 in the list.
			//For this to happen the mutator or map must lack all weapons but the Enforcer / Translocator / Redeemer. Lets try and re-validate 1 of those to show.
			if ((InvalidWeaponsToRemove.Num() == MapWeaponTypeList.Num()) && (MapWeaponTypeList.Num() > 1))
			{
				TSubclassOf<AUTWeapon> RevalidateWeapon = nullptr;
				for (TSubclassOf<AUTWeapon>& InvalidWeapon : InvalidWeaponsToRemove)
				{
					//Recheck if the item is ok with the mutator, if so, don't remove it
					if ((GameMode->BaseMutator) && (GameMode->BaseMutator->CheckRelevance(InvalidWeapon->GetDefaultObject<AActor>())))
					{
						RevalidateWeapon = InvalidWeapon;
						break;
					}
				}

				if (RevalidateWeapon)
				{
					InvalidWeaponsToRemove.Remove(RevalidateWeapon);
				}
				else
				{
					UE_LOG(LogUTLineUp, Warning, TEXT("No valid weapons found for line-up!"));
				}
			}

			for (TSubclassOf<AUTWeapon>& InvalidWeapon : InvalidWeaponsToRemove)
			{
				// Make sure at least 1 weapon is always in the list
				if (MapWeaponTypeList.Num() > 1)
				{
					MapWeaponTypeList.Remove(InvalidWeapon);
				}
			}
		}
	}
}

void AUTLineUpHelper::ForceCharacterAnimResetForLineUp(AUTCharacter* UTChar)
{
	if (UTChar)
	{
		UUTCharacterMovement* UTCM = Cast<UUTCharacterMovement>(UTChar->GetMovementComponent());
		if (UTCM)
		{
			UTCM->OnLineUp();

			//Need to turn on collision so that the line-up movement mode can find the floor and reset it
			bool bOriginalCollisionSetting = UTChar->GetActorEnableCollision();
			UTChar->SetActorEnableCollision(true);

			// This movement mode is tied to LineUp specific anims.
			UTCM->SetMovementMode(MOVE_Custom, CUSTOMMOVE_LineUp);
		
			//Reset collision to whatever it was before line-up
			UTChar->SetActorEnableCollision(bOriginalCollisionSetting);
		}

		if (UTChar->GetMesh())
		{
			//Want to still update the animations and bones even though we have turned off the Pawn, so re-enable those.
			UTChar->GetMesh()->bPauseAnims = false;
			UTChar->GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;

			//Turn off local physics sim and collisions during line-ups
			UTChar->GetMesh()->SetSimulatePhysics(false);
			UTChar->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AUTLineUpHelper::MovePreviewCharactersToLineUpSpawns(LineUpTypes LineUpType)
{
	AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
	AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());

	if (UTGS && UTGM)
	{
		AUTTeamGameMode* TeamGM = Cast<AUTTeamGameMode>(UTGM);

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
			uint8 WinningTeamNum = TeamGM->GetWinningTeamForLineUp();
			if (WinningTeamNum != 255)
			{
				RedOrWinningTeamNumber = WinningTeamNum;
				BlueOrLosingTeamNumber = 1 - RedOrWinningTeamNumber;
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
					//If we are in a team game mode and its intermission or post match, still only show winners even in FFA spawns.
					if (UTGM && UTGM->bTeamGame && (LineUpType == LineUpTypes::PostMatch || LineUpType == LineUpTypes::Intermission) && (PreviewChar->GetTeamNum() != RedOrWinningTeamNumber))
					{
						PreviewsMarkedForDestroy.Add(PreviewChar);
						continue;
					}

					SpawnTransform = FFASpawns[FFAIndex] * SpawnTransform;
					++FFAIndex;
				}
				//If they are not part of the line up display... remove them
				else
				{
					PreviewsMarkedForDestroy.Add(PreviewChar);
					continue;
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