// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTFlagRunScoreboard.h"
#include "UTFlagRunPvEGameState.h"

#include "UTFlagRunPvEScoreboard.generated.h"

UCLASS()
class UUTFlagRunPvEScoreboard : public UUTFlagRunScoreboard
{
	GENERATED_BODY()
public:
	UUTFlagRunPvEScoreboard(const FObjectInitializer& OI)
		: Super(OI)
	{
		DefendTitle = NSLOCTEXT("UTScoreboard", "Defending", "FLAG INVASION - {Difficulty}");
		
		DefendLines.Empty();
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine1", "* You are defending.  Your goal is to keep the enemy from bringing"));
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine1b", "  the flag to your base for as long as possible."));
		DefendLines.Add(FText::GetEmpty());
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine2", "* You have 3 lives to start. Extra lives are gained by team kill count."));
		DefendLines.Add(FText::GetEmpty());
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine3", "* Enemy minions respawn endlessly. Elite monsters appear "));
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine3b", "   periodically, but have limited respawns. "));
		DefendLines.Add(FText::GetEmpty());
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine5", "* The enemy flag carrier can power up Rally Points to cause monsters"));
		DefendLines.Add(NSLOCTEXT("UTPvEScoreboard", "DefenseLine5b", "  to spawn there for a limited time. "));
	}

	virtual FText GetRoundTitle(bool bIsOnDefense) const override
	{
		AUTFlagRunPvEGameState* GS = GetWorld()->GetGameState<AUTFlagRunPvEGameState>();
		FFormatNamedArguments Args;
		Args.Add("Difficulty", GetBotSkillName(GS->GameDifficulty));
		return FText::Format(DefendTitle, Args);
	}

	virtual void GetScoringStars(int32& NumStars, FLinearColor& StarColor) const override
	{
		AUTFlagRunGameState* GS = GetWorld()->GetGameState<AUTFlagRunGameState>();
		if (GS != nullptr)
		{
			NumStars = GS->BonusLevel;
			switch (GS->BonusLevel)
			{
				case 0:
				case 1:
					StarColor = BRONZECOLOR;
					break;
				case 2:
					StarColor = SILVERCOLOR;
					break;
				default:
					StarColor = GOLDCOLOR;
					break;
			}
		}
	}

	virtual void DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset) override
	{
		if (PlayerState->GetTeamNum() == 1)
		{
			Super::DrawPlayer(Index, PlayerState, RenderDelta, XOffset, YOffset);
		}
	}
};