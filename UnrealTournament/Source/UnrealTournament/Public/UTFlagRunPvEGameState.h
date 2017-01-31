// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTFlagRunGameState.h"

#include "UTFlagRunPvEGameState.generated.h"

UCLASS()
class AUTFlagRunPvEGameState : public AUTFlagRunGameState
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	int32 GameDifficulty;

	/** time next star will be awarded (relative to RemainingTime, not TimeSeconds) */
	UPROPERTY(Replicated)
	int32 NextStarTime;
	UPROPERTY(Replicated)
	int32 KillsUntilExtraLife;
	UPROPERTY()
	int32 ExtraLivesGained;

	virtual FText GetRoundStatusText(bool bForScoreboard) override;
	virtual void OnBonusLevelChanged() override
	{}
	virtual void UpdateSelectablePowerups() override
	{
		// GameMode sets this via SetSelectablePowerups()
	}
};