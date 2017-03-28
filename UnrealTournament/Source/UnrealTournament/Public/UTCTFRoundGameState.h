// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCTFGameState.h"
#include "UTCTFRoundGameState.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTCTFRoundGameState : public AUTCTFGameState
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(Replicated)
		int32 OffenseKills;

	UPROPERTY(Replicated)
		int32 DefenseKills;

	UPROPERTY(Replicated)
		int32 RemainingPickupDelay;

	UPROPERTY(Replicated)
		int32 TiebreakValue;

	UPROPERTY()
		bool bAttackerLivesLimited;

	UPROPERTY()
		bool bDefenderLivesLimited;

	UPROPERTY(Replicated)
		int32 RedLivesRemaining;

	UPROPERTY(Replicated)
		int32 BlueLivesRemaining;

	UFUNCTION(BlueprintCallable, Category = Team)
		virtual bool IsTeamOnOffense(int32 TeamNumber) const;

	UFUNCTION(BlueprintCallable, Category = Team)
		virtual bool IsTeamOnDefense(int32 TeamNumber) const;

	virtual FText GetGameStatusText(bool bForScoreboard) override;
	virtual FText GetRoundStatusText(bool bForScoreboard);
	virtual void DefaultTimer() override;

	virtual bool InOrder(class AUTPlayerState* P1, class AUTPlayerState* P2) override;

protected:
	virtual void UpdateTimeMessage();

};
