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

	/*  If true, trying to deliver own flag to enemy base */ 
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		bool bCarryOwnFlag;

	/*  If true, no auto return flag on touch */ 
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		bool bNoFlagReturn;

	/*  If true, slow flag carrier */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
		bool bSlowFlagCarrier;

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

	virtual void PlayEndOfMatchMessage() override;
	virtual void DefaultTimer() override;

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
	virtual void ResetFlags();
	virtual void InitDelayedFlag(class AUTCarriedObject* Flag);
	virtual void InitFlagForRound(class AUTCarriedObject* Flag);
	virtual void IntermissionSwapSides();
	virtual void FlagCountDown();
	virtual void FlagsAreReady();
	virtual void InitGameStateForRound();

	virtual int32 GetDefenseScore();

	virtual AActor* SetIntermissionCameras(uint32 TeamToWatch);

	virtual void ScoreRedAlternateWin();
	virtual void ScoreBlueAlternateWin();

	virtual void BeginGame() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleFlagCapture(AUTCharacter* HolderPawn, AUTPlayerState* Holder) override;
	virtual void HandleExitingIntermission() override;
	virtual int32 IntermissionTeamToView(AUTPlayerController* PC) override;
	virtual void CreateGameURLOptions(TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps);
	virtual void HandleMatchIntermission() override;
	virtual void ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType) override;
	virtual void HandleRollingAttackerRespawn(AUTPlayerState* OtherPS);
	virtual float AdjustNearbyPlayerStartScore(const AController* Player, const AController* OtherController, const ACharacter* OtherCharacter, const FVector& StartLoc, const APlayerStart* P) override;
	virtual void InitGameState() override;

	virtual void EndTeamGame(AUTTeamInfo* Winner, FName Reason);

	virtual bool UTIsHandlingReplays() override { return false; }
	virtual void StopRCTFReplayRecording();

	/** Initialize for new round. */
	virtual void InitRound();

	/** Initialize a player for the new round. */
	virtual void InitPlayerForRound(AUTPlayerState* PS);

	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	virtual bool IsTeamOnOffense(int32 TeamNumber) const;
	virtual bool IsTeamOnDefense(int32 TeamNumber) const;
	virtual bool IsPlayerOnLifeLimitedTeam(AUTPlayerState* PlayerState) const;

	UPROPERTY()
	int32 InitialBoostCount;

	// If true, players who join during the round, or switch teams during the round will be forced to
	// sit out and wait for the next round/
	UPROPERTY()
	bool bSitOutDuringRound;

};