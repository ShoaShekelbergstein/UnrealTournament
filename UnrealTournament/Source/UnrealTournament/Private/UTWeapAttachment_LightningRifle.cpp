// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealTournament.h"
#include "UTImpactEffect.h"
#include "UTWeaponStateFiringBurst.h"
#include "UTWeapAttachment_LightningRifle.h"


AUTWeapAttachment_LightningRifle::AUTWeapAttachment_LightningRifle(const FObjectInitializer& OI)
	: Super(OI)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AUTWeapAttachment_LightningRifle::BeginPlay()
{
	Super::BeginPlay();

	TInlineComponentArray<UParticleSystemComponent*> PSComponents(this);
	for (UParticleSystemComponent* PSComponent : PSComponents)
	{
		if (PSComponent->GetName() == TEXT("PoweredGlow"))
		{
			PoweredGlowComponent = PSComponent;
			break;
		}
	}
}

void AUTWeapAttachment_LightningRifle::Tick(float DeltaSeconds)
{
	if (UTOwner && PoweredGlowComponent)
	{
		PoweredGlowComponent->SetActive(UTOwner->FlashExtra == 4);
	}
	Super::Tick(DeltaSeconds);
}	

void AUTWeapAttachment_LightningRifle::ModifyFireEffect_Implementation(class UParticleSystemComponent* Effect)
{
	if (UTOwner && (UTOwner->FlashExtra != 0))
	{
		if (UTOwner->FlashExtra == 1)
		{
			Effect->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
			CustomTimeDilation = 0.5f;
		}
		else
		{
			Effect->SetWorldScale3D(FVector(4.f, 4.f, 4.f));
			CustomTimeDilation = 0.25f;
			if (UTOwner->FlashExtra == 3)
			{
				ChainEffects();
			}
		}
	}
}





