// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCTFRoundGame.h"
#include "UTCTFScoring.h"
#include "UTGauntletGameState.h"
#include "UTGauntletGame.generated.h"

class AUTGauntletFlagDispenser;
class AUTGauntletGameState;

namespace MatchState
{
	// After you score, the game enters this state for a small period of time
	// so that the person who cap'd the flag can taunt.
	extern UNREALTOURNAMENT_API const FName GauntletScoreSummary;				

	// When the score summary phase is over, if there isn't a winner, the screen will fade to black
	extern UNREALTOURNAMENT_API const FName GauntletFadeToBlack;				

	// The next ROUND message is being displayed
	extern UNREALTOURNAMENT_API const FName GauntletRoundAnnounce;				
}


UCLASS()
class UNREALTOURNAMENT_API AUTGauntletGame : public AUTCTFRoundGame
{
	GENERATED_UCLASS_BODY()
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void InitFlags() override;
	virtual void ResetFlags() override;

	virtual void FlagTeamChanged(uint8 NewTeamIndex);
	
	virtual bool IsTeamStillAlive(uint8 TeamNum);
	bool CanFlagTeamSwap(uint8 NewTeamNum);

	void GameObjectiveInitialized(AUTGameObjective* Obj);

	virtual void InitRound();

	virtual int32 GetFlagCapScore() override
	{
		return 1;
	}

	virtual void PreInitializeComponents() override;

	virtual void FlagsAreReady();

	virtual bool CheckScore_Implementation(AUTPlayerState* Scorer) override;
	virtual void InitFlagForRound(class AUTCarriedObject* Flag);
	virtual void HandleExitingIntermission() override;

	virtual void CheckGameTime() override;

	virtual void BeginGame() override;

	virtual void SetMatchState(FName NewState) override;
	virtual void ScoreObject_Implementation(AUTCarriedObject* GameObject, AUTCharacter* HolderPawn, AUTPlayerState* Holder, FName Reason) override;
	virtual void HandleFlagCapture(AUTCharacter* HolderPawn, AUTPlayerState* Holder) override;
	virtual void BroadcastCTFScore(APlayerState* ScoringPlayer, AUTTeamInfo* ScoringTeam, int32 OldScore = 0);
	virtual void GiveDefaultInventory(APawn* PlayerPawn);
protected:

	UPROPERTY(BlueprintReadWrite,Category=Game)
	float ScoreSummaryDuration;

	UPROPERTY(BlueprintReadWrite,Category=Game)
	float FadeToBlackDuration;

	UPROPERTY(BlueprintReadWrite,Category=Game)
	float RoundAnnounceDuration;


	UPROPERTY()
	AUTGauntletGameState* GauntletGameState;

	// How long does a flag have to sit idle for it to return to the neutral position.  Use ?FlagSwapTime=x to set.  Set to 0 to be instantly pick-up-able 
	UPROPERTY()
	int32 FlagSwapTime;

	virtual void EndScoreSummary();
	virtual void EndFadeToBlack();
	virtual void EndRoundAnnounce();

	virtual void ForceEndOfRound();

	UPROPERTY()
	TArray<TAssetSubclassOf<AUTWeapon>> DefaultWeaponLoadoutObjects;

};