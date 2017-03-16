// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonWidgetGroup.h"
#include "CommonButton.h"

#include "CommonButtonGroup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectedButtonChanged, class UCommonButton*, SelectedButton, int32, ButtonIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSelectionCleared );

/** 
 * Manages an arbitrary collection of CommonButton widgets.
 * Ensures that no more (and optionally, no less) than one button in the group is selected at a time
 */
UCLASS( BlueprintType )
class COMMONUI_API UCommonButtonGroup : public UCommonWidgetGroup
{
	GENERATED_BODY()
public:
	UCommonButtonGroup();

	virtual TSubclassOf<UWidget> GetWidgetType() const override { return UCommonButton::StaticClass(); }

	/** 
	 * Sets whether the group should always have a button selected.
	 * @param bRequireSelection True to force the group to always have a button selected.
	 * If true and nothing is selected, will select the first entry. If empty, will select the first button added.
	 */
	UFUNCTION(BlueprintCallable, Category = BaseButtonGroup)
	void SetSelectionRequired(bool bRequireSelection);

	/** Deselects all buttons in the group. */
	UFUNCTION(BlueprintCallable, Category = BaseButtonGroup)
	void DeselectAll();

	/** 
	 * Selects the next button in the group 
	 * @param bAllowWrap Whether to wrap to the first button if the last one is currently selected
	 */
	UFUNCTION(BlueprintCallable, Category = BaseButtonGroup)
	void SelectNextButton(bool bAllowWrap = true);

	/** 
	 * Selects the previous button in the group 
	 * @param bAllowWrap Whether to wrap to the first button if the last one is currently selected
	 */
	UFUNCTION(BlueprintCallable, Category = BaseButtonGroup)
	void SelectPreviousButton(bool bAllowWrap = true);

	/**
	 * Selects a button at a specific index in the group. Clears all selection if given an invalid index.
	 * @param ButtonIndex The index of the button in the group to select
	 */
	UFUNCTION(BlueprintCallable, Category = BaseButtonGroup)
	void SelectButtonAtIndex(int32 ButtonIndex);

	/**
	 * Get the index of the currently selected button, if any.
	 * @param The index of the currently selected button in the group, or -1 if there is no selected button.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = BaseButtonGroup)
	int32 GetSelectedButtonIndex();

	void ForEach(TFunctionRef<void(UCommonButton&, int32)> Functor);

	UPROPERTY(BlueprintAssignable)
	FOnSelectedButtonChanged OnSelectedButtonChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSelectionCleared OnSelectionCleared;

	int32 GetSelectedButtonIndex() const { return SelectedButtonIndex; }

protected:
	/** If true, the group will force that a button be selected at all times */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BaseButtonGroup, meta = (ExposeOnSpawn="true"))
	bool bSelectionRequired;

	virtual void OnWidgetAdded( UWidget* NewWidget ) override;
	virtual void OnWidgetRemoved( UWidget* OldWidget ) override;
	virtual void OnRemoveAll() override;

private:
	UFUNCTION()
	void OnSelectionStateChanged( UCommonButton* BaseButton, bool bIsSelected );

	void SelectNextButtonRecursive(int32 SelectionIndex, bool bAllowWrap);
	void SelectPrevButtonRecursive(int32 SelectionIndex, bool bAllowWrap);
	
	UCommonButton* GetButtonAtIndex(int32 Index);

	TArray<TWeakObjectPtr<UCommonButton>> Buttons;
	int32 SelectedButtonIndex;
};