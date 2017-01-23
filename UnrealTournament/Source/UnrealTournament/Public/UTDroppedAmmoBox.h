// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTDroppedPickup.h"
#include "UTPickupAmmo.h"
#include "UTPickupMessage.h"

#include "UTDroppedAmmoBox.generated.h"

UCLASS()
class AUTDroppedAmmoBox : public AUTDroppedPickup
{
	GENERATED_BODY()
public:
	AUTDroppedAmmoBox(const FObjectInitializer& OI)
		: Super(OI)
	{
		SMComp = OI.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(TEXT("SMComp")));
		SMComp->SetupAttachment(Collision);
		static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMesh(TEXT("/Game/RestrictedAssets/Proto/UT3_Pickups/Ammo/S_AmmoCrate.S_AmmoCrate"));
		SMComp->SetStaticMesh(BoxMesh.Object);
		SMComp->RelativeLocation = FVector(0.0f, 0.0f, -30.0f);
		SMComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundRef(TEXT("/Game/RestrictedAssets/Proto/UT3_Pickups/Audio/Ammo/Cue/A_Pickup_Ammo_Stinger_Cue.A_Pickup_Ammo_Stinger_Cue"));
		PickupSound = PickupSoundRef.Object;
	}
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* SMComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundBase* PickupSound;

	/** ammo in the box */
	UPROPERTY(BlueprintReadWrite)
	TArray<FStoredAmmo> Ammo;

	/** if set, restore an additional percentage of all owned weapons' ammo */
	UPROPERTY(BlueprintReadWrite)
	float GlobalRestorePct;

	virtual USoundBase* GetPickupSound_Implementation() const
	{
		return PickupSound;
	}

	void GiveTo_Implementation(APawn* Target) override
	{
		AUTCharacter* UTC = Cast<AUTCharacter>(Target);
		if (UTC != NULL)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(UTC->GetController());
			TArray<UClass*> AmmoClasses;
			GetDerivedClasses(AUTPickupAmmo::StaticClass(), AmmoClasses);
			TArray<FStoredAmmo> FinalAmmoAmounts = Ammo;
			if (GlobalRestorePct > 0.0f)
			{
				for (UClass* AmmoType : AmmoClasses)
				{
					AUTWeapon* Weap = UTC->FindInventoryType(AmmoType->GetDefaultObject<AUTPickupAmmo>()->Ammo.Type, true);
					if (Weap != nullptr)
					{
						bool bFound = false;
						for (FStoredAmmo& AmmoItem : FinalAmmoAmounts)
						{
							if (AmmoItem.Type == Weap->GetClass())
							{
								AmmoItem.Amount += Weap->MaxAmmo * GlobalRestorePct;
								bFound = true;
								break;
							}
						}
						if (!bFound)
						{
							int32 Index = FinalAmmoAmounts.AddZeroed();
							FinalAmmoAmounts[Index].Type = Weap->GetClass();
							FinalAmmoAmounts[Index].Amount = Weap->MaxAmmo * GlobalRestorePct;
						}
					}
				}
			}
			for (const FStoredAmmo& AmmoItem : FinalAmmoAmounts)
			{
				UTC->AddAmmo(AmmoItem);
				if (PC != NULL)
				{
					// send message per ammo type
					UClass** AmmoType = AmmoClasses.FindByPredicate([&](UClass* TestClass) { return AmmoItem.Type == TestClass->GetDefaultObject<AUTPickupAmmo>()->Ammo.Type; });
					if (AmmoType != NULL)
					{
						PC->ClientReceiveLocalizedMessage(UUTPickupMessage::StaticClass(), 0, NULL, NULL, *AmmoType);
					}
				}
			}
		}
	}
};