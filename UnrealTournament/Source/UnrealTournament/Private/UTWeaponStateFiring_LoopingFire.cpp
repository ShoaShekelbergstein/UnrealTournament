// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTWeaponState.h"
#include "UTWeaponStateFiring.h"
#include "UTWeaponStateFiring_LoopingFire.h"

UUTWeaponStateFiring_LoopingFire::UUTWeaponStateFiring_LoopingFire(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsInCooldown = false;
}

void UUTWeaponStateFiring_LoopingFire::BeginState(const UUTWeaponState* PrevState)
{
	Super::BeginState(PrevState);

	bIsInCooldown = false;

	if (BeginFireAnim_Weapon || BeginFireAnim_Hands)
	{
		const float BeginFireTimerTime = BeginFireAnim_Weapon ? BeginFireAnim_Weapon->GetPlayLength() : BeginFireAnim_Hands->GetPlayLength();
	
		PlayBeginFireAnims();
		GetWorld()->GetTimerManager().SetTimer(BeginFireFinishedHandle, this, &UUTWeaponStateFiring_LoopingFire::BeginFireFinished, BeginFireTimerTime, false);
	}
}

void UUTWeaponStateFiring_LoopingFire::EndState()
{
	Super::EndState();
	GetWorld()->GetTimerManager().ClearTimer(BeginFireFinishedHandle);
}

void UUTWeaponStateFiring_LoopingFire::PlayBeginFireAnims()
{
	if (BeginFireAnim_Weapon)
	{
		UAnimInstance* AnimInstance_Weapon = GetOuterAUTWeapon()->GetMesh()->GetAnimInstance();
		if (AnimInstance_Weapon != NULL)
		{
			AnimInstance_Weapon->Montage_Play(BeginFireAnim_Weapon, 1.f);
		}
	}

	if (BeginFireAnim_Hands && GetUTOwner())
	{
		UAnimInstance* AnimInstance_Hands = GetUTOwner()->FirstPersonMesh->GetAnimInstance();
		if (AnimInstance_Hands)
		{
			AnimInstance_Hands->Montage_Play(BeginFireAnim_Hands, 1.f);
		}
	}
}

void UUTWeaponStateFiring_LoopingFire::BeginFireFinished()
{
	//Don't start looping Anims if we are already in cooldown
	if (!bIsInCooldown)
	{
		PlayLoopingFireAnims();
	}
}

void UUTWeaponStateFiring_LoopingFire::PlayLoopingFireAnims()
{
	if (LoopingFireAnim_Weapon)
	{
		UAnimInstance* AnimInstance_Weapon = GetOuterAUTWeapon()->GetMesh()->GetAnimInstance();
		if (AnimInstance_Weapon != NULL)
		{
			AnimInstance_Weapon->Montage_Play(LoopingFireAnim_Weapon, 1.f);
		}
	}

	if (LoopingFireAnim_Hands && GetUTOwner())
	{
		UAnimInstance* AnimInstance_Hands = GetUTOwner()->FirstPersonMesh->GetAnimInstance();
		if (AnimInstance_Hands)
		{
			AnimInstance_Hands->Montage_Play(LoopingFireAnim_Hands, 1.f);
		}
	}
}

void UUTWeaponStateFiring_LoopingFire::PlayEndFireAnims()
{ 
	if (EndFireAnim_Weapon)
	{
		UAnimInstance* AnimInstance_Weapon = GetOuterAUTWeapon()->GetMesh()->GetAnimInstance();
		if (AnimInstance_Weapon != NULL)
		{
			AnimInstance_Weapon->Montage_Play(EndFireAnim_Weapon, 1.f);
		}
	}

	if (EndFireAnim_Hands && GetUTOwner())
	{
		UAnimInstance* AnimInstance_Hands = GetUTOwner()->FirstPersonMesh->GetAnimInstance();
		if (AnimInstance_Hands)
		{
			AnimInstance_Hands->Montage_Play(EndFireAnim_Hands, 1.f);
		}
	}
}

bool UUTWeaponStateFiring_LoopingFire::CanContinueLoopingFire() const 
{
	// FIXME: HandleContinuedFiring() goes to state active automatically which is not what we want. That would be a better check here instead of this manual check.
	return (GetOuterAUTWeapon()->GetUTOwner()->GetPendingWeapon() == NULL && GetOuterAUTWeapon()->GetUTOwner()->IsPendingFire(GetOuterAUTWeapon()->GetCurrentFireMode()) && GetOuterAUTWeapon()->HasAmmo(GetOuterAUTWeapon()->GetCurrentFireMode()));
}

void UUTWeaponStateFiring_LoopingFire::RefireCheckTimer()
{
	//If we are cooling down, don't do anything until its finished
	if (!bIsInCooldown)
	{
		// query bot to consider whether to still fire, switch modes, etc
		AUTBot* B = Cast<AUTBot>(GetUTOwner()->Controller);
		if (B != NULL)
		{
			B->CheckWeaponFiring();
		}
	
		if (!CanContinueLoopingFire())
		{
			PlayEndFireAnims();
			GetOuterAUTWeapon()->GotoActiveState();
		}
		else
		{
			FireShot();
			CurrentShot++;
		}
	}
}

bool UUTWeaponStateFiring_LoopingFire::IsFiring() const
{
	return !bIsInCooldown;
}

void UUTWeaponStateFiring_LoopingFire::PlayCoolDownAnims()
{
	if (CoolDownAnim_Weapon)
	{
		UAnimInstance* AnimInstance_Weapon = GetOuterAUTWeapon()->GetMesh()->GetAnimInstance();
		if (AnimInstance_Weapon != NULL)
		{
			AnimInstance_Weapon->Montage_Play(CoolDownAnim_Weapon, 1.f);
		}
	}

	if (CoolDownAnim_Hands && GetUTOwner())
	{
		UAnimInstance* AnimInstance_Hands = GetUTOwner()->FirstPersonMesh->GetAnimInstance();
		if (AnimInstance_Hands)
		{
			AnimInstance_Hands->Montage_Play(CoolDownAnim_Hands, 1.f);
		}
	}
}

void UUTWeaponStateFiring_LoopingFire::EnterCooldown()
{
	if (!bIsInCooldown)
	{
		bIsInCooldown = true;
		PlayCoolDownAnims();
	}
}

void UUTWeaponStateFiring_LoopingFire::ExitCooldown()
{
	if (bIsInCooldown)
	{
		bIsInCooldown = false;

		// Restart looping fire anims if needed
		if (CanContinueLoopingFire())
		{
			PlayLoopingFireAnims();
		}
		
		RefireCheckTimer();
	}
}