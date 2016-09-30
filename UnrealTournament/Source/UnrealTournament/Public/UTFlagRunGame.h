// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "UTCTFGameState.h"
#include "UTCTFScoring.h"
#include "UTCTFRoundGame.h"
#include "UTATypes.h"
#include "UTFlagRunGame.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTFlagRunGame : public AUTCTFRoundGame
{
	GENERATED_UCLASS_BODY()

public:

	/** optional class spawned at source location after translocating that continues to receive damage for a short duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AUTWeaponRedirector> AfterImageType;

	UPROPERTY()
		float RallyRequestTime;

	UPROPERTY()
		float LastEntryDefenseWarningTime;

	UPROPERTY()
		float LastEnemyRallyWarning;

	UPROPERTY()
		int32 GoldBonusTime;

	UPROPERTY()
		int32 SilverBonusTime;

	UPROPERTY()
		int32 GoldScore;

	UPROPERTY()
		int32 SilverScore;

	UPROPERTY()
		int32 BronzeScore;

	// Score for a successful defense
	UPROPERTY()
		int32 DefenseScore;

	UPROPERTY()
		bool bAllowPrototypePowerups;

	UPROPERTY()
		bool bAllowBoosts;

	UPROPERTY(BlueprintReadOnly, Category = CTF)
		int OffenseKillsNeededForPowerUp;

	UPROPERTY(BlueprintReadOnly, Category = CTF)
		int DefenseKillsNeededForPowerUp;


	TAssetSubclassOf<class AUTInventory> ActivatedPowerupPlaceholderObject;
	TAssetSubclassOf<class AUTInventory> RepulsorObject;

	UPROPERTY()
		TSubclassOf<class AUTInventory> ActivatedPowerupPlaceholderClass;

	virtual TSubclassOf<class AUTInventory> GetActivatedPowerupPlaceholderClass() { return ActivatedPowerupPlaceholderClass; };

	UPROPERTY()
		TSubclassOf<class AUTInventory> RepulsorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
		USoundBase* RallyFailedSound;

	FTimerHandle EnemyRallyWarningHandle;

	virtual void WarnEnemyRally();

	virtual void AnnounceWin(AUTTeamInfo* WinningTeam, uint8 Reason) override;
	virtual void NotifyFirstPickup(AUTCarriedObject* Flag) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual float OverrideRespawnTime(TSubclassOf<AUTInventory> InventoryType) override;
	virtual void HandleRallyRequest(AUTPlayerController* PC) override;
	virtual void CompleteRallyRequest(AUTPlayerController* PC) override;
	virtual bool CheckForWinner(AUTTeamInfo* ScoringTeam) override;
	virtual int32 PickCheatWinTeam() override;
	virtual bool AvoidPlayerStart(class AUTPlayerStart* P) override;
	virtual void InitDelayedFlag(class AUTCarriedObject* Flag) override;
	virtual void InitFlagForRound(class AUTCarriedObject* Flag) override;
	virtual void IntermissionSwapSides() override;
	virtual int32 GetFlagCapScore() override;
	virtual int32 GetDefenseScore() override;
	virtual void BroadcastCTFScore(APlayerState* ScoringPlayer, AUTTeamInfo* ScoringTeam, int32 OldScore = 0) override;
	virtual void InitGameState() override;
	virtual void CheckRoundTimeVictory() override;
	virtual void ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType) override;
	virtual void HandleTeamChange(AUTPlayerState* PS, AUTTeamInfo* OldTeam) override;
	virtual void InitGameStateForRound() override;
	virtual bool IsTeamOnOffense(int32 TeamNumber) const override;
	virtual AActor* SetIntermissionCameras(uint32 TeamToWatch) override;
	virtual void SendRestartNotifications(AUTPlayerState* PS, AUTPlayerController* PC) override;

	virtual int32 GetComSwitch(FName CommandTag, AActor* ContextActor, AUTPlayerController* Instigator, UWorld* World);
	virtual void InitFlags() override;
	virtual void HandleMatchIntermission() override;
	virtual void CheatScore() override;
	virtual void DefaultTimer() override;

	virtual void UpdateSkillRating() override;

	virtual uint8 GetNumMatchesFor(AUTPlayerState* PS, bool bRankedSession) const override;
	virtual int32 GetEloFor(AUTPlayerState* PS, bool bRankedSession) const override;
	virtual void SetEloFor(AUTPlayerState* PS, bool bRankedSession, int32 NewELoValue, bool bIncrementMatchCount) override;

	virtual void ScoreObject_Implementation(AUTCarriedObject* GameObject, AUTCharacter* HolderPawn, AUTPlayerState* Holder, FName Reason) override;
	virtual void ScoreAlternateWin(int32 WinningTeamIndex, uint8 Reason = 0);

protected:

	virtual void HandlePowerupUnlocks(APawn* Other, AController* Killer);
	virtual void UpdatePowerupUnlockProgress(AUTPlayerState* VictimPS, AUTPlayerState* KillerPS);
	virtual void GrantPowerupToTeam(int TeamIndex, AUTPlayerState* PlayerToHighlight);
};

