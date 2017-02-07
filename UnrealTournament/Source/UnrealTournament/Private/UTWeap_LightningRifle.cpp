// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UnrealNetwork.h"
#include "StatNames.h"
#include "UTWeap_LightningRifle.h"

AUTWeap_LightningRifle::AUTWeap_LightningRifle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultGroup = 9;
	Group = 9;
/*	WeaponCustomizationTag = EpicWeaponCustomizationTags::GrenadeLauncher;

	DetonationAfterFireDelay = 0.3f;

	ShotsStatsName = NAME_BioLauncherShots;
	HitsStatsName = NAME_BioLauncherHits;
	HighlightText = NSLOCTEXT("Weapon", "GrenadeHighlightText", "Hot Potato");*/
}