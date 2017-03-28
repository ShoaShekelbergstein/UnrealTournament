// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealTournament.h"
#include "UTWeaponStateEquipping_Enforcer.h"
#include "UTWeaponStateUnequipping_Enforcer.h"
#include "UTWeap_Enforcer.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

void UUTWeaponStateUnequipping_Enforcer::BeginState(const UUTWeaponState* PrevState)
{
	const UUTWeaponStateEquipping* PrevEquip = Cast<UUTWeaponStateEquipping>(PrevState);

	// if was previously equipping, pay same amount of time to take back down
	UnequipTime = (PrevEquip != NULL) ? FMath::Min(PrevEquip->PartialEquipTime, GetOuterAUTWeapon()->GetPutDownTime()) : GetOuterAUTWeapon()->GetPutDownTime();
	UnequipTimeElapsed = 0.0f;
	if (UnequipTime <= 0.0f)
	{
		PutDownFinished();
	}
	else
	{
		AUTWeap_Enforcer* OuterWeapon = Cast<AUTWeap_Enforcer>(GetOuterAUTWeapon());
		GetOuterAUTWeapon()->GetWorldTimerManager().SetTimer(PutDownFinishedHandle, this, &UUTWeaponStateUnequipping_Enforcer::PutDownFinished, UnequipTime);
		if (OuterWeapon->PutDownAnim != NULL)
		{
			UAnimInstance* AnimInstance = OuterWeapon->Mesh->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(OuterWeapon->PutDownAnim, OuterWeapon->PutDownAnim->SequenceLength / UnequipTime);
			}

			AnimInstance = OuterWeapon->LeftMesh->GetAnimInstance();
			if (AnimInstance != NULL && OuterWeapon->bDualEnforcerMode)
			{
				AnimInstance->Montage_Play(OuterWeapon->PutDownAnim, OuterWeapon->PutDownAnim->SequenceLength / UnequipTime);
			}
		}
	}
}

void UUTWeaponStateEquipping_Enforcer::StartEquip(float OverflowTime)
{
	EquipTime -= OverflowTime;
	if (EquipTime <= 0.0f)
	{
		BringUpFinished();
	}
	else
	{
		GetOuterAUTWeapon()->GetWorldTimerManager().SetTimer(BringUpFinishedHandle, this, &UUTWeaponStateEquipping_Enforcer::BringUpFinished, EquipTime);
		AUTWeap_Enforcer* OuterWeapon = Cast<AUTWeap_Enforcer>(GetOuterAUTWeapon());
		
		GetOuterAUTWeapon()->PlayWeaponAnim(GetOuterAUTWeapon()->BringUpAnim, GetOuterAUTWeapon()->BringUpAnimHands, GetAnimLengthForScaling(GetOuterAUTWeapon()->BringUpAnim, GetOuterAUTWeapon()->BringUpAnimHands) / EquipTime);
	}
}
