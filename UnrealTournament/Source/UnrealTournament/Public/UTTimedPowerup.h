// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTTimedPowerup.generated.h"

UCLASS(Blueprintable, Abstract)
class AUTTimedPowerup : public AUTInventory
{
	GENERATED_UCLASS_BODY()

	/** time remaining (in defaults, initial lifetime on pickup)
	 * note: only ticks while held (not dropped on the ground)
	 */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = Powerup)
	float TimeRemaining;

	/** Rate powerup time remaining ticks down when powerup has been dropped */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Powerup)
	float DroppedTickRate;

	/** sound played on an interval for the last few seconds to indicate time is running out */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Effects)
	USoundBase* PowerupFadingSound;

	/** sound played on the current owner when the duration runs out */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Effects)
	USoundBase* PowerupOverSound;

	/** overlay material added to the player's weapon while this powerup is in effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Effects)
	UMaterialInterface* OverlayMaterial;

	/** icon for drawing time remaining on the HUD */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = HUD)
	FCanvasIcon HUDIcon;

	UPROPERTY(BlueprintReadOnly, Category = Powerup)
	class AUTDroppedPickup* MyPickup;

	virtual void AddOverlayMaterials_Implementation(AUTGameState* GS) const override
	{
		GS->AddOverlayMaterial(OverlayMaterial);
	}

	UFUNCTION(Reliable, Client)
	void ClientSetTimeRemaining(float InTimeRemaining);

	UFUNCTION()
	virtual void PlayFadingSound();

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void TimeExpired();

	virtual void Tick(float DeltaTime);

	virtual void Destroyed() override;

	virtual bool StackPickup_Implementation(AUTInventory* ContainedInv);

	virtual void GivenTo(AUTCharacter* NewOwner, bool bAutoActivate) override;
	virtual void Removed() override;
	virtual void InitializeDroppedPickup(AUTDroppedPickup* Pickup) override;

	virtual void DrawInventoryHUD_Implementation(UUTHUDWidget* Widget, FVector2D Pos, FVector2D Size) override;

	virtual float DetourWeight_Implementation(APawn* Asker, AActor* Pickup, float PathDistance) const
	{
		return BotDesireability(Asker, Pickup, PathDistance);
	}
};