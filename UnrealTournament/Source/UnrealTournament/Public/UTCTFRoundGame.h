// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "UTCTFGameState.h"
#include "UTCTFScoring.h"
#include "UTCTFBaseGame.h"
#include "UTCTFRoundGame.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTCTFRoundGame : public AUTCTFBaseGame
{
	GENERATED_UCLASS_BODY()

	/**Alternate round victory condition - get this many kills. */
	UPROPERTY(BlueprintReadWrite, Category = CTF)
	int32 RoundLives;

	/*  If true, use world settings round times */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		bool bUseLevelTiming;

	/** Respawn wait time for team with no life limit. */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		float UnlimitedRespawnWaitTime;

	/** Limited Respawn wait time. */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		float LimitedRespawnWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = CTF)
		int32 NumRounds;

	UPROPERTY(BlueprintReadWrite, Category = CTF)
		float LastAceTime;

	UPROPERTY()
		bool bNeedFiveKillsMessage;

	UPROPERTY()
		bool bFirstRoundInitialized;

	/*  Victory due to secondary score (best total capture time) */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		bool bSecondaryWin;

	UPROPERTY(BlueprintReadOnly)
		int32 FlagPickupDelay;

	UPROPERTY()
		int32 MaxTimeScoreBonus;

	UPROPERTY()
		bool bRollingAttackerSpawns;

	UPROPERTY()
		float RollingAttackerRespawnDelay;

	UPROPERTY()
		float LastAttackerSpawnTime;

	UPROPERTY()
		float RollingSpawnStartTime;

	UPROPERTY()
		bool bLastManOccurred;
	
	UPROPERTY(config)
		bool bShouldAllowPowerupSelection;

	UPROPERTY()
		AUTPlayerState* FlagScorer;

	virtual void InitFlags();
	virtual void InitDelayedFlag(class AUTCarriedObject* Flag);
	virtual void InitFlagForRound(class AUTCarriedObject* Flag);
	virtual void IntermissionSwapSides();
	virtual void FlagCountDown();
	virtual void FlagsAreReady();
	virtual void InitGameStateForRound();

	virtual AActor* SetIntermissionCameras(uint32 TeamToWatch);

	virtual void BeginGame() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleMatchIntermission() override;
	virtual void InitGameState() override;

	virtual void EndTeamGame(AUTTeamInfo* Winner, FName Reason);

	virtual bool UTIsHandlingReplays() override { return false; }
	virtual void StopRCTFReplayRecording();

protected:

	UPROPERTY()
	int32 InitialBoostCount;

	// If true, players who join during the round, or switch teams during the round will be forced to
	// sit out and wait for the next round/
	UPROPERTY()
	bool bSitOutDuringRound;

};