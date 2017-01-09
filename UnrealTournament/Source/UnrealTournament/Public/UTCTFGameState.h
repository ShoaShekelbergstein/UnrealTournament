// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCTFFlagBase.h"
#include "UTCTFGameState.generated.h"

/** used to store a player in recorded match data so that we're guaranteed to have a valid name (use PlayerState if possible, string name if not) */
USTRUCT()
struct FSafePlayerName
{
	GENERATED_USTRUCT_BODY()

	friend uint32 GetTypeHash(const FSafePlayerName& N);

	UPROPERTY()
	AUTPlayerState* PlayerState;
protected:
	UPROPERTY()
	FString PlayerName;
public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		UObject* Data = PlayerState;
		bool bWriteName = !Map->SerializeObject(Ar, AUTPlayerState::StaticClass(), Data) || Data == NULL;
		PlayerState = Cast<AUTPlayerState>(Data);
		Ar << bWriteName;
		if (bWriteName)
		{
			Ar << PlayerName;
		}
		else if (Ar.IsLoading() && PlayerState != NULL)
		{
			PlayerName = PlayerState->PlayerName;
		}
		return true;
	}

	FSafePlayerName()
	: PlayerState(NULL)
	{}
	FSafePlayerName(AUTPlayerState* InPlayerState)
	: PlayerState(InPlayerState), PlayerName(InPlayerState != NULL ? InPlayerState->PlayerName : TEXT(""))
	{}

	inline bool operator==(const FSafePlayerName& Other) const
	{
		if (PlayerState != NULL || Other.PlayerState != NULL)
		{
			return PlayerState == Other.PlayerState;
		}
		else
		{
			return PlayerName == Other.PlayerName;
		}
	}

	FString GetPlayerName() const
	{
		return (PlayerState != NULL) ? PlayerState->PlayerName : PlayerName;
	}
};
template<>
struct TStructOpsTypeTraits<FSafePlayerName> : public TStructOpsTypeTraitsBase
{
	enum
	{
		WithNetSerializer = true
	};
};
inline uint32 GetTypeHash(const FSafePlayerName& N)
{
	return GetTypeHash(N.PlayerName) + GetTypeHash(N.PlayerState);
}

USTRUCT()
struct FCTFAssist
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FSafePlayerName AssistName;

	UPROPERTY()
		bool bCarryAssist;

	UPROPERTY()
		bool bDefendAssist;

	UPROPERTY()
		bool bReturnAssist;

	inline bool operator==(const FCTFAssist& Other) const
	{
		return AssistName == Other.AssistName;
	}

};

USTRUCT()
struct FCTFScoringPlay
{
	GENERATED_USTRUCT_BODY()

	/** team that got the cap */
	UPROPERTY()
	AUTTeamInfo* Team;
	UPROPERTY()
	FSafePlayerName ScoredBy;
	UPROPERTY()
	int32 ScoredByCaps;

	UPROPERTY()
		int32 RedBonus;

	UPROPERTY()
		int32 BlueBonus;

	UPROPERTY()
		TArray<FCTFAssist> Assists;
	/** Remaining time in seconds when the cap happened */
	UPROPERTY()
		int32 RemainingTime;
	/** period in which the cap happened (0 : first half, 1 : second half, 2+: OT) */
	UPROPERTY()
	uint8 Period;

	/**For Asymmetric CTF. */
	UPROPERTY()
	bool bDefenseWon;

	UPROPERTY()
		bool bAnnihilation;

	UPROPERTY()
		int32 TeamScores[2];

	FCTFScoringPlay()
		: Team(NULL), RemainingTime(0), Period(0), bDefenseWon(false), bAnnihilation(false)
	{}
	FCTFScoringPlay(const FCTFScoringPlay& Other) = default;

	inline bool operator==(const FCTFScoringPlay& Other) const
	{
		return (Team == Other.Team && ScoredBy == Other.ScoredBy && Assists == Other.Assists && RemainingTime == Other.RemainingTime && Period == Other.Period && bDefenseWon == Other.bDefenseWon);
	}
};

UCLASS()
class UNREALTOURNAMENT_API AUTCTFGameState: public AUTGameState
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly,Replicated,Category = CTF)
	uint32 bSecondHalf : 1;

	UPROPERTY(BlueprintReadOnly,Replicated,ReplicatedUsing=OnIntermissionChanged, Category = CTF)
	uint32 bIsAtIntermission : 1;

	/** The Elapsed time at which Overtime began */
	UPROPERTY(BlueprintReadOnly, Category = CTF)
	int32 OvertimeStartTime;

	/** Will be true if the game is playing advantage going in to half-time */
	UPROPERTY(Replicated)
		uint32 bPlayingAdvantage : 1;

	/** Delay before bringing up scoreboard at halftime. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = CTF)
		float HalftimeScoreDelay;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = CTF)
	TArray<AUTCTFFlagBase*> FlagBases;

	UPROPERTY(Replicated)
	uint8 AdvantageTeamIndex;

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 CTFRound;

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 NumRounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText RedAdvantageStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText BlueAdvantageStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText RoundInProgressStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText FullRoundInProgressStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText IntermissionStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText HalftimeStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText ExtendedOvertimeStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText FirstHalfStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameState)
		FText SecondHalfStatus;

	/** Sets the # of teams.  This will also Pre-seed FlagsBases */
	virtual void SetMaxNumberOfTeams(int32 TeamCount);

	/** Cache a flag by in the FlagBases array */
	virtual void CacheFlagBase(AUTCTFFlagBase* BaseToCache);

	/** Returns the current state of a given flag */
	virtual FName GetFlagState(uint8 TeamNum);

	UFUNCTION(BlueprintCallable, Category = GameState)
	virtual AUTPlayerState* GetFlagHolder(uint8 TeamNum);
	virtual AUTCTFFlagBase* GetFlagBase(uint8 TeamNum);

	virtual void ResetFlags();

	/** Find the current team that is in the lead */
	virtual AUTTeamInfo* FindLeadingTeam();

	virtual float GetClockTime() override;
	virtual bool IsMatchInProgress() const override;
	virtual bool IsMatchInOvertime() const override;
	virtual bool IsMatchIntermission() const override;
	virtual void OnRep_MatchState() override;

	virtual FName OverrideCameraStyle(APlayerController* PCOwner, FName CurrentCameraStyle);
	
	UFUNCTION()
		virtual void OnIntermissionChanged();

	virtual void ToggleScoreboards();

	virtual void UpdateHighlights_Implementation() override;
	virtual void AddMinorHighlights_Implementation(AUTPlayerState* PS) override;

private:
	/** list of scoring plays
	 * replicating dynamic arrays is dangerous for bandwidth and performance, but the alternative in this case is some painful code so we're as safe as possible by tightly restricting access
	 */
	UPROPERTY(Replicated)
	TArray<FCTFScoringPlay> ScoringPlays;

public:
	inline const TArray<const FCTFScoringPlay>& GetScoringPlays() const
	{
		return *(const TArray<const FCTFScoringPlay>*)&ScoringPlays;
	}
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Scoring)
	virtual void AddScoringPlay(const FCTFScoringPlay& NewScoringPlay);

	virtual FText GetGameStatusText(bool bForScoreboard) override;

	virtual float ScoreCameraView(AUTPlayerState* InPS, AUTCharacter *Character) override;

	virtual uint8 NearestTeamSide(AActor* InActor) override;

	bool GetImportantPickups_Implementation(TArray<AUTPickup*>& PickupList);
};