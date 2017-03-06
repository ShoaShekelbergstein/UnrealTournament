// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTWeap_Enforcer.generated.h"

UCLASS(abstract)
class UNREALTOURNAMENT_API AUTWeap_Enforcer : public AUTWeapon
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AUTWeaponAttachment> SingleWieldAttachmentType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AUTWeaponAttachment> DualWieldAttachmentType;

	/** Left hand weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* LeftMesh;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "States")
	UUTWeaponStateEquipping* EnforcerEquippingState;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "States")
	UUTWeaponState* EnforcerUnequippingState;

	/** How much spread increases for each shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enforcer)
	float SpreadIncrease;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		USoundBase* ReloadClipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		float ReloadClipTime;

	FTimerHandle ReloadClipHandle;
	FTimerHandle ReloadSoundHandle;

	virtual void GotoState(class UUTWeaponState* NewState) override;
	virtual void ReloadClip();
	virtual void PlayReloadSound();
	virtual bool HasAnyAmmo() override;

	/** How much spread increases for each shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enforcer)
	float SpreadResetInterval;

	/** Max spread  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enforcer)
	float MaxSpread;

	/** Last time a shot was fired (used for calculating spread). */
	UPROPERTY(BlueprintReadWrite, Category = Enforcer)
	float LastFireTime;

	/** Stopping power against players with melee weapons.  Overrides normal momentum imparted by bullets. */
	UPROPERTY(BlueprintReadWrite, Category = Enforcer)
	float StoppingPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_BringUpLeftHandFirstAttach;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_BringUpLeftWeaponFirstAttach;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_BringUpHand;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_PutDownHand;
		
	/** Unequip anim for when we have dual enforcer out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_PutDownLeftWeapon;

	/** Unequip anim for when we have dual enforcer out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UAnimMontage* Dual_PutDownRightWeapon;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> Dual_FireAnimationLeftHand;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> Dual_FireAnimationRightHand;

	/** socket to attach weapon to hands; if None, then the hands are hidden */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName HandsAttachSocketLeft;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> Dual_FireAnimationLeftWeapon;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		TArray<UAnimMontage*> Dual_FireAnimationRightWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (ClampMin = "0.1"))
	TArray<float> FireIntervalDualWield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<float> SpreadDualWield;

	/** Weapon bring up time when dual wielding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float DualBringUpTime;

	/** Weapon put down time when dual wielding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float DualPutDownTime;

	virtual float GetBringUpTime() override;
	virtual float GetPutDownTime() override;

	UPROPERTY()
	FVector FirstPLeftMeshOffset;
	UPROPERTY()
	FRotator FirstPLeftMeshRotation;
		
	UPROPERTY()
	int32 FireCount;

	/** Toggle when firing dual enforcers to make them alternate. */
	UPROPERTY()
		bool bFireLeftSide;

	/**Track whether the last fired shot was from the left or right gun. Used to sync firing effects and impact effects **/
	UPROPERTY()
		bool bWasLastShotLeftSide;

	UPROPERTY()
	int32 ImpactCount;

	UPROPERTY()
	bool bDualEnforcerMode;

	UPROPERTY(BlueprintReadOnly, Replicated, ReplicatedUsing = BecomeDual, Category = "Weapon")
	bool bBecomeDual;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<UParticleSystemComponent*> LeftMuzzleFlash;

	virtual void PlayFiringEffects() override;
	virtual void FireInstantHit(bool bDealDamage, FHitResult* OutHit) override;
	virtual bool StackPickup_Implementation(AUTInventory* ContainedInv) override; 
	virtual void BringUp(float OverflowTime) override;
	virtual bool PutDown() override;
	virtual void PlayImpactEffects_Implementation(const FVector& TargetLoc, uint8 FireMode, const FVector& SpawnLocation, const FRotator& SpawnRotation) override;
	virtual void UpdateOverlays() override;
	virtual void SetSkin(UMaterialInterface* NewSkin) override;
	virtual void GotoEquippingState(float OverflowTime) override;
	virtual void FireShot() override;
	virtual void StateChanged() override;
	virtual void UpdateWeaponHand() override;
	
	virtual void PlayWeaponAnim(UAnimMontage* WeaponAnim, UAnimMontage* HandsAnim = NULL, float RateOverride = 0.0f) override;

	virtual TArray<UMeshComponent*> Get1PMeshes_Implementation() const
	{
		TArray<UMeshComponent*> Result = Super::Get1PMeshes_Implementation();
		Result.Add(LeftMesh);
		Result.Add(LeftOverlayMesh);
		return Result;
	}

	/** Switch to second enforcer mode
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void BecomeDual();
	virtual	float GetImpartedMomentumMag(AActor* HitActor) override;
	virtual void DetachFromOwner_Implementation() override;
	virtual void AttachToOwner_Implementation() override;

	virtual void DualEquipFinished();

	virtual void FiringInfoUpdated_Implementation(uint8 InFireMode, uint8 FlashCount, FVector InFlashLocation) override;

	/** Call to modify our spread */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	void ModifySpread();

protected:
	
	UPROPERTY()
	USkeletalMeshComponent* LeftOverlayMesh;

	virtual void AttachLeftMesh();
};

