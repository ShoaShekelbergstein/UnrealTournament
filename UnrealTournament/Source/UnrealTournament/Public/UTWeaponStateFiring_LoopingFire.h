// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTWeaponStateFiring_LoopingFire.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTWeaponStateFiring_LoopingFire : public UUTWeaponStateFiring
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadWrite, Category = LoopingFire)
	bool bIsInCooldown;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* BeginFireAnim_Weapon;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* BeginFireAnim_Hands;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* LoopingFireAnim_Weapon;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* LoopingFireAnim_Hands;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* EndFireAnim_Weapon;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* EndFireAnim_Hands;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* CoolDownAnim_Weapon;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* CoolDownAnim_Hands;


protected:
	/** current shot since we started firing */
	int32 CurrentShot;
	
	virtual void PlayBeginFireAnims();
	virtual void PlayLoopingFireAnims();
	virtual void PlayEndFireAnims();
	virtual void PlayCoolDownAnims();

	FTimerHandle BeginFireFinishedHandle;
	
	UFUNCTION()
	virtual void BeginFireFinished();

public:
	virtual bool CanContinueLoopingFire() const;

	virtual void BeginState(const UUTWeaponState* PrevState) override;
	virtual void EndState() override;

	/** called after the refire delay to see what we should do next (generally, fire or go back to active state) */
	virtual void RefireCheckTimer();
	virtual bool IsFiring() const override;
	
	virtual void EnterCooldown();
	virtual void ExitCooldown();

};

