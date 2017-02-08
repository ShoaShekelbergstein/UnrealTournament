// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTWeap_Sniper.h"
#include "UTWeap_LightningRifle.generated.h"

UCLASS(abstract)
class UNREALTOURNAMENT_API AUTWeap_LightningRifle : public AUTWeap_Sniper
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(BlueprintReadWrite)
		float ChargePct;

	virtual void DrawWeaponCrosshair_Implementation(UUTHUDWidget* WeaponHudWidget, float RenderDelta) override;

};
