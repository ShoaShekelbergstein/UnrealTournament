// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CommonUITypes.h"
#include "Components/Button.h"
#include "CommonButton.generated.h"

class UCommonTextStyle;
class SBox;

/* ---- All properties must be EditDefaultsOnly, BlueprintReadOnly !!! -----
 *       we return the CDO to blueprints, so we cannot allow any changes (blueprint doesn't support const variables)
 */
UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class UCommonButtonStyle : public UObject
{
	GENERATED_BODY()

public:
	
	/** Whether or not the style uses a drop shadow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	bool bSingleMaterial;

	/** The normal (un-selected) brush to apply to each size of this button */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", meta = (EditCondition = "bSingleMaterial"))
	FSlateBrush Material;

	/** The normal (un-selected) brush to apply to each size of this button */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Normal", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush NormalBase;

	/** The normal (un-selected) brush to apply to each size of this button when hovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Normal", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush NormalHovered;

	/** The normal (un-selected) brush to apply to each size of this button when pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Normal", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush NormalPressed;

	/** The selected brush to apply to each size of this button */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selected", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush SelectedBase;

	/** The selected brush to apply to each size of this button when hovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selected", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush SelectedHovered;

	/** The selected brush to apply to each size of this button when pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selected", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush SelectedPressed;

	/** The disabled brush to apply to each size of this button */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Disabled", meta = (EditCondition = "!bSingleMaterial"))
	FSlateBrush Disabled;

	/** The button content padding to apply for each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	FMargin ButtonPadding;
	
	/** The custom padding of the button to use for each size */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	FMargin CustomPadding;

	/** The minimum width of buttons using this style */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	int32 MinWidth;

	/** The minimum height of buttons using this style */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	int32 MinHeight;

	/** The text style to use when un-selected */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TSubclassOf<UCommonTextStyle> NormalTextStyle;

	/** The text style to use when selected */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TSubclassOf<UCommonTextStyle> SelectedTextStyle;

	/** The text style to use when disabled */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TSubclassOf<UCommonTextStyle> DisabledTextStyle;

	/** The sound to play when the button is pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", meta = (DisplayName = "Pressed Sound"))
	FSlateSound PressedSlateSound;

	/** The sound to play when the button is hovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties", meta = (DisplayName = "Hovered Sound"))
	FSlateSound HoveredSlateSound;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetMaterialBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetNormalBaseBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetNormalHoveredBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetNormalPressedBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetSelectedBaseBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetSelectedHoveredBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetSelectedPressedBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetDisabledBrush(FSlateBrush& Brush) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetButtonPadding(FMargin& OutButtonPadding) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	void GetCustomPadding(FMargin& OutCustomPadding) const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	UCommonTextStyle* GetNormalTextStyle() const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	UCommonTextStyle* GetSelectedTextStyle() const;

	UFUNCTION(BlueprintCallable, Category = "Common ButtonStyle|Getters")
	UCommonTextStyle* GetDisabledTextStyle() const;

};

/** Custom UButton override that allows us to disable clicking without disabling the widget entirely */
UCLASS(Experimental)	// "Experimental" to hide it in the designer
class UCommonButtonInternal : public UButton
{
	GENERATED_UCLASS_BODY()

public:
	void SetInteractionEnabled(bool bInIsInteractionEnabled);
	bool IsHovered() const;
	bool IsPressed() const;

	void SetMinDesiredHeight(int32 InMinHeight);
	void SetMinDesiredWidth(int32 InMinWidth);

protected:
	/** The minimum width of the button */
	UPROPERTY()
	int32 MinWidth;

	/** The minimum height of the button */
	UPROPERTY()
	int32 MinHeight;

	/** If true, this button can be interacted with it normally. Otherwise, it will not react to being hovered or clicked. */
	UPROPERTY()
	bool bInteractionEnabled;
	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** Cached pointer to the underlying slate button owned by this UWidget */
	TSharedPtr<SBox> MyBox;

	/** Cached pointer to the underlying slate button owned by this UWidget */
	TSharedPtr<class SCommonButton> MyCommonButton;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCommonSelectedStateChanged, class UCommonButton*, Button, bool, Selected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonButtonClicked, class UCommonButton*, Button);

UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonButton : public UCommonUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** Called when the Selected state of this button changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCommonSelectedStateChanged OnSelectedChanged;

	/** Called when the Button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCommonButtonClicked OnButtonClicked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCommonButtonClicked OnButtonHovered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCommonButtonClicked OnButtonUnhovered;

public:

	// UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool Initialize() override;
	virtual void SetIsEnabled(bool bInIsEnabled) override;
	virtual bool NativeIsInteractable() const override;
	// End of UUserWidget interface
	
	/** Enables this button (use instead of SetIsEnabled) */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void EnableButton();

	/** Disables this button (use instead of SetIsEnabled) */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void DisableButton();
	
	/** Disables this button with a reason (use instead of SetIsEnabled) */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void DisableButtonWithReason(const FText& DisabledReason);

	/** Is this button currently interactable? (use instead of GetIsEnabled) */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	bool IsInteractionEnabled() const;

	/** Is this button currently hovered? */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	bool IsHovered() const;

	/** Is this button currently pressed? */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	bool IsPressed() const;

	/** Change whether this widget is selectable at all. If false and currently selected, will deselect. */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetIsSelectable(bool bInIsSelectable);

	/** Change whether this widget is toggleable. If toggleable, clicking when selected will deselect. */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetIsToggleable(bool bInIsToggleable);

	/** 
	 * Change the selected state manually.
	 * @param bGiveClickFeedback	If true, the button may give user feedback as if it were clicked. IE: Play a click sound, trigger animations as if it were clicked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetIsSelected(bool InSelected, bool bGiveClickFeedback = true);

	/** @returns True if the button is currently in a selected state, False otherwise */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	bool GetSelected() const;

	UFUNCTION( BlueprintCallable, Category = "Common Button" )
	void ClearSelection();

	/** Sets the style of this button, rebuilds the internal styling */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetStyle(TSubclassOf<UCommonButtonStyle> InStyle = nullptr);

	/** @Returns Current button style*/
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	UCommonButtonStyle* GetStyle() const;

	/** @return The current button padding that corresponds to the current size and selection state */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	void GetCurrentButtonPadding(FMargin& OutButtonPadding) const;

	/** @return The custom padding that corresponds to the current size and selection state */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	void GetCurrentCustomPadding(FMargin& OutCustomPadding) const;
	
	/** @return The text style that corresponds to the current size and selection state */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	UCommonTextStyle* GetCurrentTextStyle() const;

	/** @return The class of the text style that corresponds to the current size and selection state */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	TSubclassOf<UCommonTextStyle> GetCurrentTextStyleClass() const;

	/** Sets the minimum dimensions of this button */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetMinDimensions(int32 InMinWidth, int32 InMinHeight);

	/** Updates the current triggered action */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Setters")
	void SetTriggeredInputAction(const FDataTableRowHandle &InputActionRow, class UCommonActivatablePanel* OldPanel);

	/** Gets the appropriate input action that is set */
	UFUNCTION(BlueprintCallable, Category = "Common Button|Getters")
	bool GetInputAction(FDataTableRowHandle &InputActionRow) const;
	
protected:
	virtual void SynchronizeProperties() override;

	/** Associates this button at its priority with the given key */
	virtual void BindTriggeringInputActionToClick();

	/** Associates this button at its priority with the given key */
	void UnbindTriggeringInputActionToClick();

	UFUNCTION()
	virtual void HandleTriggeringActionCommited(bool& bPassthrough);

	virtual void ExecuteTriggeredInput();

	/** Handler function registered to the underlying buttons click. */
	UFUNCTION()
	void HandleButtonClicked();

	/** Called when clicked before the blueprint event is fired, for use by descendant classes */
	
	UFUNCTION(BlueprintImplementableEvent, Category="Common Button")
	void OnSelected();
	virtual void NativeOnSelected(bool bBroadcast);

	UFUNCTION(BlueprintImplementableEvent, Category="Common Button")
	void OnDeselected();
	virtual void NativeOnDeselected(bool bBroadcast);

	UFUNCTION(BlueprintImplementableEvent, Category="Common Button")
	void OnHovered();
	virtual void NativeOnHovered();

	UFUNCTION(BlueprintImplementableEvent, Category="Common Button")
	void OnUnhovered();
	virtual void NativeOnUnhovered();

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnClicked();
	virtual void NativeOnClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnEnabled();
	virtual void NativeOnEnabled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnDisabled();
	virtual void NativeOnDisabled();

	/** Allows derived classes to take action when the current text style has changed */
	UFUNCTION(BlueprintImplementableEvent, meta=(BlueprintProtected="true"), Category = "Common Button")
	void OnCurrentTextStyleChanged();
	virtual void NativeOnCurrentTextStyleChanged();

	/** Internal method to allow the selected state to be set regardless of selectability or toggleability */
	UFUNCTION(BlueprintCallable, meta=(BlueprintProtected="true"), Category = "Common Button")
	void SetSelectedInternal(bool bInSelected, bool bAllowSound = true, bool bBroadcast = true);

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnTriggeredInputActionChanged(const FDataTableRowHandle& NewTriggeredAction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnActionProgress(float HeldPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void OnActionComplete();

protected:
	void RefreshDimensions();
	virtual void NativeOnMouseEnter( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual void NativeOnMouseLeave( const FPointerEvent& InMouseEvent ) override;

	/** The minimum width of the button (only used if greater than the style's minimum) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Layout, meta = (ClampMin = "0"))
	int32 MinWidth;

	/** The minimum height of the button (only used if greater than the style's minimum) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Layout, meta = (ClampMin = "0"))
	int32 MinHeight;

	/** References the button style asset that defines a style in multiple sizes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	TSubclassOf<UCommonButtonStyle> Style;

	/** The type of mouse action required by the user to trigger the button's 'Click' */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Style, meta = (ExposeOnSpawn = true))
	bool bApplyAlphaOnDisable;

	/** Optional override for the sound to play when this button is pressed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound, meta = (DisplayName = "Pressed Sound Override"))
	FSlateSound PressedSlateSoundOverride;

	/** Optional override for the sound to play when this button is hovered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound, meta = (DisplayName = "Hovered Sound Override"))
	FSlateSound HoveredSlateSoundOverride;
	
	/** True if the button supports being in a "selected" state, which will update the style accordingly */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Selection, meta = (ExposeOnSpawn = true))
	bool bSelectable;

	/** True if the button can be deselected by clicking it when selected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Selection, meta = (ExposeOnSpawn = true, EditCondition = "bSelectable"))
	bool bToggleable;

	/** The type of mouse action required by the user to trigger the button's 'Click' */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Selection, meta = (ExposeOnSpawn = true))
	TEnumAsByte<EButtonClickMethod::Type> ClickMethod;

	/** 
	 *	The input action that is bound to this button. The common input manager will trigger this button to 
	 *	click if the action was pressed 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (ExposeOnSpawn = true, RowType = CommonInputActionData))
	FDataTableRowHandle TriggeringInputAction;

	/**
	 *	The input action that can be visualized as well as triggered when the user
	 *	clicks the button.
	 */
	FDataTableRowHandle TriggeredInputAction;

private:
	friend class UCommonButtonGroup;
	friend class UPageView;

	const UCommonButtonStyle* GetStyleCDO() const;
	void BuildStyles();
	void SetButtonStyle();
	
	FText EnabledTooltipText;
	FText DisabledTooltipText;

	/** Internally managed and applied style to use when not selected */
	FButtonStyle NormalStyle;
	
	/** Internally managed and applied style to use when selected */
	FButtonStyle SelectedStyle;

	/** True if this button is currently selected */
	bool bSelected;

	/** True if interaction with this button is currently enabled */
	bool bInteractionEnabled;

	/**
	 * The actual UButton that we wrap this user widget into. 
	 * Allows us to get user widget customization and built-in button functionality. 
	 */
	TWeakObjectPtr<class UCommonButtonInternal> RootButton;
};