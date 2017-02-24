// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTWeaponAttachment.h"
#include "UTWeapAttachment_LightningRifle.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTWeapAttachment_LightningRifle : public AUTWeaponAttachment
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LinkGun)
		UParticleSystemComponent* PoweredGlowComponent;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ModifyFireEffect_Implementation(class UParticleSystemComponent* Effect);

	UFUNCTION(BlueprintImplementableEvent)
		void ChainEffects();
};
