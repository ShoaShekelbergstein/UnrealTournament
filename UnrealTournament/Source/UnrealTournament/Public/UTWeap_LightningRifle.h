// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTWeap_Sniper.h"
#include "UTWeap_LightningRifle.generated.h"

UCLASS(abstract)
class UNREALTOURNAMENT_API AUTWeap_LightningRifle : public AUTWeap_Sniper
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(BlueprintReadWrite)
		float ChargePct;

	virtual void DrawWeaponCrosshair_Implementation(UUTHUDWidget* WeaponHudWidget, float RenderDelta) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		float FullPowerHeadshotDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		float FullPowerBonusDamage;

	/** How fast charge increases to value of 1 (fully charged).  Scaling for time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		float ChargeSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		USoundBase* FullyPoweredHitEnemySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		USoundBase* FullyPoweredNoHitEnemySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		USoundBase* ChargeSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = LightningRifle)
		USoundBase* FullyPoweredSound;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRepCharging, Category = LightningRifle)
		bool bIsCharging;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = LightningRifle)
		bool bIsFullyPowered;

	/** True when zoom button is pressed. */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = LightningRifle)
		bool bZoomHeld;

	UFUNCTION()
		void OnRepCharging();

	virtual bool CanHeadShot() override;
	virtual int32 GetHitScanDamage() override;
	virtual void SetFlashExtra(AActor* HitActor) override;
	virtual bool ShouldAIDelayFiring_Implementation() override;
	virtual void Tick(float DeltaTime) override;
	virtual void FireShot() override;
	virtual void OnStartedFiring_Implementation() override;
	virtual void OnContinuedFiring_Implementation() override;
	virtual void OnStoppedFiring_Implementation() override;
	virtual void OnRep_ZoomState_Implementation() override;
	virtual void Removed() override;
};
