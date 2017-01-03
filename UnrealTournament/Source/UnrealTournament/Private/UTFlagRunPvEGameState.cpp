// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTFlagRunPvEGameState.h"
#include "UnrealNetwork.h"

void AUTFlagRunPvEGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTFlagRunPvEGameState, KillsUntilExtraLife);
	DOREPLIFETIME(AUTFlagRunPvEGameState, NextStarTime);
}

FText AUTFlagRunPvEGameState::GetRoundStatusText(bool bForScoreboard)
{
	if (bForScoreboard)
	{
		// FIXME
		return Super::GetRoundStatusText(bForScoreboard);
	}
	else
	{
		const TCHAR StarChar = TEXT('\u2605');
		FString StarLine;
		for (int i = 0; i < BonusLevel; i++)
		{
			StarLine += StarChar;
		}
		FText NextText = NSLOCTEXT("UnrealTournament", "ToNextStar", "Next: {BonusTime}");
		FFormatNamedArguments Args;
		Args.Add("BonusTime", FText::AsNumber(FMath::Max<int32>(NextStarTime - ElapsedTime, 0)));
		FString SecondLine = FText::Format(NextText, Args).ToString();
		return FText::FromString(FString::Printf(TEXT("%s\n%s"), *StarLine, *SecondLine));
	}
}