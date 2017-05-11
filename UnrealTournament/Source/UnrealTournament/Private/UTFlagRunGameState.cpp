// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTFlagRunGame.h"
#include "UTFlagRunGameState.h"
#include "UTCTFGameMode.h"
#include "UTPowerupSelectorUserWidget.h"
#include "Net/UnrealNetwork.h"
#include "UTCTFScoring.h"
#include "StatNames.h"
#include "UTCountDownMessage.h"
#include "UTAnnouncer.h"
#include "UTCTFMajorMessage.h"
#include "UTRallyPoint.h"

AUTFlagRunGameState::AUTFlagRunGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRedToCap = false;
	GoldBonusText = NSLOCTEXT("FlagRun", "GoldBonusText", "\u2605 \u2605 \u2605");
	SilverBonusText = NSLOCTEXT("FlagRun", "SilverBonusText", "\u2605 \u2605");
	GoldBonusTimedText = NSLOCTEXT("FlagRun", "GoldBonusTimeText", "\u2605 \u2605 \u2605 {BonusTime}");
	SilverBonusTimedText = NSLOCTEXT("FlagRun", "SilverBonusTimeText", "\u2605 \u2605 {BonusTime}");
	BronzeBonusTimedText = NSLOCTEXT("FlagRun", "BronzeBonusTimeText", "\u2605 {BonusTime}");
	BronzeBonusText = NSLOCTEXT("FlagRun", "BronzeBonusText", "\u2605");
	BonusLevel = 3;
	bAttackerLivesLimited = false;
	bDefenderLivesLimited = true;
	FlagRunMessageSwitch = 0;
	FlagRunMessageTeam = nullptr;
	bPlayStatusAnnouncements = true;
	bEnemyRallyPointIdentified = false;
	EarlyEndTime = 0;
	bTeamGame = true;
	GoldBonusThreshold = 120;
	SilverBonusThreshold = 60;

	HighlightMap.Add(HighlightNames::MostKillsTeam, NSLOCTEXT("AUTGameMode", "MostKillsTeam", "Most Kills for Team ({0})"));
	HighlightMap.Add(HighlightNames::BadMF, NSLOCTEXT("AUTGameMode", "MostKillsTeam", "Most Kills for Team ({0})"));
	HighlightMap.Add(HighlightNames::BadAss, NSLOCTEXT("AUTGameMode", "MostKillsTeam", "Most Kills for Team ({0})"));
	HighlightMap.Add(HighlightNames::LikeABoss, NSLOCTEXT("AUTGameMode", "MostKillsTeam", "Most Kills for Team ({0})"));
	HighlightMap.Add(HighlightNames::DeathIncarnate, NSLOCTEXT("AUTGameMode", "MostKills", "Most Kills ({0})"));
	HighlightMap.Add(HighlightNames::ComeAtMeBro, NSLOCTEXT("AUTGameMode", "MostKills", "Most Kills ({0})"));
	HighlightMap.Add(HighlightNames::ThisIsSparta, NSLOCTEXT("AUTGameMode", "MostKills", "Most Kills ({0})"));

	HighlightMap.Add(HighlightNames::NaturalBornKiller, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::SpecialForces, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::HiredGun, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::HappyToBeHere, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::BobLife, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::GameOver, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::CoolBeans, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::NotSureIfSerious, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::AllOutOfBubbleGum, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::MoreThanAHandful, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::ToughGuy, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::LargerThanLife, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::AssKicker, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::Destroyer, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));
	HighlightMap.Add(HighlightNames::LockedAndLoaded, NSLOCTEXT("AUTGameMode", "LotsOfKills", "{0} Kills"));

	HighlightMap.Add(HighlightNames::RedeemerRejection, NSLOCTEXT("AUTGameMode", "RedeemerRejection", "Rejected Redeemer"));
	HighlightMap.Add(HighlightNames::FlagDenials, NSLOCTEXT("AUTGameMode", "FlagDenials", "{0} Denials"));
	HighlightMap.Add(HighlightNames::WeaponKills, NSLOCTEXT("AUTGameMode", "WeaponKills", "{0} kills with {1}"));
	HighlightMap.Add(HighlightNames::KillingBlowsAward, NSLOCTEXT("AUTGameMode", "KillingBlowsAward", "{0} killing blows"));
	HighlightMap.Add(HighlightNames::MostKillingBlowsAward, NSLOCTEXT("AUTGameMode", "MostKillingBlowsAward", "Most killing blows ({0})"));
	HighlightMap.Add(HighlightNames::CoupDeGrace, NSLOCTEXT("AUTGameMode", "MostKillingBlowsAward", "Most killing blows ({0})"));
	HighlightMap.Add(HighlightNames::HardToKill, NSLOCTEXT("AUTGameMode", "HardToKill", "Only died {0} times"));
	HighlightMap.Add(HighlightNames::Rallies, NSLOCTEXT("AUTGameMode", "Rallies", "{0} Rallies"));
	HighlightMap.Add(HighlightNames::RallyPointPowered, NSLOCTEXT("AUTGameMode", "RallyPointPowered", "{0} RallyPoints Powered"));
	HighlightMap.Add(HighlightNames::HatTrick, NSLOCTEXT("AUTGameMode", "HatTrick", "3 Flag Caps"));
	HighlightMap.Add(HighlightNames::LikeTheWind, NSLOCTEXT("AUTGameMode", "LikeTheWind", "\u2605 \u2605 \u2605 Cap"));
	HighlightMap.Add(HighlightNames::DeliveryBoy, NSLOCTEXT("AUTGameMode", "DeliveryBoy", "Capped the Flag"));

	ShortHighlightMap.Add(HighlightNames::BadMF, NSLOCTEXT("AUTGameMode", "BadMF", "Bad to the Bone"));
	ShortHighlightMap.Add(HighlightNames::BadAss, NSLOCTEXT("AUTGameMode", "BadAss", "Superior Genetics"));
	ShortHighlightMap.Add(HighlightNames::LikeABoss, NSLOCTEXT("AUTGameMode", "LikeABoss", "Like a Boss"));
	ShortHighlightMap.Add(HighlightNames::DeathIncarnate, NSLOCTEXT("AUTGameMode", "DeathIncarnate", "Death Incarnate"));
	ShortHighlightMap.Add(HighlightNames::NaturalBornKiller, NSLOCTEXT("AUTGameMode", "NaturalBornKiller", "Natural Born Killer"));
	ShortHighlightMap.Add(HighlightNames::SpecialForces, NSLOCTEXT("AUTGameMode", "SpecialForces", "Honey Badger"));
	ShortHighlightMap.Add(HighlightNames::HiredGun, NSLOCTEXT("AUTGameMode", "HiredGun", "Hired Gun"));
	ShortHighlightMap.Add(HighlightNames::HappyToBeHere, NSLOCTEXT("AUTGameMode", "HappyToBeHere", "Just Happy To Be Here"));
	ShortHighlightMap.Add(HighlightNames::MostKillsTeam, NSLOCTEXT("AUTGameMode", "ShortMostKills", "Top Gun"));
	ShortHighlightMap.Add(HighlightNames::BobLife, NSLOCTEXT("AUTGameMode", "BobLife", "Living the Bob Life"));
	ShortHighlightMap.Add(HighlightNames::GameOver, NSLOCTEXT("AUTGameMode", "GameOver", "Game Over, Man"));
	ShortHighlightMap.Add(HighlightNames::CoolBeans, NSLOCTEXT("AUTGameMode", "CoolBeans", "Cool Beans Yo"));
	ShortHighlightMap.Add(HighlightNames::NotSureIfSerious, NSLOCTEXT("AUTGameMode", "NotSureIfSerious", "Not Sure If Serious"));
	ShortHighlightMap.Add(HighlightNames::ComeAtMeBro, NSLOCTEXT("AUTGameMode", "ComeAtMeBro", "Come at Me Bro"));
	ShortHighlightMap.Add(HighlightNames::ThisIsSparta, NSLOCTEXT("AUTGameMode", "ThisIsSparta", "This is Sparta!"));
	ShortHighlightMap.Add(HighlightNames::AllOutOfBubbleGum, NSLOCTEXT("AUTGameMode", "AllOutOfBubbleGum", "All Out of Bubblegum"));
	ShortHighlightMap.Add(HighlightNames::MoreThanAHandful, NSLOCTEXT("AUTGameMode", "MoreThanAHandful", "More Than A Handfull"));
	ShortHighlightMap.Add(HighlightNames::ToughGuy, NSLOCTEXT("AUTGameMode", "ToughGuy", "Tough Guy"));
	ShortHighlightMap.Add(HighlightNames::LargerThanLife, NSLOCTEXT("AUTGameMode", "LargerThanLife", "Larger Than Life"));
	ShortHighlightMap.Add(HighlightNames::AssKicker, NSLOCTEXT("AUTGameMode", "AssKicker", "Ass Kicker"));
	ShortHighlightMap.Add(HighlightNames::Destroyer, NSLOCTEXT("AUTGameMode", "Destroyer", "Destroyer"));
	ShortHighlightMap.Add(HighlightNames::LockedAndLoaded, NSLOCTEXT("AUTGameMode", "LockedAndLoaded", "Locked And Loaded"));

	ShortHighlightMap.Add(HighlightNames::RedeemerRejection, NSLOCTEXT("AUTGameMode", "ShortRejection", "Redeem this"));
	ShortHighlightMap.Add(HighlightNames::FlagDenials, NSLOCTEXT("AUTGameMode", "ShortDenials", "You shall not pass"));
	ShortHighlightMap.Add(HighlightNames::WeaponKills, NSLOCTEXT("AUTGameMode", "ShortWeaponKills", "Weapon Master"));
	ShortHighlightMap.Add(HighlightNames::KillingBlowsAward, NSLOCTEXT("AUTGameMode", "ShortKillingBlowsAward", "Nice Shot"));
	ShortHighlightMap.Add(HighlightNames::MostKillingBlowsAward, NSLOCTEXT("AUTGameMode", "ShortMostKillingBlowsAward", "Punisher"));
	ShortHighlightMap.Add(HighlightNames::CoupDeGrace, NSLOCTEXT("AUTGameMode", "CoupDeGrace", "Coup de Grace"));
	ShortHighlightMap.Add(HighlightNames::HardToKill, NSLOCTEXT("AUTGameMode", "ShortHardToKill", "Hard to Kill"));
	ShortHighlightMap.Add(HighlightNames::Rallies, NSLOCTEXT("AUTGameMode", "ShortRallies", "Beam me Up"));
	ShortHighlightMap.Add(HighlightNames::RallyPointPowered, NSLOCTEXT("AUTGameMode", "ShortRallyPointPowered", "Power Source"));
	ShortHighlightMap.Add(HighlightNames::HatTrick, NSLOCTEXT("AUTGameMode", "ShortHatTrick", "Hat Trick"));
	ShortHighlightMap.Add(HighlightNames::LikeTheWind, NSLOCTEXT("AUTGameMode", "ShortLikeTheWind", "Run Like the Wind"));
	ShortHighlightMap.Add(HighlightNames::DeliveryBoy, NSLOCTEXT("AUTGameMode", "ShortDeliveryBoy", "Delivery Boy"));
}

void AUTFlagRunGameState::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() && GetWorld()->GetAuthGameMode<AUTFlagRunGame>())
	{
		bAllowBoosts = GetWorld()->GetAuthGameMode<AUTFlagRunGame>()->bAllowBoosts;
	}

	UpdateSelectablePowerups();
	AddModeSpecificOverlays();
}

void AUTFlagRunGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTFlagRunGameState, bRedToCap);
	DOREPLIFETIME(AUTFlagRunGameState, BonusLevel);
	DOREPLIFETIME(AUTFlagRunGameState, CurrentRallyPoint);
	DOREPLIFETIME(AUTFlagRunGameState, bEnemyRallyPointIdentified);
	DOREPLIFETIME(AUTFlagRunGameState, FlagRunMessageSwitch);
	DOREPLIFETIME(AUTFlagRunGameState, FlagRunMessageTeam);
	DOREPLIFETIME(AUTFlagRunGameState, bAttackersCanRally);
	DOREPLIFETIME(AUTFlagRunGameState, EarlyEndTime);
	DOREPLIFETIME(AUTFlagRunGameState, bAllowBoosts);
	DOREPLIFETIME(AUTFlagRunGameState, OffenseSelectablePowerups);
	DOREPLIFETIME(AUTFlagRunGameState, DefenseSelectablePowerups);
}

void AUTFlagRunGameState::OnBonusLevelChanged()
{
	if (BonusLevel < 3)
	{
		USoundBase* SoundToPlay = UUTCountDownMessage::StaticClass()->GetDefaultObject<UUTCountDownMessage>()->TimeEndingSound;
		if (SoundToPlay != NULL)
		{
			for (FLocalPlayerIterator It(GEngine, GetWorld()); It; ++It)
			{
				AUTPlayerController* PC = Cast<AUTPlayerController>(It->PlayerController);
				if (PC && PC->IsLocalPlayerController())
				{
					PC->UTClientPlaySound(SoundToPlay);
				}
			}
		}
	}
}

void AUTFlagRunGameState::UpdateTimeMessage()
{
	if (!bIsAtIntermission && (GetNetMode() != NM_DedicatedServer) && IsMatchInProgress())
	{
		// bonus time countdowns
		if (RemainingTime <= GoldBonusThreshold + 7)
		{
			if (RemainingTime > GoldBonusThreshold)
			{
				for (FLocalPlayerIterator It(GEngine, GetWorld()); It; ++It)
				{
					AUTPlayerController* PC = Cast<AUTPlayerController>(It->PlayerController);
					if (PC != NULL)
					{
						PC->ClientReceiveLocalizedMessage(UUTCountDownMessage::StaticClass(), 4000 + RemainingTime - GoldBonusThreshold);
					}
				}
			}
			else if ((RemainingTime <= SilverBonusThreshold + 7) && (RemainingTime > SilverBonusThreshold))
			{
				for (FLocalPlayerIterator It(GEngine, GetWorld()); It; ++It)
				{
					AUTPlayerController* PC = Cast<AUTPlayerController>(It->PlayerController);
					if (PC != NULL)
					{
						PC->ClientReceiveLocalizedMessage(UUTCountDownMessage::StaticClass(), 3000 + RemainingTime - SilverBonusThreshold);
					}
				}
			}
		}
	}
}

FLinearColor AUTFlagRunGameState::GetGameStatusColor()
{
	if (CTFRound > 0)
	{
		switch(BonusLevel)
		{
			case 1: return BRONZECOLOR; break;
			case 2: return SILVERCOLOR; break;
			case 3: return GOLDCOLOR; break;
		}
	}
	return FLinearColor::White;
}

FText AUTFlagRunGameState::GetRoundStatusText(bool bForScoreboard)
{
	if (bForScoreboard)
	{
		FFormatNamedArguments Args;
		Args.Add("RoundNum", FText::AsNumber(CTFRound));
		Args.Add("NumRounds", FText::AsNumber(NumRounds));
		return (NumRounds > 0) ? FText::Format(FullRoundInProgressStatus, Args) : FText::Format(RoundInProgressStatus, Args);
	}
	else
	{
		FText StatusText = BronzeBonusTimedText;
		int32 RemainingBonus = FMath::Clamp(RemainingTime, 0, 59);
		if (BonusLevel == 3)
		{
			RemainingBonus = FMath::Clamp(RemainingTime - GoldBonusThreshold, 0, 60);
			StatusText = GoldBonusTimedText;
			FFormatNamedArguments Args;
			Args.Add("BonusTime", FText::AsNumber(RemainingBonus));
			return FText::Format(GoldBonusTimedText, Args);
		}
		else if (BonusLevel == 2)
		{
			RemainingBonus = FMath::Clamp(RemainingTime - SilverBonusThreshold, 0, 59);
			StatusText = SilverBonusTimedText;
		}
		FFormatNamedArguments Args;
		Args.Add("BonusTime", FText::AsNumber(RemainingBonus));
		return FText::Format(StatusText, Args);
	}
}

void AUTFlagRunGameState::UpdateSelectablePowerups()
{
	if (!bAllowBoosts)
	{
		OffenseSelectablePowerups.Empty();
		DefenseSelectablePowerups.Empty();
		return;
	}
	const int32 RedTeamIndex = 0;
	const int32 BlueTeamIndex = 1;
	const bool bIsRedTeamOffense = IsTeamOnDefenseNextRound(RedTeamIndex);

	TSubclassOf<UUTPowerupSelectorUserWidget> OffensePowerupSelectorWidget;
	TSubclassOf<UUTPowerupSelectorUserWidget> DefensePowerupSelectorWidget;

	if (bIsRedTeamOffense)
	{
		OffensePowerupSelectorWidget = LoadClass<UUTPowerupSelectorUserWidget>(NULL, *GetPowerupSelectWidgetPath(RedTeamIndex), NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
		DefensePowerupSelectorWidget = LoadClass<UUTPowerupSelectorUserWidget>(NULL, *GetPowerupSelectWidgetPath(BlueTeamIndex), NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	}
	else
	{
		OffensePowerupSelectorWidget = LoadClass<UUTPowerupSelectorUserWidget>(NULL, *GetPowerupSelectWidgetPath(BlueTeamIndex), NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
		DefensePowerupSelectorWidget = LoadClass<UUTPowerupSelectorUserWidget>(NULL, *GetPowerupSelectWidgetPath(RedTeamIndex), NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
	}

	if (OffensePowerupSelectorWidget)
	{
		for (TSubclassOf<class AUTInventory> BoostItem : OffensePowerupSelectorWidget.GetDefaultObject()->SelectablePowerups)
		{
			OffenseSelectablePowerups.Add(BoostItem);
		}
	}

	if (DefensePowerupSelectorWidget)
	{
		for (TSubclassOf<class AUTInventory> BoostItem : DefensePowerupSelectorWidget.GetDefaultObject()->SelectablePowerups)
		{
			DefenseSelectablePowerups.Add(BoostItem);
		}
	}
}

void AUTFlagRunGameState::SetSelectablePowerups(const TArray<TSubclassOf<AUTInventory>>& OffenseList, const TArray<TSubclassOf<AUTInventory>>& DefenseList)
{
	OffenseSelectablePowerups = OffenseList;
	DefenseSelectablePowerups = DefenseList;
	for (int32 i = 0; i < PlayerArray.Num(); i++)
	{
		AUTPlayerState* PS = Cast<AUTPlayerState>(PlayerArray[i]);
		if (PS != nullptr && PS->BoostClass != nullptr && !OffenseList.Contains(PS->BoostClass) && !DefenseList.Contains(PS->BoostClass))
		{
			PS->ServerSetBoostItem(0);
		}
	}
}

FString AUTFlagRunGameState::GetPowerupSelectWidgetPath(int32 TeamNumber)
{
	if (!bAllowBoosts)
	{
		if (IsTeamOnDefenseNextRound(TeamNumber))
		{
			return TEXT("/Game/RestrictedAssets/Blueprints/BP_PowerupSelector_EmptyDefense.BP_PowerupSelector_EmptyDefense_C");
		}
		else
		{
			return TEXT("/Game/RestrictedAssets/Blueprints/BP_PowerupSelector_EmptyOffense.BP_PowerupSelector_EmptyOffense_C");
		}
	}
	else
	{
		if (IsTeamOnDefenseNextRound(TeamNumber))
		{
			return TEXT("/Game/RestrictedAssets/Blueprints/BP_PowerupSelector_Defense.BP_PowerupSelector_Defense_C");
		}
		else
		{
			return TEXT("/Game/RestrictedAssets/Blueprints/BP_PowerupSelector_Offense.BP_PowerupSelector_Offense_C");
		}
	}
}

void AUTFlagRunGameState::AddModeSpecificOverlays()
{
	for (TSubclassOf<class AUTInventory> BoostClass : OffenseSelectablePowerups)
	{
		BoostClass.GetDefaultObject()->AddOverlayMaterials(this);
	}

	for (TSubclassOf<class AUTInventory> BoostClass : DefenseSelectablePowerups)
	{
		BoostClass.GetDefaultObject()->AddOverlayMaterials(this);
	}
}

TSubclassOf<class AUTInventory> AUTFlagRunGameState::GetSelectableBoostByIndex(AUTPlayerState* PlayerState, int Index) const
{
	if (PlayerState != nullptr && (IsMatchInProgress() ? IsTeamOnDefense(PlayerState->GetTeamNum()) : IsTeamOnDefenseNextRound(PlayerState->GetTeamNum())))
	{
		if ((DefenseSelectablePowerups.Num() > 0) && (Index < DefenseSelectablePowerups.Num()))
		{
			return DefenseSelectablePowerups[Index];
		}
	}
	else
	{
		if ((OffenseSelectablePowerups.Num() > 0) && (Index < OffenseSelectablePowerups.Num()))
		{
			return OffenseSelectablePowerups[Index];
		}
	}

	return nullptr;
}

bool AUTFlagRunGameState::IsSelectedBoostValid(AUTPlayerState* PlayerState) const
{
	if (PlayerState == nullptr || PlayerState->BoostClass == nullptr)
	{
		return false;
	}

	return IsTeamOnDefenseNextRound(PlayerState->GetTeamNum()) ? DefenseSelectablePowerups.Contains(PlayerState->BoostClass) : OffenseSelectablePowerups.Contains(PlayerState->BoostClass);
}

void AUTFlagRunGameState::PrecacheAllPowerupAnnouncements(class UUTAnnouncer* Announcer) const
{
	for (TSubclassOf<class AUTInventory> PowerupClass : DefenseSelectablePowerups)
	{
		CachePowerupAnnouncement(Announcer, PowerupClass);
	}

	for (TSubclassOf<class AUTInventory> PowerupClass : OffenseSelectablePowerups)
	{
		CachePowerupAnnouncement(Announcer, PowerupClass);
	}
}

void AUTFlagRunGameState::CachePowerupAnnouncement(class UUTAnnouncer* Announcer, const TSubclassOf<AUTInventory> PowerupClass) const
{
	AUTInventory* Powerup = PowerupClass->GetDefaultObject<AUTInventory>();
	if (Powerup)
	{
		Announcer->PrecacheAnnouncement(Powerup->AnnouncementName);
	}
}

AUTCTFFlag* AUTFlagRunGameState::GetOffenseFlag()
{
	int OffenseTeam = bRedToCap ? 0 : 1;
	return ((FlagBases.Num() > OffenseTeam) && FlagBases[OffenseTeam]) ? FlagBases[OffenseTeam]->MyFlag : nullptr;
}

void AUTFlagRunGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Role == ROLE_Authority)
	{
		uint8 OffensiveTeam = bRedToCap ? 0 : 1;
		if (FlagBases.IsValidIndex(OffensiveTeam) && FlagBases[OffensiveTeam] != nullptr)
		{
			AUTCTFFlag* Flag = Cast<AUTCTFFlag>(FlagBases[OffensiveTeam]->GetCarriedObject());
			bAttackersCanRally = (CurrentRallyPoint != nullptr) && (CurrentRallyPoint->RallyPointState == RallyPointStates::Powered);
			AUTGameVolume* GV = Flag && Flag->HoldingPawn && Flag->HoldingPawn->UTCharacterMovement ? Cast<AUTGameVolume>(Flag->HoldingPawn->UTCharacterMovement->GetPhysicsVolume()) : nullptr;
			bool bInFlagRoom = GV && (GV->bIsDefenderBase || GV->bIsTeamSafeVolume);
			bHaveEstablishedFlagRunner = (!bInFlagRoom && Flag && Flag->Holder && Flag->HoldingPawn && (GetWorld()->GetTimeSeconds() - Flag->PickedUpTime > 2.f));
		}
	}
}

bool AUTFlagRunGameState::IsTeamOnOffense(int32 TeamNumber) const
{
	const bool bIsOnRedTeam = (TeamNumber == 0);
	return (bRedToCap == bIsOnRedTeam);
}

bool AUTFlagRunGameState::IsTeamOnDefense(int32 TeamNumber) const
{
	return !IsTeamOnOffense(TeamNumber);
}

bool AUTFlagRunGameState::IsTeamOnDefenseNextRound(int32 TeamNumber) const
{
	//We alternate teams, so if we are on offense now, next round we will be on defense
	return IsTeamOnOffense(TeamNumber);
}

void AUTFlagRunGameState::CheckTimerMessage()
{
	RemainingTime -= EarlyEndTime;
	Super::CheckTimerMessage();
	RemainingTime += EarlyEndTime;
}

int32 AUTFlagRunGameState::NumHighlightsNeeded()
{
	return HasMatchEnded() ? 4 : 1;
}

// new plan - rank order kills, give pending award.. Early out if good enough, override for lower
void AUTFlagRunGameState::UpdateRoundHighlights()
{
	ClearHighlights();

	bHaveRallyHighlight = false;
	bHaveRallyPoweredHighlight = false;
	HappyCount = 0;
	HiredGunCount = 0;

	//Collect all the weapons
	TArray<AUTWeapon *> StatsWeapons;
	if (StatsWeapons.Num() == 0)
	{
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AUTPickupWeapon* Pickup = Cast<AUTPickupWeapon>(*It);
			if (Pickup && Pickup->GetInventoryType())
			{
				StatsWeapons.AddUnique(Pickup->GetInventoryType()->GetDefaultObject<AUTWeapon>());
			}
		}
	}

	AUTPlayerState* MostKills = NULL;
	AUTPlayerState* MostKillsRed = NULL;
	AUTPlayerState* MostKillsBlue = NULL;
	AUTPlayerState* MostHeadShotsPS = NULL;
	AUTPlayerState* MostAirRoxPS = NULL;
	AUTPlayerState* MostKillingBlowsRed = NULL;
	AUTPlayerState* MostKillingBlowsBlue = NULL;

	for (TActorIterator<AUTPlayerState> It(GetWorld()); It; ++It)
	{
		AUTPlayerState* PS = *It;
		if (PS && !PS->bOnlySpectator)
		{
			int32 TeamIndex = PS->Team ? PS->Team->TeamIndex : 0;
			int32 TotalKills = PS->RoundKills + PS->RoundKillAssists;
			if (TotalKills > (MostKills ? MostKills->RoundKills + MostKills->RoundKillAssists : 0))
			{
				MostKills = PS;
			}
			AUTPlayerState* TopTeamKiller = (TeamIndex == 0) ? MostKillsRed : MostKillsBlue;
			if (TotalKills > (TopTeamKiller ? TopTeamKiller->RoundKills + TopTeamKiller->RoundKillAssists : 0))
			{
				if (TeamIndex == 0)
				{
					MostKillsRed = PS;
				}
				else
				{
					MostKillsBlue = PS;
				}
			}
			AUTPlayerState* TopKillingBlows = (TeamIndex == 0) ? MostKillingBlowsRed : MostKillingBlowsBlue;
			if (PS->RoundKills > (TopKillingBlows ? TopKillingBlows->RoundKills : 0))
			{
				if (TeamIndex == 0)
				{
					MostKillingBlowsRed = PS;
				}
				else
				{
					MostKillingBlowsBlue = PS;
				}
			}

			//Figure out what weapon killed the most
			PS->FavoriteWeapon = nullptr;
			int32 BestKills = 0;
			for (AUTWeapon* Weapon : StatsWeapons)
			{
				int32 Kills = Weapon->GetWeaponKillStatsForRound(PS);
				if (Kills > BestKills)
				{
					BestKills = Kills;
					PS->FavoriteWeapon = Weapon->GetClass();
				}
			}

			if (PS->GetRoundStatsValue(NAME_SniperHeadshotKills) > (MostHeadShotsPS ? MostHeadShotsPS->GetRoundStatsValue(NAME_SniperHeadshotKills) : 1.f))
			{
				MostHeadShotsPS = PS;
			}
			if (PS->GetRoundStatsValue(NAME_AirRox) > (MostAirRoxPS ? MostAirRoxPS->GetRoundStatsValue(NAME_AirRox) : 1.f))
			{
				MostAirRoxPS = PS;
			}
		}
	}

	for (TActorIterator<AUTPlayerState> It(GetWorld()); It; ++It)
	{
		AUTPlayerState* PS = *It;
		if (PS && !PS->bOnlySpectator)
		{
			int32 TotalKills = PS->RoundKills + PS->RoundKillAssists;
			if (MostKills && (TotalKills >= 20) && (TotalKills >= MostKills->RoundKills + MostKills->RoundKillAssists))
			{
				PS->AddMatchHighlight(HighlightNames::DeathIncarnate, TotalKills);
			}
			else if (PS->Team)
			{
				if ((PS->FlagCaptures == 3) && ((PS->Team->TeamIndex == 0) == bRedToCap))
				{
					PS->AddMatchHighlight(HighlightNames::HatTrick, 3);
				}
				else
				{
					AUTPlayerState* TopTeamKiller = (PS->Team->TeamIndex == 0) ? MostKillsRed : MostKillsBlue;
					if (TopTeamKiller && (TotalKills >= TopTeamKiller->RoundKills + TopTeamKiller->RoundKillAssists))
					{
						if (TotalKills >= 15)
						{
							PS->AddMatchHighlight((PS == TopTeamKiller) ? HighlightNames::BadMF : HighlightNames::BadAss, TotalKills);
						}
						else if (TotalKills >= 10)
						{
							PS->AddMatchHighlight((PS == TopTeamKiller) ? HighlightNames::LikeABoss : HighlightNames::ThisIsSparta, TotalKills);
						}
						else
						{
							PS->AddMatchHighlight((PS == TopTeamKiller) ? HighlightNames::MostKillsTeam : HighlightNames::ComeAtMeBro, TotalKills);
						}
					}
					else
					{
						AUTPlayerState* TopKillingBlows = (PS->Team->TeamIndex == 0) ? MostKillingBlowsRed : MostKillingBlowsBlue;
						if (TopKillingBlows && (PS->RoundKills >= FMath::Max(2, TopKillingBlows->RoundKills)))
						{
							PS->AddMatchHighlight((PS == TopKillingBlows) ? HighlightNames::MostKillingBlowsAward : HighlightNames::CoupDeGrace, PS->RoundKills);
						}
					}
				}
			}
			if (PS->MatchHighlights[0] == NAME_None)
			{
				if (PS->GetRoundStatsValue(NAME_FlagDenials) > 1)
				{
					PS->AddMatchHighlight(HighlightNames::FlagDenials, PS->GetRoundStatsValue(NAME_FlagDenials));
				}
				else if (PS->GetRoundStatsValue(NAME_RedeemerRejected) > 0)
				{
					PS->AddMatchHighlight(HighlightNames::RedeemerRejection, PS->GetRoundStatsValue(NAME_RedeemerRejected));
				}
			}
		}
	}

	for (TActorIterator<AUTPlayerState> It(GetWorld()); It; ++It)
	{
		AUTPlayerState* PS = *It;
		if (PS && !PS->bOnlySpectator && PS->MatchHighlights[0] == NAME_None)
		{
			if (MostHeadShotsPS && (PS->GetRoundStatsValue(NAME_SniperHeadshotKills) == MostHeadShotsPS->GetRoundStatsValue(NAME_SniperHeadshotKills)))
			{
				PS->AddMatchHighlight(HighlightNames::MostHeadShots, MostHeadShotsPS->GetRoundStatsValue(NAME_SniperHeadshotKills));
			}
			else if (MostAirRoxPS && (PS->GetRoundStatsValue(NAME_AirRox) == MostAirRoxPS->GetRoundStatsValue(NAME_AirRox)))
			{
				PS->AddMatchHighlight(HighlightNames::MostAirRockets, MostAirRoxPS->GetRoundStatsValue(NAME_AirRox));
			}
		}
	}

	for (TActorIterator<AUTPlayerState> It(GetWorld()); It; ++It)
	{
		AUTPlayerState* PS = *It;
		if (PS && !PS->bOnlySpectator && PS->MatchHighlights[0] == NAME_None)
		{
			// only add low priority highlights if not enough high priority highlights
			AddMinorRoundHighlights(PS);
		}
	}
}

void AUTFlagRunGameState::AddMinorRoundHighlights(AUTPlayerState* PS)
{
	if (PS->MatchHighlights[0] != NAME_None)
	{
		return;
	}

	// sprees and multikills
	FName SpreeStatsNames[5] = { NAME_SpreeKillLevel4, NAME_SpreeKillLevel3, NAME_SpreeKillLevel2, NAME_SpreeKillLevel1, NAME_SpreeKillLevel0 };
	for (int32 i = 0; i < 5; i++)
	{
		if (PS->GetRoundStatsValue(SpreeStatsNames[i]) > 0)
		{
			PS->AddMatchHighlight(SpreeStatsNames[i], PS->GetRoundStatsValue(SpreeStatsNames[i]));
			return;
		}
	}

	if (PS->RoundKills + PS->RoundKillAssists >= 15)
	{
		FName KillerNames[2] = { HighlightNames::NaturalBornKiller, HighlightNames::LargerThanLife };
		PS->MatchHighlights[0] = KillerNames[NaturalKillerCount % 2];
		NaturalKillerCount++;
		PS->MatchHighlightData[0] = PS->RoundKills + PS->RoundKillAssists;
		return;
	}

	FName MultiKillsNames[4] = { NAME_MultiKillLevel3, NAME_MultiKillLevel2, NAME_MultiKillLevel1, NAME_MultiKillLevel0 };
	for (int32 i = 0; i < 2; i++)
	{
		if (PS->GetRoundStatsValue(MultiKillsNames[i]) > 0)
		{
			PS->AddMatchHighlight(MultiKillsNames[i], PS->GetRoundStatsValue(MultiKillsNames[i]));
			return;
		}
	}
	if (PS->RoundKills + PS->RoundKillAssists >= 13)
	{
		PS->MatchHighlights[0] = HighlightNames::AssKicker;
		PS->MatchHighlightData[0] = PS->RoundKills + PS->RoundKillAssists;
		return;
	}
	if (PS->RoundKills + PS->RoundKillAssists >= 10)
	{
		FName DestroyerNames[2] = { HighlightNames::SpecialForces, HighlightNames::Destroyer };
		PS->MatchHighlights[0] = DestroyerNames[DestroyerCount % 2];
		DestroyerCount++;
		PS->MatchHighlightData[0] = PS->RoundKills + PS->RoundKillAssists;
		return;
	}

	// announced kills
	FName AnnouncedKills[5] = { NAME_AmazingCombos, NAME_AirRox, NAME_AirSnot, NAME_SniperHeadshotKills, NAME_FlakShreds };
	for (int32 i = 0; i < 5; i++)
	{
		if (PS->GetRoundStatsValue(AnnouncedKills[i]) > 1)
		{
			PS->AddMatchHighlight(AnnouncedKills[i], PS->GetRoundStatsValue(AnnouncedKills[i]));
			return;
		}
	}

	// Most kills with favorite weapon, if needed
	bool bHasMultipleKillWeapon = false;
	if (PS->FavoriteWeapon)
	{
		AUTWeapon* DefaultWeapon = PS->FavoriteWeapon->GetDefaultObject<AUTWeapon>();
		int32 WeaponKills = DefaultWeapon->GetWeaponKillStatsForRound(PS);
		if (WeaponKills > 1)
		{
			bHasMultipleKillWeapon = true;
			bool bIsBestOverall = true;
			for (int32 i = 0; i < PlayerArray.Num(); i++)
			{
				AUTPlayerState* OtherPS = Cast<AUTPlayerState>(PlayerArray[i]);
				if (OtherPS && (PS != OtherPS) && (DefaultWeapon->GetWeaponKillStatsForRound(OtherPS) > WeaponKills))
				{
					bIsBestOverall = false;
					break;
				}
			}
			if (bIsBestOverall)
			{
				PS->AddMatchHighlight(HighlightNames::MostWeaponKills, WeaponKills);
				return;
			}
		}
	}

	int32 NumRallies = PS->GetRoundStatsValue(NAME_Rallies);
	int32 NumRalliesPowered = PS->GetRoundStatsValue(NAME_RalliesPowered);

	if (bHasMultipleKillWeapon)
	{
		AUTWeapon* DefaultWeapon = PS->FavoriteWeapon->GetDefaultObject<AUTWeapon>();
		int32 WeaponKills = DefaultWeapon->GetWeaponKillStatsForRound(PS);
		PS->AddMatchHighlight(HighlightNames::WeaponKills, WeaponKills);
	}
	else if (PS->GetRoundStatsValue(NAME_FCKills) > 1)
	{
		PS->AddMatchHighlight(NAME_FCKills, PS->GetRoundStatsValue(NAME_FCKills));
	}
	else if (PS->RoundKills >= FMath::Max(3, 2 * PS->RoundDeaths))
	{
		PS->AddMatchHighlight(HighlightNames::HardToKill, PS->RoundDeaths);
	}
	else if (!bHaveRallyPoweredHighlight && (NumRalliesPowered > 1))
	{
		PS->AddMatchHighlight(HighlightNames::RallyPointPowered, NumRalliesPowered);
		bHaveRallyPoweredHighlight = true;
	}
	else if (!bHaveRallyHighlight && (NumRallies > 3))
	{
		PS->AddMatchHighlight(HighlightNames::Rallies, NumRallies);
		bHaveRallyHighlight = true;
	}
	else if (PS->Team && (PS->CarriedObject != nullptr) && (PS->Team->RoundBonus > GoldBonusThreshold))
	{
		PS->AddMatchHighlight(HighlightNames::LikeTheWind, 0);
	}
	else if (PS->RoundKills + PS->RoundKillAssists > 0)
	{
		PS->MatchHighlightData[0] = PS->RoundKills + PS->RoundKillAssists;

		if (PS->MatchHighlightData[0] > 7)
		{
			FName BubbleNames[2] = { HighlightNames::AllOutOfBubbleGum, HighlightNames::ToughGuy };
			PS->MatchHighlights[0] = BubbleNames[BubbleGumCount % 2];
			BubbleGumCount++;
		}
		else if (PS->MatchHighlightData[0] == 6)
		{
			PS->MatchHighlights[0] = HighlightNames::MoreThanAHandful;
		}
		else if (PS->CarriedObject != nullptr)
		{
			PS->AddMatchHighlight(HighlightNames::DeliveryBoy, 0);
		}
		else if (PS->MatchHighlightData[0] > 4)
		{
			FName HappyNames[2] = { HighlightNames::BobLife, HighlightNames::GameOver };
			PS->MatchHighlights[0] = HappyNames[BobLifeCount % 2];
			BobLifeCount++;
		}
		else if (PS->RoundKills > 1)
		{
			PS->MatchHighlights[0] = HighlightNames::KillingBlowsAward;
			PS->MatchHighlightData[0] = PS->RoundKills;
		}
		else if (PS->MatchHighlightData[0] > 2)
		{
			FName HappyNames[3] = { HighlightNames::HiredGun, HighlightNames::CoolBeans, HighlightNames::LockedAndLoaded };
			PS->MatchHighlights[0] = HappyNames[HiredGunCount % 3];
			HiredGunCount++;
		}
		else
		{
			FName HappyNames[2] = { HighlightNames::HappyToBeHere, HighlightNames::NotSureIfSerious };
			PS->MatchHighlights[0] = HappyNames[HappyCount % 2];
			HappyCount++;
		}
	}
	else if (PS->CarriedObject != nullptr)
	{
		PS->AddMatchHighlight(HighlightNames::DeliveryBoy, 0);
	}
	else
	{
		PS->MatchHighlights[0] = HighlightNames::ParticipationAward;
	}
}



