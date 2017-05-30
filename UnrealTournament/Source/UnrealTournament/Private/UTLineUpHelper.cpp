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

void AUTLineUpHelper::InitializeLineUp(LineUpTypes LineUpType)
{
	ActiveType = LineUpType;
	StartLineUpWithDelay(CalculateLineUpDelay());
}

void AUTLineUpHelper::CalculateLineUpSlots()
{
	AUTGameState* UTGameState = GetWorld()->GetGameState<AUTGameState>();
	if (UTGameState)
	{
		AUTLineUpZone* ZoneToUse = UTGameState->GetAppropriateSpawnList(ActiveType);
		if (ZoneToUse)
		{
			TArray<AController*> UnassignedControllers;
			for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
			{
				UnassignedControllers.Add(Iterator->Get());
			}
			SortControllers(UnassignedControllers);

			AUTTeamGameMode* TeamGM = Cast<AUTTeamGameMode>(GetWorld()->GetAuthGameMode());
			int Team1Number = 0;
			int Team2Number = 1;

			//Spawn using Winning / Losing teams instead of team color based teams. This means the red list = winning team and blue list = losing team.
			if (TeamGM && TeamGM->UTGameState && (ActiveType == LineUpTypes::PostMatch || ActiveType == LineUpTypes::Intermission))
			{
				uint8 WinningTeamNum = TeamGM->GetWinningTeamForLineUp();
				if (WinningTeamNum != 255)
				{
					Team1Number = WinningTeamNum;
					Team2Number = 1 - WinningTeamNum;
				}
			}

			for (FLineUpSpawn& Spawn : ZoneToUse->SpawnLocations)
			{
				FLineUpSlot NewSlot;
				NewSlot.SpotLocation = Spawn.Location * ZoneToUse->GetActorTransform();

				//Find the highest rated player controller to fill in this spot
				int TeamNumberToFill = -1;
				switch (Spawn.SpawnType)
				{	
					case LineUpSpawnTypes::Team1:
					case LineUpSpawnTypes::WinningTeam:
					{
						TeamNumberToFill = Team1Number;
						break;
					}
					case LineUpSpawnTypes::Team2:
					case LineUpSpawnTypes::LosingTeam:
					{
						TeamNumberToFill = Team2Number;
						break;
					}
					//Do nothing
					case LineUpSpawnTypes::FFA:
					default:
					{
						break;
					}
				}

				// This is a team slot, need to fill with someone on the correct team
				if (TeamNumberToFill >= 0)
				{
					for (int index = 0; index < UnassignedControllers.Num(); ++index)
					{
						const IUTTeamInterface* TeamInterface = Cast<IUTTeamInterface>(UnassignedControllers[index]);
						if (TeamInterface)
						{
							static const int FFATeamNum = 255;
							if ((TeamInterface->GetTeamNum() == TeamNumberToFill) || (TeamInterface->GetTeamNum() == FFATeamNum))
							{
								NewSlot.ControllerInSpot = UnassignedControllers[index];
								UnassignedControllers.RemoveAt(index);

								//Found highest rated fit, don't need to look further
								LineUpSlots.Add(NewSlot);
								break;
							}
						}
					}
				}
				//Just grab highest rated person for this spot as its FFA
				else
				{
					if (UnassignedControllers.Num() > 0)
					{
						NewSlot.ControllerInSpot = UnassignedControllers[0];
						UnassignedControllers.RemoveAt(0);

						LineUpSlots.Add(NewSlot);
					}
				}
			}
		}
	}
}

void AUTLineUpHelper::StartLineUpWithDelay(float TimeDelay)
{
	GetWorld()->GetTimerManager().ClearTimer(DelayedLineUpHandle);
	if ((TimeDelay > SMALL_NUMBER))
	{
		GetWorld()->GetTimerManager().SetTimer(DelayedLineUpHandle, FTimerDelegate::CreateUObject(this, &AUTLineUpHelper::PerformLineUp), TimeDelay, false);
	}
	else
	{
		PerformLineUp();
	}
}

bool AUTLineUpHelper::IsActive()
{
	return bIsActive;
}

float AUTLineUpHelper::CalculateLineUpDelay()
{
	float TimeDelay = 0.0f;
	switch (ActiveType)
	{
		case LineUpTypes::Intro:
		{
			TimeDelay = TimerDelayForIntro;
			break;
		}
		case LineUpTypes::Intermission:
		{
			TimeDelay = TimerDelayForIntermission;
			break;
		}
		case LineUpTypes::PostMatch:
		{
			TimeDelay = TimerDelayForEndMatch;
			break;
		}
	}
	
	return TimeDelay;
}

AUTLineUpHelper::AUTLineUpHelper(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAlwaysRelevant = true;

	TimerDelayForIntro = 0.f;
	TimerDelayForIntermission = 9.f;
	TimerDelayForEndMatch = 9.f;

	bIsPlacingPlayers = false;
	bIsActive = false;
}

void AUTLineUpHelper::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTLineUpHelper, bIsActive);
	DOREPLIFETIME(AUTLineUpHelper, ActiveType);
	DOREPLIFETIME(AUTLineUpHelper, bIsPlacingPlayers);
	DOREPLIFETIME(AUTLineUpHelper, LineUpSlots);
}

void AUTLineUpHelper::BeginPlay()
{
	Super::BeginPlay();
	
	BuildMapWeaponList();
}


void AUTLineUpHelper::CleanUp()
{
	ActiveType = LineUpTypes::Invalid;

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
	}
}

void AUTLineUpHelper::PerformLineUp()
{
	bIsActive = true;

	AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
	if (UTGM)
	{
		//Set the line up weapons before we remove the pawns so that we can see what weapon was equipped going into line up.
		SetLineUpWeapons();
		UTGM->RemoveAllPawns();

		CalculateLineUpSlots();
		SpawnCharactersToSlots();
		SetupCharactersForLineUp();
		FlagFixUp();

		NotifyClientsOfLineUp();
	}
}

void AUTLineUpHelper::SpawnCharactersToSlots()
{	
	AUTGameMode* UTGM = Cast<AUTGameMode>(GetWorld()->GetAuthGameMode());
	if (UTGM)
	{
		//Go through all controllers and spawn/respawn pawns
		bIsPlacingPlayers = true;
		for (FLineUpSlot& Slot : LineUpSlots)
		{
			if (Slot.ControllerInSpot)
			{
				AUTCharacter* UTChar = Cast<AUTCharacter>(Slot.ControllerInSpot->GetPawn());
				if (!UTChar || UTChar->IsDead() || UTChar->IsRagdoll())
				{
					if (Slot.ControllerInSpot->GetPawn())
					{
						Slot.ControllerInSpot->UnPossess();
					}
					UTGM->RestartPlayer(Slot.ControllerInSpot);
					if (Slot.ControllerInSpot->GetPawn())
					{
						UTChar = Cast<AUTCharacter>(Slot.ControllerInSpot->GetPawn());
					}
				}

				if (UTChar && !UTChar->IsDead())
				{
					PlayerPreviewCharacters.Add(UTChar);
					UTChar->TeleportTo(Slot.SpotLocation.GetTranslation(), Slot.SpotLocation.GetRotation().Rotator(), false, true);
					UTChar->Controller->SetControlRotation(Slot.SpotLocation.GetRotation().Rotator());
					UTChar->Controller->ClientSetRotation(Slot.SpotLocation.GetRotation().Rotator());
					UTChar->DeactivateSpawnProtection();

					SpawnPlayerWeapon(UTChar);
				}
			}
		}
		bIsPlacingPlayers = false;
	}
}

void AUTLineUpHelper::SetLineUpWeapons()
{
	//Setup Line-Up weapon to be what is currently equipped
	for (FLineUpSlot& Slot : LineUpSlots)
	{
		if (Slot.ControllerInSpot != nullptr)
		{
			AUTPlayerState* UTPS = Cast<AUTPlayerState>(Slot.ControllerInSpot->PlayerState);
			{
				UTPS->LineUpWeapon = UTPS->LineUpWeapon = (UTPS->FavoriteWeapon != NULL) ? UTPS->FavoriteWeapon : NULL;

				//Either we didn't have an existing favorite weapon, or its not valid in this map
				if (UTPS->LineUpWeapon == NULL || (!MapWeaponTypeList.Contains(UTPS->LineUpWeapon)))
				{
					AUTCharacter* UTChar = Cast<AUTCharacter>(Slot.ControllerInSpot->GetPawn());
					UTPS->LineUpWeapon = (UTChar) ? UTChar->GetWeaponClass() : nullptr;
				}
			}
		}
	}
}

void AUTLineUpHelper::NotifyClientsOfLineUp()
{
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>(*Iterator);
		if (UTPC)
		{
			UTPC->ClientPrepareForLineUp();
		}
	}
}

void AUTLineUpHelper::SetupCharactersForLineUp()
{
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		if (It->IsValid() && !Cast<ASpectatorPawn>(It->Get()))
		{
			// freeze all Pawns on server
			It->Get()->TurnOff();

			AUTCharacter* UTChar = Cast<AUTCharacter>(*It);
			if (UTChar)
			{
				// Setup custom animations
				AUTLineUpHelper::ApplyCharacterAnimsForLineUp(UTChar);

				//Start Intro Anims if they haven't been set
				if (UTChar->ActiveLineUpIntroIndex != GetIntroMontageIndex(UTChar))
				{
					UTChar->ActiveLineUpIntroIndex = GetIntroMontageIndex(UTChar);
					AUTLineUpHelper::PlayIntroForCharacter(UTChar);
				}
			}
		}
	}
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

void AUTLineUpHelper::ApplyCharacterAnimsForLineUp(AUTCharacter* UTChar)
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

UAnimMontage* AUTLineUpHelper::GetIntroMontage(AUTCharacter* UTChar)
{
	if (UTChar)
	{
		AUTPlayerState* UTPS = Cast<AUTPlayerState>(UTChar->PlayerState);
		if (UTPS && (UTPS->TauntClass != NULL))
		{
			AUTTaunt* UTTaunt = Cast<AUTTaunt>(UTPS->TauntClass->GetDefaultObject());
			return UTTaunt ? UTTaunt->TauntMontage : nullptr;
		}
	}

	return nullptr;
}

void AUTLineUpHelper::PlayIntroForCharacter(AUTCharacter* UTChar)
{
	if (UTChar)
	{
		//Initial Spawn Anim
		UAnimInstance* AnimInstance = UTChar->GetMesh()->GetAnimInstance();
		UAnimMontage* SpawnMontage = AUTLineUpHelper::GetIntroMontage(UTChar);
		if (AnimInstance && SpawnMontage)
		{
			AnimInstance->Montage_Play(SpawnMontage);
		}
	}
}

void AUTLineUpHelper::SortControllers(TArray<AController*>& ControllersToSort)
{
	bool(*SortFunc)(AController&, AController&);
	SortFunc = [](AController& A, AController& B)
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
	ControllersToSort.Sort(SortFunc);
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
				//ClientUpdatePlayerClones();
			}
		}
	}
}

bool AUTLineUpHelper::CanInitiateGroupTaunt(AUTPlayerState* PlayerToCheck)
{
	AUTGameState* UTGS = Cast<AUTGameState>(GetWorld()->GetGameState());
	return (UTGS &&
		    PlayerToCheck &&
			(ActiveType != LineUpTypes::Invalid) &&
			(ActiveType != LineUpTypes::None) &&
			(ActiveType != LineUpTypes::Intro) && //no group taunts during intro line ups
			(PlayerToCheck == UTGS->LeadLineUpPlayer));
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

int AUTLineUpHelper::GetIntroMontageIndex(AUTCharacter* UTChar)
{
	if (UTChar && UTChar->GetWorld())
	{
		AUTGameState* UTGameState = UTChar->GetWorld()->GetGameState<AUTGameState>();
		if (UTGameState)
		{
			AUTLineUpZone* ZoneToUse = UTGameState->GetAppropriateSpawnList(LineUpTypes::Intro);
			if (ZoneToUse)
			{
				if (ZoneToUse->DefaultIntroMontages.Num() > 0)
				{
					return FMath::RandHelper(ZoneToUse->DefaultIntroMontages.Num());
				}
			}
		}
	}

	return -1;
}