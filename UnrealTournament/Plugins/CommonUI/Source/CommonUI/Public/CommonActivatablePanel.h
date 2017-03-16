// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "CommonUITypes.h"
#include "CommonUserWidget.h"
#include "CommonInputManager.h"
#include "CommonActionHandlerInterface.h"

#include "CommonActivatablePanel.generated.h"

/**
* Activatable Panel Delegates
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetActivationChangedDynamic, class UCommonActivatablePanel*, Panel);

UCLASS(BlueprintType, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonActivatablePanel : public UCommonUserWidget, public ICommonActionHandlerInterface
{
	GENERATED_UCLASS_BODY()

public:
	// UUserWidget interface
	virtual void NativeDestruct() override;
	// End UUserWidget interface

	/** True if this widget is currently activated */
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	bool IsActivated() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void BeginActivate(UCommonInputManager* InCommonInputManager);

	/**
	 *	Used to do custom delayed activation
	 *	NOTE: This function is meant to only be overridden and not have the parent called. The base implementation
	 *	simply calls EndActivate.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ActivatablePanel")
	void OnBeginActivate();
	
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void EndActivate();
		
	/**
	 *	Used to do custom delayed deactivation
	 *	NOTE: This function is meant to only be overridden and not have the parent called. The base implementation
	 *	simply calls EndDeactivate.
	 */	
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void BeginDeactivate();

	/** Tells the panel to stop listening to input if we're currently active and lets others who are interested know via delegates */
	UFUNCTION(BlueprintNativeEvent, Category = ActivatablePanel)
	void OnBeginDeactivate();

	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void EndDeactivate();

	/** Removes a panel off of its associated stack */
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void PopPanel();
	
	/** Fires when the widget is activated. */
	UPROPERTY(BlueprintAssignable)
	FOnWidgetActivationChangedDynamic OnWidgetActivated;
	
	/** Fires when the widget is deactivated. */
	UPROPERTY(BlueprintAssignable)
	FOnWidgetActivationChangedDynamic OnWidgetDeactivated;
	
	/**
	 * Gets the list of input action rows for visualization of input actions that this activatable panel 
	 * Handles.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	bool GetInputActions(TArray<FDataTableRowHandle>& InputActionDataRows) const;

	/**
	 * Trigger an input action handler based on a given input action data row
	 */
	bool TriggerInputActionHandler(const FDataTableRowHandle& InputActionDataRow);

	/**
	 *	Start/Stop Listening for an input action's progress
	 */
	bool StartListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent);
	bool StopListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent);
	
	/** Set an input action handler up for a press action */	
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void SetInputActionHandler(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent);

	/** Set an input action handler up for a hold action */
	UFUNCTION(BlueprintCallable, Category = ActivatablePanel)
	void SetInputActionHandlerWithProgress(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent);
	
	/** Begin UCommonActionHandlerInterface */
	virtual bool HandleInput(FKey KeyPressed, EInputEvent EventType, bool& bPassThrough) override;
	void UpdateCurrentlyHeldAction(float InDeltaTime) override;
	/** End UCommonActionHandlerInterface */

	/**
	 *	Let's the input handling go past the stack and let's the global input handler have a chance
	 *	to handle the input.
	 */
	bool ShouldBypassStack();

protected:
	/** Allows the BP to take action when activated */
	UFUNCTION(BlueprintImplementableEvent, Category = ActivatablePanel)
	void OnActivated();
	virtual void NativeOnActivated();

	/** Allows the BP to take action when deactivated */
	UFUNCTION(BlueprintImplementableEvent, Category = ActivatablePanel)
	void OnDeactivated();
	virtual void NativeOnDeactivated();

	/**
	 *	Takes a held and action and determines if a held action has started or ended.
	 *	If the action has started because a key was pressed this action handler panel will have
	 *	the common input manager track the input action handler and call the UpdateCurrentlyHeldAction 
	 *  during tick.
	 */
	bool HandleHeldAction(const FDataTableRowHandle& InputActionRow, const FCommonInputActionHandlerData& InputActionHandler, const FCommonInputActionData& InputActionData, EInputEvent EventType, bool& bPassThrough);
	
	/** Determines if the given input action is the currently held action when clearing an action */
	bool IsCurrentlyHeldAction(const FDataTableRowHandle& InputActionRow) const;
	
	/** Used for starting the tick of a held action if a key was pressed */
	void SetCurrentlyHeldAction(const FDataTableRowHandle& InputActionRow);

	/** Used for clearing a hold action if a key was released */
	void ClearCurrentlyHeldAction();
	
	/** Prevents unhandled actions from going further down the stack of activatable panels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ActivatablePanel)
	bool bConsumeAllActions;

	/** Whether or not actions can be exposed by a reflector widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ActivatablePanel)
	bool bExposeActionsExternally;

	/**
	 *	Let's the input handling go past the stack and let's the global input handler have a chance
	 *	to handle the input. NOTE: Should not be combined with input actions registered using
	 *	SetInputActionHandler or SetInputActionHandlerWithProgress.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ActivatablePanel)
	bool bShouldBypassStack;

private:
	/** The primary way to enable and disable action handling on this activatable panel */
	bool bIsActive;
	
	FDataTableRowHandle CurrentlyHeldAction;

	/** The time spent when the input key was held */
	float CurrentlyHeldActionHeldTime;
	struct FInputActionRowFuncs : public TDefaultMapKeyFuncs<FDataTableRowHandle, FCommonInputActionHandlerData, false>
	{
		static FORCEINLINE uint32 GetKeyHash(const FDataTableRowHandle& InputActionRow);
	};

	typedef TMap<FDataTableRowHandle, FCommonInputActionHandlerData, FDefaultSetAllocator, FInputActionRowFuncs> InputActionHandlerMap;

	InputActionHandlerMap InputActionHandlers;
	TWeakObjectPtr<UCommonInputManager> CommonInputManager;
};