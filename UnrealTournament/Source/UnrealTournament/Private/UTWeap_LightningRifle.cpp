// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UnrealNetwork.h"
#include "StatNames.h"
#include "UTWeap_LightningRifle.h"
#include "UTWeapon.h"

AUTWeap_LightningRifle::AUTWeap_LightningRifle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FullPowerBonusDamage = 40.f;
	HeadshotDamage = 125.f;
	ChargeSpeed = 1.25f;
}

void AUTWeap_LightningRifle::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUTWeap_LightningRifle, bIsCharging);
	DOREPLIFETIME(AUTWeap_LightningRifle, bIsFullyPowered);
}

void AUTWeap_LightningRifle::OnStartedFiring_Implementation()
{
	Super::OnStartedFiring_Implementation();
	bZoomHeld = false;
	bIsCharging = false;
	ChargePct = 0.f;
}

void AUTWeap_LightningRifle::OnContinuedFiring_Implementation()
{
	Super::OnContinuedFiring_Implementation();
	bZoomHeld = false;
	bIsFullyPowered = false;
	bIsCharging = false;
	ChargePct = 0.f;
}

void AUTWeap_LightningRifle::OnStoppedFiring_Implementation()
{
	Super::OnStoppedFiring_Implementation();
	if (!bZoomHeld && (CurrentFireMode == 0) && (Role == ROLE_Authority))
	{
		bIsFullyPowered = false;
		bZoomHeld = true;
		if ((ZoomState == EZoomState::EZS_Zoomed) || (ZoomState == EZoomState::EZS_ZoomingIn))
		{
			bIsCharging = true;
		}
	}
}

void AUTWeap_LightningRifle::Removed()
{
	bZoomHeld = false;
	bIsFullyPowered = false;
	bIsCharging = false;
	Super::Removed();
}

void AUTWeap_LightningRifle::OnRep_ZoomState_Implementation()
{
	Super::OnRep_ZoomState_Implementation();
	if (ZoomState == EZoomState::EZS_NotZoomed)
	{
		bZoomHeld = false;
		ChargePct = 0.f;
		bIsFullyPowered = false;
		bIsCharging = false;
	}
	else if (ZoomState == EZoomState::EZS_ZoomingIn)
	{
		bZoomHeld = true;
		if (Role == ROLE_Authority)
		{
			bIsCharging = true;
		}
	}
}

void AUTWeap_LightningRifle::DrawWeaponCrosshair_Implementation(UUTHUDWidget* WeaponHudWidget, float RenderDelta)
{
	Super::DrawWeaponCrosshair_Implementation(WeaponHudWidget, RenderDelta);

	if ((ChargePct > 0.f) && (ZoomState != EZoomState::EZS_NotZoomed) && WeaponHudWidget && WeaponHudWidget->UTHUDOwner)
	{
		float Width = 150.f;
		float Height = 21.f;
		float WidthScale = WeaponHudWidget->GetRenderScale();
		float HeightScale = WidthScale;
		WidthScale *= 0.75f;
		//	WeaponHudWidget->DrawTexture(WeaponHudWidget->UTHUDOwner->HUDAtlas, 0.f, 96.f, Scale*Width, Scale*Height, 127, 671, Width, Height, 0.7f, FLinearColor::White, FVector2D(0.5f, 0.5f));
		WeaponHudWidget->DrawTexture(WeaponHudWidget->UTHUDOwner->HUDAtlas, 0.f, 32.f, WidthScale*Width*ChargePct, HeightScale*Height, 127, 641, Width, Height, 0.7f, BLUEHUDCOLOR, FVector2D(0.5f, 0.5f));
		if (ChargePct == 1.f)
		{
			WeaponHudWidget->DrawText(NSLOCTEXT("LightningRifle", "Charged", "CHARGED"), 0.f, 28.f, WeaponHudWidget->UTHUDOwner->TinyFont, 1.f, 1.f, FLinearColor::Yellow, ETextHorzPos::Center, ETextVertPos::Center);
		}
		WeaponHudWidget->DrawTexture(WeaponHudWidget->UTHUDOwner->HUDAtlas, 0.f, 32.f, WidthScale*Width, HeightScale*Height, 127, 612, Width, Height, 1.f, FLinearColor::White, FVector2D(0.5f, 0.5f));
	}
}

bool AUTWeap_LightningRifle::CanHeadShot()
{
	return bIsFullyPowered;
}

int32 AUTWeap_LightningRifle::GetHitScanDamage()
{
	return InstantHitInfo[CurrentFireMode].Damage + (bIsFullyPowered ? FullPowerBonusDamage : 0.f);
}

void AUTWeap_LightningRifle::SetFlashExtra(AActor* HitActor)
{
	if (UTOwner)
	{
		if (!bIsCharging)
		{
			UTOwner->SetFlashExtra(0, CurrentFireMode);
		}
		else if (bIsFullyPowered)
		{
			AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
			if (Cast<AUTCharacter>(HitActor) && GS && !GS->OnSameTeam(UTOwner, HitActor))
			{
				UTOwner->SetFlashExtra(3, CurrentFireMode);
				UUTGameplayStatics::UTPlaySound(GetWorld(), FullyPoweredHitEnemySound, UTOwner, SRT_AllButOwner, false, FVector::ZeroVector, GetCurrentTargetPC(), NULL, true, FireSoundAmp);
			}
			else
			{
				UTOwner->SetFlashExtra(2, CurrentFireMode);
				UUTGameplayStatics::UTPlaySound(GetWorld(), FullyPoweredNoHitEnemySound, UTOwner, SRT_AllButOwner, false, FVector::ZeroVector, GetCurrentTargetPC(), NULL, true, FireSoundAmp);
			}
		}
	}
}

void AUTWeap_LightningRifle::PlayFiringSound(uint8 EffectFiringMode)
{
	if (bIsFullyPowered)
	{
		// Hack - skip here since played in SetFlashExtra FIXMESTEVE
	}
	else
	{
		Super::PlayFiringSound(EffectFiringMode);
	}
}

void AUTWeap_LightningRifle::OnRepCharging()
{
}

void AUTWeap_LightningRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UTOwner)
	{
		if (bIsCharging)
		{
			// FIXMESTEVE get charge value through timeline
			UTOwner->SetAmbientSound(ChargeSound, false);
			ChargePct = FMath::Min(1.f, ChargePct + ChargeSpeed*DeltaTime);
			UTOwner->ChangeAmbientSoundPitch(ChargeSound, ChargePct);
			bool bWasFullyPowered = bIsFullyPowered;
			bIsFullyPowered = (ChargePct >= 1.f);
			if (bIsFullyPowered && !bWasFullyPowered)
			{
				UUTGameplayStatics::UTPlaySound(GetWorld(), FullyPoweredSound, UTOwner, SRT_AllButOwner, false, FVector::ZeroVector, GetCurrentTargetPC(), NULL, true, FireSoundAmp);
			}
		}
		else
		{
			UTOwner->SetAmbientSound(ChargeSound, true);
		}
	}
}

void AUTWeap_LightningRifle::FireShot()
{
	if (UTOwner)
	{
		UTOwner->DeactivateSpawnProtection();
	}

	ConsumeAmmo(bIsFullyPowered ? 2 : 1);
	if (!FireShotOverride() && GetUTOwner() != NULL) // script event may kill user
	{
		if ((ZoomState == EZoomState::EZS_NotZoomed) && ProjClass.IsValidIndex(CurrentFireMode) && ProjClass[CurrentFireMode] != NULL)
		{
			FireProjectile();
		}
		else if (InstantHitInfo.IsValidIndex(CurrentFireMode) && InstantHitInfo[CurrentFireMode].DamageType != NULL)
		{
			if (InstantHitInfo[CurrentFireMode].ConeDotAngle > 0.0f)
			{
				FireCone();
			}
			else
			{
				FireInstantHit();
			}
		}
		//UE_LOG(UT, Warning, TEXT("FireShot"));
		PlayFiringEffects();
	}
	if (GetUTOwner() != NULL)
	{
		GetUTOwner()->InventoryEvent(InventoryEventName::FiredWeapon);
	}
	ChargePct = 0.f;
	FireZOffsetTime = 0.f;
}

//set fire interval appropriately

