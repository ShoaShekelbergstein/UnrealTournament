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
	void CloseWidget();
};
