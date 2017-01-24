// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTWeap_GrenadeLauncher.h"
#include "UTProj_Grenade_Sticky.h"
#include "UTLift.h"
#include "UTTeamDeco.h"

AUTProj_Grenade_Sticky::AUTProj_Grenade_Sticky(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LifeTime = 20.0f;
	MinimumLifeTime = 0.2f;
	bAlwaysShootable = true;
	CollisionComp->SetCollisionProfileName(TEXT("ProjectileShootable"));
}

void AUTProj_Grenade_Sticky::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority && !bFakeClientProjectile)
	{
		AUTCharacter* UTCharacter = Cast<AUTCharacter>(Instigator);
		if (UTCharacter)
		{ 
			GrenadeLauncherOwner = Cast<AUTWeap_GrenadeLauncher>(UTCharacter->GetWeapon());
			if (GrenadeLauncherOwner)
			{
				GrenadeLauncherOwner->RegisterStickyGrenade(this);
			}
		}

		GetWorldTimerManager().SetTimer(FLifeTimeHandle, this, &ThisClass::ExplodeDueToTimeout, LifeTime, false);
		GetWorldTimerManager().SetTimer(FArmedHandle, this, &ThisClass::ArmGrenade, MinimumLifeTime, false);
	}
}

void AUTProj_Grenade_Sticky::ArmGrenade()
{
	bArmed = true;
	InitialVisualOffset = FinalVisualOffset;
}

void AUTProj_Grenade_Sticky::ShutDown()
{
	Super::ShutDown();

	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().ClearTimer(FLifeTimeHandle);

		if (GrenadeLauncherOwner)
		{
			GrenadeLauncherOwner->UnregisterStickyGrenade(this);
		}
	}

	if (SavedFakeProjectile)
	{
		SavedFakeProjectile->ShutDown();
	}
}

void AUTProj_Grenade_Sticky::Destroyed()
{
	Super::Destroyed();

	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().ClearTimer(FLifeTimeHandle);

		if (GrenadeLauncherOwner)
		{
			GrenadeLauncherOwner->UnregisterStickyGrenade(this);
		}
	}
}

void AUTProj_Grenade_Sticky::ExplodeDueToTimeout()
{
	ArmGrenade();
	Explode(GetActorLocation(), FVector(0, 0, 0), nullptr);
}

void AUTProj_Grenade_Sticky::Explode_Implementation(const FVector& HitLocation, const FVector& HitNormal, UPrimitiveComponent* HitComp)
{
	if (bArmed || Role != ROLE_Authority || bTearOff)
	{
		// If we still have a fake projectile, AUTProjectile::Explode may skip it
		SavedFakeProjectile = MyFakeProjectile;
		MyFakeProjectile = nullptr;
		Super::Explode_Implementation(HitLocation, HitNormal, HitComp);
		MyFakeProjectile = SavedFakeProjectile;
	}
}

uint8 AUTProj_Grenade_Sticky::GetInstigatorTeamNum()
{
	AController* Controller = GetInstigatorController();
	
	const IUTTeamInterface* TeamInterfaceController = Cast<IUTTeamInterface>(Controller);
	if (TeamInterfaceController != NULL)
	{
		return TeamInterfaceController->GetTeamNum();
	}

	APawn* Pawn = GetInstigator();
	const IUTTeamInterface* TeamInterfacePawn = Cast<IUTTeamInterface>(Pawn);
	if (TeamInterfacePawn != NULL)
	{
		return TeamInterfacePawn->GetTeamNum();
	}

	return 0;
}

float AUTProj_Grenade_Sticky::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float NewDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (NewDamageAmount > 0.0f && !bExploded)
	{
		if (bArmed)
		{
			AUTPlayerController* UTPC = Cast<AUTPlayerController>(EventInstigator);
			if (UTPC)
			{
				uint8 DamagerTeamNum = UTPC->GetTeamNum();
				if (DamagerTeamNum != GetInstigatorTeamNum())
				{
					PlayDamagedDetonationEffects();
					InstigatorController = UTPC;
					Explode(GetActorLocation(), FVector(0,0,1), nullptr);
				}
			}
		}
	}

	return NewDamageAmount;
}

void AUTProj_Grenade_Sticky::OnStop(const FHitResult& Hit)
{
	Super::OnStop(Hit);

	ArmGrenade();

	PlayIdleEffects();

	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE_SHOOTABLE);

	if (Hit.Actor.IsValid() && Cast<AUTLift>(Hit.Actor.Get()) == nullptr)
	{
		AttachToComponent(Hit.Actor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AUTProj_Grenade_Sticky::ProcessHit_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FVector& HitLocation, const FVector& HitNormal)
{
	// If we're hitting a team wall, just ignore it
	AUTTeamDeco* TeamDeco = Cast<AUTTeamDeco>(OtherActor);
	if (TeamDeco)
	{
		if (!TeamDeco->bBlockTeamProjectiles && TeamDeco->GetTeamNum() == GetInstigatorTeamNum())
		{
			return;
		}
	}
	
	// If we hit a teammate that isn't ourselves, just ignore it
	AUTCharacter* UTChar = Cast<AUTCharacter>(OtherActor);
	if (UTChar)
	{
		if (GetInstigatorTeamNum() == UTChar->GetTeamNum() && GetInstigatorController() != UTChar->GetController())
		{
			return;
		}
	}

	// Ignore projectile hits
	AUTProjectile* UTProj = Cast<AUTProjectile>(OtherActor);
	if (UTProj)
	{
		return;
	}

	// Ignore volume hits
	AVolume* Volume = Cast<AVolume>(OtherActor);
	if (Volume)
	{
		return;
	}

	ProjectileMovement->Velocity = FVector::ZeroVector;

	ArmGrenade();

	Super::ProcessHit_Implementation(OtherActor, OtherComp, HitLocation, HitNormal);

	PlayIdleEffects();
}