// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTBotCharacter.h"

UUTBotCharacter::UUTBotCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxRunSpeed = 940.f;
	MultiJumpImpulse = 600.f;
	MaxMultiJumpCount = 0;
	bCanDodge = true;
	AirControl = 0.55f;
	DodgeResetInterval = 0.35f;
	JumpImpulse = 730.f;
	ExtraArmor = 0;
	MaxWallRunFallZ = -120.f;
	WallRunGravityScaling = 0.08f;
	MaxAdditiveDodgeJumpSpeed = 700.f;
	DodgeImpulseHorizontal = 1500.f;
	DodgeImpulseVertical = 500.f;
	DodgeMaxHorizontalVelocity = 1700.f;
}