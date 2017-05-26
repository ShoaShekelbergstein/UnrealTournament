#pragma  once

#include "UTLineUpZone.h"
#include "UTLineUpHelper.generated.h"


USTRUCT()
struct FLineUpSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform SpotLocation;

	UPROPERTY()
	AController* ControllerInSpot;
};

UCLASS()
class UNREALTOURNAMENT_API AUTLineUpHelper : public AActor
{
	GENERATED_UCLASS_BODY()

	UFUNCTION()
	static void ForceCharacterAnimResetForLineUp(AUTCharacter* UTChar);

	UFUNCTION()
	bool CanInitiateGroupTaunt(AUTPlayerState* PlayerToCheck);

	UFUNCTION()
	void OnPlayerChange();

	UFUNCTION()
	void InitializeLineUp(LineUpTypes LineUpType);

	UFUNCTION()
	void CleanUp();
	
	UPROPERTY(Replicated)
	LineUpTypes ActiveType;
	
	UPROPERTY(Replicated)
	bool bIsPlacingPlayers;

	/*Handles all the clean up for a particular player when a line-up is ending*/
	static void CleanUpPlayerAfterLineUp(AUTPlayerController* UTPC);

	virtual void BeginPlay() override;

	UFUNCTION()
	bool IsActive();

protected:

	UFUNCTION()
	void DestroySpawnedClones();

	UFUNCTION()
	void SpawnCharactersToSlots();

	UFUNCTION()
	void PerformLineUp();

	UFUNCTION()
	void SetLineUpWeapons();

	UFUNCTION()
	void SpawnPlayerWeapon(AUTCharacter* UTChar);
	
	/*Flag can be in a bad state since we recreate pawns during Line Up. This function re-assigns the flag to the correct player pawn*/
	UFUNCTION()
	void FlagFixUp();

	UFUNCTION()
	void BuildMapWeaponList();

	UFUNCTION()
	void NotifyClientsOfLineUp();
	
	UFUNCTION()
	void SetupCharactersForLineUp();

	float TimerDelayForIntro;
	float TimerDelayForIntermission;
	float TimerDelayForEndMatch;

	FTimerHandle DelayedLineUpHandle;

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

	UPROPERTY(Replicated)
	TArray<FLineUpSlot> LineUpSlots;

private:

	float CalculateLineUpDelay();
	void CalculateLineUpSlots();
	void StartLineUpWithDelay(float TimeDelay);
	
	void SortControllers(TArray<AController*>& ControllersToSort);
};