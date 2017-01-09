// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UserWidget.h"
#include "AssetData.h"
#include "UTUMGWidget.generated.h"

class UUTLocalPlayer;

UCLASS()
class UNREALTOURNAMENT_API UUTUMGWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, category = UMG)
	FName WidgetTag;
	
	UPROPERTY()
	UUTLocalPlayer* UTPlayerOwner;

	/** Associated this UMG widget with a HUD. */
	virtual void AssociateLocalPlayer(UUTLocalPlayer* inLocalPlayer);

	/** Where in the viewport stack to sort this widget */
	UPROPERTY(EditDefaultsOnly, Category="Display")
	float DisplayZOrder;

	UFUNCTION(BlueprintCallable, category = UMG)
	UUTLocalPlayer* GetPlayerOwner();

	UFUNCTION(BlueprintCallable, category = UMG)
	virtual void CloseWidget();

	// Displays a partical system behind this widget at a given screen location
	UFUNCTION(BlueprintCallable, category = UMG)
	/**
	 * Displays a particle system in world.
	 * @param ScreenPosition - Where on the screen to display the effect
	 * @param bRelativeCoords - If true, ScreenPosition will be relative to the viewport
	 * @param LocationModifier - A vector that will be added to the final location in local space
	 * @param DirectionModifier - A rotator that will be added to the final direction in local space
	 **/
	void ShowParticalSystem(UParticleSystem* ParticalSystem, FVector2D ScreenLocation, bool bRelativeCoords = false, FVector LocationModifier = FVector(0.f,0.f,0.f), FRotator DirectionModifier = FRotator(0.f,0.f,0.f));

	// This event is called when the UMG widget is opened.  At this point, the PlayerOwner should be valid
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetOpened();
	virtual void WidgetOpened_Implementation()
	{
	}

	// This event is called when the widget is closed.  NOTE: none of the cached data is safe at this point
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetClosed();
	virtual void WidgetClosed_Implementation()
	{
	}





};
