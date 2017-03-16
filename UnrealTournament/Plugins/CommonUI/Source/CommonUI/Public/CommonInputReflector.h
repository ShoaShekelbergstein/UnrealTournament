// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CommonUITypes.h"

#include "CommonInputReflector.generated.h"

class UCommonActivatablePanel;

UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class UCommonInputReflector : public UCommonUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CommonInputReflector)
	TSubclassOf<UCommonButton> ButtonType;

	/** Let the layout have a chance to clear any buttons displayed */
	UFUNCTION(BlueprintImplementableEvent, Category = CommonInputReflector)
	void ClearButtons();

	/** Let the layout have a chance to add a new button that we're tracking as an active button */
	UFUNCTION(BlueprintImplementableEvent, Category = CommonInputReflector)
	void OnButtonAdded(UCommonButton* AddedButton);

protected:
	// UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// End UUserWidget

	void UpdateActiveButtonsToShow(UCommonActivatablePanel* PoppedPanel = nullptr);

	/** 
	 *	Any changes to the activatable panel stack in the input manager needs to be
	 *	tracked so that we can update the active buttons being displayed 
	 */
	virtual void HandleActivatablePanelPushed();
	virtual void HandleActivatablePanelPopped(UCommonActivatablePanel* PoppedPanel);

private:
	
	UPROPERTY()
	TArray<UCommonButton*> ActiveButtons;
	
	UPROPERTY()
	TArray<UCommonButton*> InactiveButtons;
};
