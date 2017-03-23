// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTGameRuleset.h"

UUTGameRuleset::UUTGameRuleset(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCompetitiveMatch = false;
	OptionFlags = GAME_OPTION_FLAGS_All;
}


void UUTGameRuleset::GetCompleteMapList(TArray<FString>& OutMapList, bool bInsureNew)
{
	if (bInsureNew) OutMapList.Empty();

	if (!EpicMaps.IsEmpty())
	{
		EpicMaps.ParseIntoArray(OutMapList, TEXT(","), true);
	}

	for (int32 i=0; i < CustomMapList.Num(); i++)
	{
		OutMapList.Add(CustomMapList[i]);
	}
}
