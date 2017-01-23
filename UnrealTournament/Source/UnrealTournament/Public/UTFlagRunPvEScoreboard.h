// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTFlagRunScoreboard.h"
#include "UTFlagRunGameState.h"

#include "UTFlagRunPvEScoreboard.generated.h"

UCLASS()
class UUTFlagRunPvEScoreboard : public UUTFlagRunScoreboard
{
	GENERATED_BODY()
public:

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
					StarColor = GS->BronzeBonusColor;
					break;
				case 2:
					StarColor = GS->SilverBonusColor;
					break;
				default:
					StarColor = GS->GoldBonusColor;
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