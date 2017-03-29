#pragma  once

#include "UTLineUpZone.h"
#include "UTLineUpHelper.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTLineUpHelper : public AActor
{
	GENERATED_UCLASS_BODY()

	UFUNCTION()
	void HandleLineUp(LineUpTypes IntroType);

	UFUNCTION()
	void OnPlayerChange();

	UFUNCTION()
	bool CanInitiateGroupTaunt(AUTPlayerState* PlayerToCheck);

	UFUNCTION()
	void CleanUp();

	void ForceCharacterAnimResetForLineUp(AUTCharacter* UTChar);

	void SetupDelayedLineUp();

	UFUNCTION()
	void OnRep_LineUpInfo();

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_LineUpInfo)
	bool bIsActive;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_LineUpInfo)
	LineUpTypes LastActiveType;

	UPROPERTY()
	bool bIsPlacingPlayers;
	
	/*Handles all the clean up for a particular player when a line-up is ending*/
	static void CleanUpPlayerAfterLineUp(AUTPlayerController* UTPC);

protected:

	void ClientUpdatePlayerClones();

	UFUNCTION()
	void HandleIntro(LineUpTypes ZoneType);

	UFUNCTION()
	void HandleIntermission(LineUpTypes IntermissionType);

	UFUNCTION()
	void HandleEndMatchSummary(LineUpTypes SummaryType);

	UFUNCTION()
	void SortPlayers();

	UFUNCTION()
	void MovePlayers(LineUpTypes ZoneType);

	UFUNCTION()
	void MovePlayersDelayed(LineUpTypes ZoneType, FTimerHandle& TimerHandleToStart, float TimeDelay);

	UFUNCTION()
	void SpawnPlayerWeapon(AUTCharacter* UTChar);

	UFUNCTION()
	void DestroySpawnedClones();
	
	UFUNCTION()
	void MovePreviewCharactersToLineUpSpawns(LineUpTypes LineUpType);

	/*Flag can be in a bad state since we recreate pawns during Line Up. This function re-assigns the flag to the correct player pawn*/
	UFUNCTION()
	void FlagFixUp();

	UFUNCTION()
	void BuildMapWeaponList();

	TWeakPtr<AUTCharacter> SelectedCharacter;

	float TimerDelayForIntro;
	float TimerDelayForIntermission;
	float TimerDelayForEndMatch;

	FTimerHandle IntroHandle;
	FTimerHandle IntermissionHandle;
	FTimerHandle MatchSummaryHandle;

	/** preview actors */
	UPROPERTY()
	TArray<class AUTCharacter*> PlayerPreviewCharacters;

	/** preview weapon */
	UPROPERTY()
	TArray<class AUTWeapon*> PreviewWeapons;

	UPROPERTY()
	TArray<class UAnimationAsset*> PreviewAnimations;

	UPROPERTY()
	TArray<TSubclassOf<AUTWeapon>> MapWeaponTypeList;

};