// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UnrealNetwork.h"
#include "StatNames.h"
#include "UTWeap_LightningRifle.h"

AUTWeap_LightningRifle::AUTWeap_LightningRifle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AUTWeap_LightningRifle::DrawWeaponCrosshair_Implementation(UUTHUDWidget* WeaponHudWidget, float RenderDelta)
{
	Super::DrawWeaponCrosshair_Implementation(WeaponHudWidget, RenderDelta);

	if ((ChargePct > 0.f) && WeaponHudWidget && WeaponHudWidget->UTHUDOwner)
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
