// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTWeap_Enforcer.h"
#include "UTWeaponState.h"
#include "UTWeaponStateActive.h"
#include "UTWeaponStateFiring.h"
#include "UTWeaponStateFiring_Enforcer.h"
#include "UTWeaponStateFiringBurstEnforcer.h"
#include "UTWeaponStateEquipping_Enforcer.h"
#include "UTWeaponStateUnequipping_Enforcer.h"
#include "Particles/ParticleSystemComponent.h"
#include "UTImpactEffect.h"
#include "UnrealNetwork.h"
#include "UTWeaponAttachment.h"
#include "StatNames.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

AUTWeap_Enforcer::AUTWeap_Enforcer(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultGroup = 2;
	Ammo = 20;
	MaxAmmo = 40;
	LastFireTime = 0.f;
	SpreadResetInterval = 1.f;
	SpreadIncrease = 0.01f;
	MaxSpread = 0.05f;
	VerticalSpreadScaling = 8.f;
	BringUpTime = 0.28f;
	DualBringUpTime = 0.36f;
	PutDownTime = 0.2f;
	DualPutDownTime = 0.3f;
	StoppingPower = 30000.f;
	BaseAISelectRating = 0.4f;
	FireCount = 0;
	ImpactCount = 0;
	bDualEnforcerMode = false;
	bBecomeDual = false;
	bCanThrowWeapon = false;
	bFireLeftSide = false;
	FOVOffset = FVector(0.7f, 1.f, 1.f);
	MaxTracerDist = 2500.f;
	bNoDropInTeamSafe = true;
	ReloadClipTime = 2.0f;
	
	LeftMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("LeftMesh"));
	LeftMesh->SetOnlyOwnerSee(true);
	LeftMesh->SetupAttachment(RootComponent);
	LeftMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	LeftMesh->bSelfShadowOnly = true;
	LeftMesh->bHiddenInGame = true;
	FirstPLeftMeshOffset = FVector(0.f);

	EnforcerEquippingState = ObjectInitializer.CreateDefaultSubobject<UUTWeaponStateEquipping_Enforcer>(this, TEXT("EnforcerEquippingState"));
	EnforcerUnequippingState = ObjectInitializer.CreateDefaultSubobject<UUTWeaponStateUnequipping_Enforcer>(this, TEXT("EnforcerUnequippingState"));

	KillStatsName = NAME_EnforcerKills;
	DeathStatsName = NAME_EnforcerDeaths;
	HitsStatsName = NAME_EnforcerHits;
	ShotsStatsName = NAME_EnforcerShots;

	DisplayName = NSLOCTEXT("UTWeap_Enforcer", "DisplayName", "Enforcer");
	bCheckHeadSphere = true;
	bCheckMovingHeadSphere = true;
	bIgnoreShockballs = true;

	WeaponCustomizationTag = EpicWeaponCustomizationTags::Enforcer;
	WeaponSkinCustomizationTag = EpicWeaponSkinCustomizationTags::Enforcer;

	HighlightText = NSLOCTEXT("Weapon", "EnforcerHighlightText", "Gunslinger");
}

float AUTWeap_Enforcer::GetPutDownTime()
{
	return bDualEnforcerMode ? DualPutDownTime : PutDownTime;
}

float AUTWeap_Enforcer::GetBringUpTime()
{
	return bDualEnforcerMode ? DualBringUpTime : BringUpTime;
}

float AUTWeap_Enforcer::GetImpartedMomentumMag(AActor* HitActor)
{
	AUTCharacter* HitChar = Cast<AUTCharacter>(HitActor);
	if (HitChar && HitChar->IsDead())
	{
		return 20000.f;
	}
	return (HitChar && HitChar->GetWeapon() && HitChar->GetWeapon()->bAffectedByStoppingPower)
		? StoppingPower
		: InstantHitInfo[CurrentFireMode].Momentum;
}

void AUTWeap_Enforcer::FireShot()
{
	Super::FireShot();

	if (GetNetMode() != NM_DedicatedServer)
	{
		FireCount++;

		UUTWeaponStateFiringBurst* BurstFireMode = Cast<UUTWeaponStateFiringBurst>(FiringState[GetCurrentFireMode()]);
		if ((BurstFireMode && FireCount >= BurstFireMode->BurstSize * 2) || (!BurstFireMode && FireCount > 1))
		{
			FireCount = 0;
		}
	}

}

void AUTWeap_Enforcer::FireInstantHit(bool bDealDamage, FHitResult* OutHit)
{
	// burst mode takes care of spread variation itself
	if (!Cast<UUTWeaponStateFiringBurst>(FiringState[GetCurrentFireMode()]))
	{
		ModifySpread();
	}

	Super::FireInstantHit(bDealDamage, OutHit);
	if (UTOwner)
	{
		LastFireTime = UTOwner->GetWorld()->GetTimeSeconds();
	}
}

void AUTWeap_Enforcer::ModifySpread_Implementation()
{
	float TimeSinceFired = UTOwner->GetWorld()->GetTimeSeconds() - LastFireTime;
	float SpreadScalingOverTime = FMath::Max(0.f, 1.f - (TimeSinceFired - FireInterval[GetCurrentFireMode()]) / (SpreadResetInterval - FireInterval[GetCurrentFireMode()]));
	Spread[GetCurrentFireMode()] = FMath::Min(MaxSpread, Spread[GetCurrentFireMode()] + SpreadIncrease) * SpreadScalingOverTime;
}

void AUTWeap_Enforcer::StateChanged()
{
	if (!FiringState.Contains(Cast<UUTWeaponStateFiring>(CurrentState)))
	{
		FireCount = 0;
		ImpactCount = 0;
	}

	//Reset bFireLeftSide everytime we stop firing and go back to active
	if (Cast<UUTWeaponStateActive>(CurrentState))
	{
		bFireLeftSide = false;
	}

	Super::StateChanged();
}

void AUTWeap_Enforcer::PlayFiringEffects()
{
	UUTWeaponStateFiringBurstEnforcer* BurstFireMode = Cast<UUTWeaponStateFiringBurstEnforcer>(FiringState[GetCurrentFireMode()]);

	if (UTOwner != NULL)
	{
		//Firing single right enforcer
		if (!bDualEnforcerMode)
		{
			if (!BurstFireMode || BurstFireMode->CurrentShot == 0)
			{
				Super::PlayFiringEffects();
			}
			else if (ShouldPlay1PVisuals())
			{
				UTOwner->TargetEyeOffset.X = FiringViewKickback;
				// muzzle flash
				if (MuzzleFlash.IsValidIndex(CurrentFireMode) && MuzzleFlash[CurrentFireMode] != NULL && MuzzleFlash[CurrentFireMode]->Template != NULL)
				{
					// if we detect a looping particle system, then don't reactivate it
					if (!MuzzleFlash[CurrentFireMode]->bIsActive || !IsLoopingParticleSystem(MuzzleFlash[CurrentFireMode]->Template))
					{
						MuzzleFlash[CurrentFireMode]->ActivateSystem();
					}
				}
			}
		}
		else
		{
			if (FPFireSound.IsValidIndex(CurrentFireMode) && FPFireSound[CurrentFireMode] != NULL && Cast<APlayerController>(UTOwner->Controller) != NULL && UTOwner->IsLocallyControlled())
			{
				UUTGameplayStatics::UTPlaySound(GetWorld(), FPFireSound[CurrentFireMode], UTOwner, SRT_AllButOwner, false, FVector::ZeroVector, GetCurrentTargetPC(), NULL, true, SAT_WeaponFire);
			}
			else
			{
				UUTGameplayStatics::UTPlaySound(GetWorld(), FireSound[CurrentFireMode], UTOwner, SRT_AllButOwner, false, FVector::ZeroVector, GetCurrentTargetPC(), NULL, true, SAT_WeaponFire);
			}

			if (UTOwner && ShouldPlay1PVisuals())
			{
				UAnimInstance* HandAnimInstance = UTOwner->FirstPersonMesh ? UTOwner->FirstPersonMesh->GetAnimInstance() : nullptr;

				if (!BurstFireMode || (BurstFireMode->CurrentShot == 0))
				{
					//Firing Dual Enforcer Right Gun
					if (!bFireLeftSide)
					{
						if ((HandAnimInstance) && Dual_FireAnimationRightHand.IsValidIndex(CurrentFireMode) && (Dual_FireAnimationRightHand[CurrentFireMode] != NULL))
						{
							HandAnimInstance->Montage_Play(Dual_FireAnimationRightHand[CurrentFireMode], UTOwner->GetFireRateMultiplier());
						}

						if (Mesh && Dual_FireAnimationRightWeapon.IsValidIndex(CurrentFireMode) && (Dual_FireAnimationRightWeapon[CurrentFireMode] != NULL))
						{
							UAnimInstance* RightGunAnimInstance = Mesh->GetAnimInstance();
							if (RightGunAnimInstance)
							{
								RightGunAnimInstance->Montage_Play(Dual_FireAnimationRightWeapon[CurrentFireMode], UTOwner->GetFireRateMultiplier());
							}
						}
					}
					// Firing Dual Enforcer Left Gun
					else
					{
						if ((HandAnimInstance) && Dual_FireAnimationLeftHand.IsValidIndex(CurrentFireMode) && (Dual_FireAnimationLeftHand[CurrentFireMode] != NULL))
						{
							HandAnimInstance->Montage_Play(Dual_FireAnimationLeftHand[CurrentFireMode], UTOwner->GetFireRateMultiplier());
						}

						if (Mesh && Dual_FireAnimationLeftWeapon.IsValidIndex(CurrentFireMode) && (Dual_FireAnimationLeftWeapon[CurrentFireMode] != NULL))
						{
							UAnimInstance* LeftGunAnimInstance = LeftMesh->GetAnimInstance();
							if (LeftGunAnimInstance)
							{
								LeftGunAnimInstance->Montage_Play(Dual_FireAnimationLeftWeapon[CurrentFireMode], UTOwner->GetFireRateMultiplier());
							}
						}
					}
				}
			}

			//Alternate every shot, or every volley in burst mode
			if (!BurstFireMode || ((BurstFireMode->CurrentShot / BurstFireMode->BurstSize) == 0))
			{
				bFireLeftSide = !bFireLeftSide;
			}
		}
	}
}

void AUTWeap_Enforcer::PlayImpactEffects_Implementation(const FVector& TargetLoc, uint8 FireMode, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	UUTWeaponStateFiringBurst* BurstFireMode = Cast<UUTWeaponStateFiringBurst>(FiringState[GetCurrentFireMode()]);
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (bDualEnforcerMode && (BurstFireMode ? (FireCount / BurstFireMode->BurstSize != 0) : bFireLeftSide))
		{
			// fire effects
			static FName NAME_HitLocation(TEXT("HitLocation"));
			static FName NAME_LocalHitLocation(TEXT("LocalHitLocation"));

			//TODO: This is a really ugly solution, what if somebody modifies this later
			//Is the best solution really to split out a separate MuzzleFlash too??
			uint8 LeftHandMuzzleFlashIndex = CurrentFireMode + 2;
			const FVector LeftSpawnLocation = (MuzzleFlash.IsValidIndex(LeftHandMuzzleFlashIndex) && MuzzleFlash[LeftHandMuzzleFlashIndex] != NULL) ? MuzzleFlash[LeftHandMuzzleFlashIndex]->GetComponentLocation() : UTOwner->GetActorLocation() + UTOwner->GetControlRotation().RotateVector(FireOffset);
			if (FireEffect.IsValidIndex(CurrentFireMode) && FireEffect[CurrentFireMode] != NULL)
			{
				UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect[CurrentFireMode], LeftSpawnLocation, (MuzzleFlash.IsValidIndex(LeftHandMuzzleFlashIndex) && MuzzleFlash[LeftHandMuzzleFlashIndex] != NULL) ? MuzzleFlash[LeftHandMuzzleFlashIndex]->GetComponentRotation() : (TargetLoc - SpawnLocation).Rotation(), true);
				FVector AdjustedTargetLoc = ((TargetLoc - LeftSpawnLocation).SizeSquared() > 4000000.f)
					? LeftSpawnLocation + MaxTracerDist * (TargetLoc - LeftSpawnLocation).GetSafeNormal()
					: TargetLoc;
				PSC->SetVectorParameter(NAME_HitLocation, AdjustedTargetLoc);
				PSC->SetVectorParameter(NAME_LocalHitLocation, PSC->ComponentToWorld.InverseTransformPosition(AdjustedTargetLoc));
			}
			// perhaps the muzzle flash also contains hit effect (constant beam, etc) so set the parameter on it instead
			else if (MuzzleFlash.IsValidIndex(LeftHandMuzzleFlashIndex) && MuzzleFlash[LeftHandMuzzleFlashIndex] != NULL)
			{
				MuzzleFlash[LeftHandMuzzleFlashIndex]->SetVectorParameter(NAME_HitLocation, TargetLoc);
				MuzzleFlash[LeftHandMuzzleFlashIndex]->SetVectorParameter(NAME_LocalHitLocation, MuzzleFlash[LeftHandMuzzleFlashIndex]->ComponentToWorld.InverseTransformPositionNoScale(TargetLoc));
			}

			if ((TargetLoc - LastImpactEffectLocation).Size() >= ImpactEffectSkipDistance || GetWorld()->TimeSeconds - LastImpactEffectTime >= MaxImpactEffectSkipTime)
			{
				if (ImpactEffect.IsValidIndex(CurrentFireMode) && ImpactEffect[CurrentFireMode] != NULL)
				{
					FHitResult ImpactHit = GetImpactEffectHit(UTOwner, LeftSpawnLocation, TargetLoc);
					if (!CancelImpactEffect(ImpactHit))
					{
						ImpactEffect[CurrentFireMode].GetDefaultObject()->SpawnEffect(GetWorld(), FTransform(ImpactHit.Normal.Rotation(), ImpactHit.Location), ImpactHit.Component.Get(), NULL, UTOwner->Controller);
					}
				}
				LastImpactEffectLocation = TargetLoc;
				LastImpactEffectTime = GetWorld()->TimeSeconds;
			}
		}
		else
		{
			Super::PlayImpactEffects_Implementation(TargetLoc, FireMode, SpawnLocation, SpawnRotation);
		}

		ImpactCount++;

		if ((BurstFireMode && ImpactCount >= BurstFireMode->BurstSize * 2) || (!BurstFireMode && ImpactCount > 1))
		{
			ImpactCount = 0;
		}
	}
}

bool AUTWeap_Enforcer::StackPickup_Implementation(AUTInventory* ContainedInv)
{
	if (!bBecomeDual)
	{
		BecomeDual();
	}
	return Super::StackPickup_Implementation(ContainedInv);
}

void AUTWeap_Enforcer::BecomeDual()
{
	if (Role == ROLE_Authority)
	{
		if (bBecomeDual)
		{
			return;
		}

		MaxAmmo *= 2;
	}
	bBecomeDual = true;

	//For spectators this may not have been set
	if (EnforcerEquippingState->EquipTime == 0.0f)
	{
		EnforcerEquippingState->EquipTime = GetBringUpTime();
	}

	// pick up the second enforcer
	AttachLeftMesh();

	// the UneqippingState needs to be updated so that both guns are lowered during weapon switch
	UnequippingState = EnforcerUnequippingState;

	BaseAISelectRating = FMath::Max<float>(BaseAISelectRating, 0.6f);

	//Setup a timer to fire once the equip animation finishes
	FTimerHandle TempHandle;
	GetWorldTimerManager().SetTimer(TempHandle, this, &AUTWeap_Enforcer::DualEquipFinished, EnforcerEquippingState->EquipTime);
}

void AUTWeap_Enforcer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AUTWeap_Enforcer, bBecomeDual, COND_None);
}

void AUTWeap_Enforcer::DualEquipFinished()
{
	if (!bDualEnforcerMode)
	{
		bDualEnforcerMode = true;
		FireInterval = FireIntervalDualWield;

		//Reset the FireRate timer
		if (Cast<UUTWeaponStateFiring>(CurrentState) != NULL)
		{
			((UUTWeaponStateFiring*)CurrentState)->UpdateTiming();
		}

		//Update the animation since the stance has changed
		//Change the weapon attachment
		AttachmentType = DualWieldAttachmentType;

		if (UTOwner != NULL && UTOwner->GetWeapon() == this)
		{
			GetUTOwner()->SetWeaponAttachmentClass(AttachmentType);
			if (ShouldPlay1PVisuals())
			{
				UpdateWeaponHand();
			}
		}
		
		if (Role == ROLE_Authority)
		{
			OnRep_AttachmentType();
		}
	}
}

bool AUTWeap_Enforcer::HasAnyAmmo()
{
	// can always reload
	return true;
}

void AUTWeap_Enforcer::GotoState(UUTWeaponState* NewState)
{
	Super::GotoState(NewState);

	if ((CurrentState == ActiveState) && (Role == ROLE_Authority) && !Super::HasAnyAmmo() && UTOwner)
	{
		// @TODO FIXMESTEVE - if keep this functionality and have animation, need full weapon state to support
		if (Cast<AUTPlayerController>(UTOwner->GetController()))
		{
			GetWorldTimerManager().SetTimer(ReloadSoundHandle, this, &AUTWeap_Enforcer::PlayReloadSound, 0.5f, false);
		}
		GetWorldTimerManager().SetTimer(ReloadClipHandle, this, &AUTWeap_Enforcer::ReloadClip, ReloadClipTime, false);
	}
}

void AUTWeap_Enforcer::PlayReloadSound()
{
	if (!Super::HasAnyAmmo() && (CurrentState == ActiveState) && (Role == ROLE_Authority))
	{
		Cast<AUTPlayerController>(UTOwner->GetController())->UTClientPlaySound(ReloadClipSound);
	}
}

void AUTWeap_Enforcer::ReloadClip()
{
	if (!Super::HasAnyAmmo() && (CurrentState == ActiveState) && (Role == ROLE_Authority))
	{
		AddAmmo(20);
	}
}

void AUTWeap_Enforcer::BringUp(float OverflowTime)
{
	Super::BringUp(OverflowTime);
	
	if ((Dual_BringUpHand != NULL) && UTOwner && UTOwner->FirstPersonMesh)
	{
		UAnimInstance* HandAnimInstance = UTOwner->FirstPersonMesh->GetAnimInstance();
		if (HandAnimInstance != NULL)
		{
			HandAnimInstance->Montage_Play(Dual_BringUpHand, Dual_BringUpHand->SequenceLength / EnforcerEquippingState->EquipTime);
		}
	}
}

bool AUTWeap_Enforcer::PutDown()
{
	const bool Result = Super::PutDown();
	
	if ((Result))
	{
		if (bDualEnforcerMode)
		{
			if (UTOwner && UTOwner->FirstPersonMesh)
			{
				UAnimInstance* HandsAnimInstance = UTOwner->FirstPersonMesh->GetAnimInstance();
				if (HandsAnimInstance)
				{
					HandsAnimInstance->Montage_Play(Dual_PutDownHand, Dual_PutDownHand->SequenceLength / EnforcerEquippingState->EquipTime);
				}
			}

			if (LeftMesh && Dual_PutDownLeftWeapon)
			{
				UAnimInstance* LeftAnimInstance = LeftMesh->GetAnimInstance();
				if (LeftAnimInstance)
				{
					LeftAnimInstance->Montage_Play(Dual_PutDownLeftWeapon, Dual_PutDownLeftWeapon->SequenceLength / EnforcerEquippingState->EquipTime);
				}
			}

			if (Mesh && Dual_PutDownRightWeapon)
			{
				UAnimInstance* RightAnimInstance = Mesh->GetAnimInstance();
				if (RightAnimInstance)
				{
					RightAnimInstance->Montage_Play(Dual_PutDownRightWeapon, Dual_PutDownRightWeapon->SequenceLength / EnforcerEquippingState->EquipTime);
				}
			}
		}
	}
	
	return Result;
}

void AUTWeap_Enforcer::GotoEquippingState(float OverflowTime)
{
	GotoState(EnforcerEquippingState);
	if (CurrentState == EnforcerEquippingState)
	{
		EnforcerEquippingState->StartEquip(OverflowTime);
	}
}

void AUTWeap_Enforcer::UpdateOverlays()
{
	UpdateOverlaysShared(this, GetUTOwner(), Mesh, OverlayEffectParams, OverlayMesh);
	if (bBecomeDual)
	{
		UpdateOverlaysShared(this, GetUTOwner(), LeftMesh, OverlayEffectParams, LeftOverlayMesh);
	}
}

void AUTWeap_Enforcer::SetSkin(UMaterialInterface* NewSkin)
{
	if (LeftMesh != NULL)
	{
		if (NewSkin != NULL)
		{
			for (int32 i = 0; i < LeftMesh->GetNumMaterials(); i++)
			{
				LeftMesh->SetMaterial(i, NewSkin);
			}
		}
		else
		{
			for (int32 i = 0; i < LeftMesh->GetNumMaterials(); i++)
			{
				LeftMesh->SetMaterial(i, GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->LeftMesh->GetMaterial(i));
			}
		}
	}

	Super::SetSkin(NewSkin);
}


void AUTWeap_Enforcer::AttachLeftMesh()
{
	if (UTOwner == NULL)
	{
		return;
	}

	if (LeftMesh != NULL && LeftMesh->SkeletalMesh != NULL)
	{
		LeftMesh->SetHiddenInGame(false);
		LeftMesh->AttachToComponent(UTOwner->FirstPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, (GetWeaponHand() != EWeaponHand::HAND_Hidden) ? HandsAttachSocketLeft : NAME_None);
		if (Cast<APlayerController>(UTOwner->Controller) != NULL && UTOwner->IsLocallyControlled())
		{
			LeftMesh->LastRenderTime = GetWorld()->TimeSeconds;
			LeftMesh->bRecentlyRendered = true;
		}

		if (Dual_BringUpLeftWeaponFirstAttach != NULL)
		{
			UAnimInstance* LeftWeaponAnimInstance = LeftMesh->GetAnimInstance();
			if (LeftWeaponAnimInstance != NULL)
			{
				LeftWeaponAnimInstance->Montage_Play(Dual_BringUpLeftWeaponFirstAttach, Dual_BringUpLeftWeaponFirstAttach->SequenceLength / EnforcerEquippingState->EquipTime);
			}
		}

		if ((Dual_BringUpLeftHandFirstAttach != NULL) && UTOwner && UTOwner->FirstPersonMesh)
		{
			UAnimInstance* HandAnimInstance = UTOwner->FirstPersonMesh->GetAnimInstance();
			if (HandAnimInstance != NULL)
			{
				HandAnimInstance->Montage_Play(Dual_BringUpLeftHandFirstAttach, Dual_BringUpLeftHandFirstAttach->SequenceLength / EnforcerEquippingState->EquipTime);
			}
		}

		if (UTOwner != NULL && UTOwner->GetWeapon() == this && GetNetMode() != NM_DedicatedServer)
		{
			UpdateOverlays();
		}
		if (ShouldPlay1PVisuals())
		{
			LeftMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose; // needed for anims to be ticked even if weapon is not currently displayed, e.g. sniper zoom
			LeftMesh->LastRenderTime = GetWorld()->TimeSeconds;
			LeftMesh->bRecentlyRendered = true;
			if (LeftOverlayMesh != NULL)
			{
				LeftOverlayMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPose;
				LeftOverlayMesh->LastRenderTime = GetWorld()->TimeSeconds;
				LeftOverlayMesh->bRecentlyRendered = true;
			}
		}
	}
}

void AUTWeap_Enforcer::AttachToOwner_Implementation()
{
	if (UTOwner == NULL)
	{
		return;
	}
	
	if (bBecomeDual && !bDualEnforcerMode)
	{
		DualEquipFinished();
	}

	// attach left mesh
	if (bDualEnforcerMode)
	{
		AttachLeftMesh();
		AttachmentType = DualWieldAttachmentType;
		GetUTOwner()->SetWeaponAttachmentClass(AttachmentType);
	}

	Super::AttachToOwner_Implementation();
}

void AUTWeap_Enforcer::DetachFromOwner_Implementation()
{
	//TODO revisit this if I split the muzzle flash
	//make sure particle system really stops NOW since we're going to unregister it
	//for (int32 i = 0; i < MuzzleFlash.Num(); i++)
	//{
	//	if (MuzzleFlash[i] != NULL)
	//	{
	//		UParticleSystem* SavedTemplate = MuzzleFlash[i]->Template;
	//		MuzzleFlash[i]->DeactivateSystem();
	//		MuzzleFlash[i]->KillParticlesForced();
	//		// FIXME: KillParticlesForced() doesn't kill particles immediately for GPU particles, but the below does...
	//		MuzzleFlash[i]->SetTemplate(NULL);
	//		MuzzleFlash[i]->SetTemplate(SavedTemplate);
	//	}
	//}

	if (LeftMesh != NULL && LeftMesh->SkeletalMesh != NULL)
	{
		LeftMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	Super::DetachFromOwner_Implementation();
}

void AUTWeap_Enforcer::UpdateWeaponHand()
{
	Super::UpdateWeaponHand();
	if (bDualEnforcerMode)
	{
		FirstPLeftMeshOffset = FVector::ZeroVector;
		FirstPLeftMeshRotation = FRotator::ZeroRotator;
		switch (GetWeaponHand())
		{
			case EWeaponHand::HAND_Center:
				// TODO: not implemented, fallthrough
				UE_LOG(UT, Warning, TEXT("HAND_Center is not implemented yet!"));
			case EWeaponHand::HAND_Right:
				LeftMesh->SetRelativeLocationAndRotation(GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->LeftMesh->RelativeLocation, GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->LeftMesh->RelativeRotation);
				break;
			case EWeaponHand::HAND_Left:
			{
				// swap
				LeftMesh->SetRelativeLocationAndRotation(GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->Mesh->RelativeLocation, GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->Mesh->RelativeRotation);
				Mesh->SetRelativeLocationAndRotation(GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->LeftMesh->RelativeLocation, GetClass()->GetDefaultObject<AUTWeap_Enforcer>()->LeftMesh->RelativeRotation);
				break;
			}
			case EWeaponHand::HAND_Hidden:
			{
				Mesh->SetRelativeLocationAndRotation(FVector(-50.0f, 20.0f, -50.0f), FRotator::ZeroRotator);
				LeftMesh->SetRelativeLocationAndRotation(FVector(-50.0f, -20.0f, -50.0f), FRotator::ZeroRotator);
				break;
			}
		}
	}
}

void AUTWeap_Enforcer::PlayWeaponAnim(UAnimMontage* WeaponAnim, UAnimMontage* HandsAnim /* = NULL */, float RateOverride /* = 0.0f */)
{
	//Ignore this function if we are in Dual Enforcer Mode as we are manually handing weapon anims
	if (!bDualEnforcerMode)
	{
		Super::PlayWeaponAnim(WeaponAnim, HandsAnim, RateOverride);
	}
}

void AUTWeap_Enforcer::FiringInfoUpdated_Implementation(uint8 InFireMode, uint8 FlashCount, FVector InFlashLocation)
{
	CurrentFireMode = InFireMode;
	UUTWeaponStateFiringBurst* BurstFireMode = Cast<UUTWeaponStateFiringBurst>(FiringState[GetCurrentFireMode()]);
	if (InFlashLocation.IsZero())
	{
		FireCount = 0;
		ImpactCount = 0;
		if (BurstFireMode != nullptr)
		{
			BurstFireMode->CurrentShot = 0;
		}
	}
	else
	{
		PlayFiringEffects();

		FireCount++;
		if (BurstFireMode)
		{
			BurstFireMode->CurrentShot++;
		}

		if ((BurstFireMode && FireCount >= BurstFireMode->BurstSize * 2) || (!BurstFireMode && FireCount > 1))
		{
			FireCount = 0;
		}
		if (BurstFireMode && BurstFireMode->CurrentShot >= BurstFireMode->BurstSize)
		{
			BurstFireMode->CurrentShot = 0;
		}
	}
}