// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTCarriedObjectMessage.h"
#include "UTCTFGameMessage.h"
#include "UTGauntletGameMessage.h"
#include "UTGauntletGameState.h"
#include "UTAnnouncer.h"

UUTGauntletGameMessage::UUTGauntletGameMessage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MessageArea = FName(TEXT("Announcements"));
	MessageSlot = FName(TEXT("CountdownMessages"));
	InitialFlagSpawnMessage = NSLOCTEXT("SCTFGameMessage","FlagSpawnTimer","Flag spawns in 30 seconds!");
	FlagSpawnMessage = NSLOCTEXT("SCTFGameMessage", "FlagSpawn", "Flag spawns in...");
	YouAreOnOffenseMessage = NSLOCTEXT("SCTFGameMessage", "OnOffense", "You are on OFFENSE!");
	YouAreOnDefenseMessage = NSLOCTEXT("SCTFGameMessage", "OnDefense", "You are on DEFENSE!");
}

FText UUTGauntletGameMessage::GetText(int32 Switch, bool bTargetsPlayerState1, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const
{

	switch (Switch)
	{
		case 0 : return InitialFlagSpawnMessage; break;
		case 1 : return FlagSpawnMessage; break;
		case 2 : return YouAreOnOffenseMessage; break;
		case 3 : return YouAreOnDefenseMessage; break;
		case 4 : return FText::FromString(TEXT(" ")); break;			// 4 is a UMG class
		case 5 : return FText::FromString(TEXT(" ")); break;			// 5 is a UMG class
	}

	return Super::GetText(Switch, bTargetsPlayerState1, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
}

FString UUTGauntletGameMessage::GetAnnouncementUMGClassname(int32 Switch, const UObject* OptionalObject) const
{
	if (Switch == 4) return TEXT("/Game/RestrictedAssets/UI/UMGHudMessages/UTTeamScoreMessageWidget.UTTeamScoreMessageWidget");
	if (Switch == 5) return TEXT("/Game/RestrictedAssets/UI/UMGHudMessages/UTTeamVictoryMessage.UTTeamVictoryMessage");
	return TEXT("");
}

float UUTGauntletGameMessage::GetLifeTime(int32 Switch) const
{
	if (Switch == 4) return 6.0f;
	if (Switch == 5) return 5.0f;
    return Blueprint_GetLifeTime(Switch);
}
