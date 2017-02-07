// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTDroppedAmmoBox.h"
#include "UTInventory.h"

#include "UTAmmoBoost.generated.h"

UCLASS()
class AUTAmmoBoost : public AUTInventory
{
	GENERATED_BODY()
public:
	AUTAmmoBoost(const FObjectInitializer& OI)
		: Super(OI)
	{
		NumBoxes = 6;
		AmmoRefillPct = 0.2f;
		TossSpeedXY = 1500.0f;
		TossSpeedZ = 750.0f;
	}

	/** number of ammo boxes to spawn */
	UPROPERTY(EditDefaultsOnly)
	int32 NumBoxes;
	/** refill percent per box */
	UPROPERTY(EditDefaultsOnly)
	float AmmoRefillPct;
	/** toss speed */
	UPROPERTY(EditDefaultsOnly)
	float TossSpeedXY;
	UPROPERTY(EditDefaultsOnly)
	float TossSpeedZ;

	virtual bool HandleGivenTo_Implementation(AUTCharacter* NewOwner) override
	{
		TSubclassOf<AUTDroppedAmmoBox> BoxType = *DroppedPickupClass;
		if (BoxType == nullptr)
		{
			BoxType = AUTDroppedAmmoBox::StaticClass();
		}
		FActorSpawnParameters Params;
		Params.Instigator = NewOwner;
		for (int32 i = 0; i < NumBoxes; i++)
		{
			const FRotator XYDir(0.0f, 360.0f / NumBoxes * i, 0.0f);
			AUTDroppedAmmoBox* Pickup = NewOwner->GetWorld()->SpawnActor<AUTDroppedAmmoBox>(BoxType, NewOwner->GetActorLocation(), XYDir, Params);
			if (Pickup != NULL)
			{
				Pickup->Movement->Velocity = XYDir.Vector() * TossSpeedXY + FVector(0.0f, 0.0f, TossSpeedZ);
				Pickup->GlobalRestorePct = AmmoRefillPct;
				Pickup->SetLifeSpan(30.0f);
			}
		}
		return true;
	}
};