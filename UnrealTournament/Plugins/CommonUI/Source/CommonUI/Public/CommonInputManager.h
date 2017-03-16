// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UserWidget.h"
#include "CommonUITypes.h"
#include "CommonActionHandlerInterface.h"
#include "Tickable.h"
#include "CommonInputManager.generated.h"

class UCommonActivatablePanel;
class UCommonGlobalInputHandler;

DECLARE_MULTICAST_DELEGATE_OneParam(FCommonInputActivatablePopped, UCommonActivatablePanel* /* ActivatablePanel */);
DECLARE_MULTICAST_DELEGATE(FCommonInputActivatablePanelPushed);

UCLASS()
class COMMONUI_API UCommonInputManager : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Begin UObject */
	virtual void PostInitProperties() override;
	/** End UObject */

	/** Registers an action handler panel with the manager */
	UFUNCTION(BlueprintCallable, Category = CommonInputManager)
	void PushActivatablePanel(UCommonActivatablePanel* ActivatablePanel);

	/** Unregisters an action handler panel from the manager */
	UFUNCTION(BlueprintCallable, Category = CommonInputManager)
	void PopActivatablePanel(UCommonActivatablePanel* ActivatablePanel);

	/** Start listening for an existing held action on an activatable panel */	
	UFUNCTION(BlueprintCallable, Category = CommonInputManager)
	bool StartListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent);

	/** Stop listening for an existing held action on an activatable panel */
	UFUNCTION(BlueprintCallable, Category = CommonInputManager)
	bool StopListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent);

	/** Handles input rerouting from the game view port */
	void HandleRerouteInput(int32 ControllerId, FKey Key, EInputEvent EventType, FReply& Reply);
	
	/** Gets the list of actions that can be triggered */
	UFUNCTION(BlueprintCallable, Category = CommonInputManager)
	bool GetAvailableInputActions(TArray<FDataTableRowHandle>& AvailableInputActions);

	/** Short circuit the input handling process and directly trigger an input action */
	void TriggerInputActionHandler(const FDataTableRowHandle& InputActionDataRow);
	
	/** Gets/Sets a held input action handler */
	TScriptInterface<ICommonActionHandlerInterface> GetHeldInputActionHandler() { return CurrentlyHeldActionInputHandler; }
	void SetHeldInputActionHandler(TScriptInterface<ICommonActionHandlerInterface> InActionHandler)
	{
		CurrentlyHeldActionInputHandler = InActionHandler;
	}

	/** Begin FTickableGameObject */
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return !HasAnyFlags(RF_ClassDefaultObject); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UFortUITeamInfo, STATGROUP_Tickables); }
	/** End FTickableGameObject */

	/** Delegates to listen for changes to the activatable panel stack */
	FCommonInputActivatablePopped OnActivatablePanelPopped;
	FCommonInputActivatablePanelPushed OnActivatablePanelPushed;

	/** Access to global input handler */
	UCommonGlobalInputHandler* GetGlobalInputHandler() { return GlobalInputHandler; }

private:
	UFUNCTION()
	void OnPanelActivated(UCommonActivatablePanel* ActivatedPanel);
	UFUNCTION()
	void OnPanelDeactivated(UCommonActivatablePanel* DeactivatedPanel);

	UPROPERTY()
	TScriptInterface<ICommonActionHandlerInterface> CurrentlyHeldActionInputHandler;

	UPROPERTY()
	TArray<UCommonActivatablePanel*> ActivatablePanelStack;

	/** The global input handler for other user widgets such as buttons, tab lists, etc.*/
	UPROPERTY()
	UCommonGlobalInputHandler* GlobalInputHandler;

	bool bPendingActivatablePanelOperation;
};