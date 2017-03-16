// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUITypes.h"
#include "CommonActionHandlerInterface.h"

#include "CommonGlobalInputHandler.generated.h"

class UUserWidget;

struct FCommonGlobalInputHandlerData
{
	FDataTableRowHandle InputActionDataRow;
	TWeakObjectPtr<UUserWidget> UserWidget;
	FCommonInputActionHandlerData InputActionHandlerData;

	FCommonGlobalInputHandlerData(FDataTableRowHandle InputActionRow, const UUserWidget* UserWidgetPtr)
		: InputActionDataRow(InputActionRow), UserWidget(UserWidgetPtr)
	{
	}

	bool operator==(const FCommonGlobalInputHandlerData& Other) const
	{
		return InputActionDataRow == Other.InputActionDataRow && UserWidget.Get() == Other.UserWidget.Get();
	}
};

UCLASS()
class COMMONUI_API UCommonGlobalInputHandler: public UObject, public ICommonActionHandlerInterface
{
	GENERATED_UCLASS_BODY()
	/** Begin ICommonActionHandlerInterface */
	virtual bool HandleInput(FKey KeyPressed, EInputEvent EventType, bool& bPassThrough) override;
	virtual void UpdateCurrentlyHeldAction(float InDeltaTime) override;
	/** End ICommonActionHandlerInterface */
	
	/** Adds an input action handler for a specified widget 
	 *	@param UserWidget The widget that we're triggering an input action for. The priority of the widget is used to break ties.
	 *	@param InputActionRow The input action we're interested in listening for.
	 *	@param CommitedEvent The event that gets triggered if an action is complete.
	 */
	void RegisterInputAction(const UUserWidget* UserWidget, FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, 
		FCommonActionCompleteSingle CompleteEvent = FCommonActionCompleteSingle(), FCommonActionProgressSingle ProgressEvent = FCommonActionProgressSingle());
	
	/** Removes an input action handler for a specified widget
	 *	@param UserWidget The widget that we're triggering an input action for. The priority of the widget is used to break ties.
	 *	@param InputActionRow The input action we're interested in listening for.
	 *	@param CommitedEvent The event that gets triggered if an action is complete.
	 */
	void UnregisterInputAction(const UUserWidget* UserWidget, FDataTableRowHandle InputActionRow);
protected:

	/**
	 *	Takes a held and action and determines if a held action has started or ended.
	 *	If the action has started because a key was pressed this action handler panel will have
	 *	the common input manager track the input action handler and call the UpdateCurrentlyHeldAction
	 *  during tick.
	 */
	bool HandleHeldAction(const FCommonGlobalInputHandlerData& GlobalInputHandlerData, const FCommonInputActionData& InputActionData, EInputEvent EventType, bool& bPassThrough);
	
	/** Determines if the given input action is the currently held action when clearing an action */
	bool IsCurrentlyHeldAction(const TWeakObjectPtr<UUserWidget>& UserWidget, const FDataTableRowHandle& InputActionRow) const;
	
	/** Used for starting the tick of a held action if a key was pressed */
	void SetCurrentlyHeldAction(const TWeakObjectPtr<UUserWidget>& UserWidget, const FDataTableRowHandle& InputActionRow);
	
	/** Used for clearing a hold action if a key was released */
	void ClearCurrentlyHeldAction();
	
	/** Determines if there is visible widget that can be interacted with */
	bool IsWidgetVisible(TWeakObjectPtr<UUserWidget>& UserWidget);
private:
	float CurrentlyHeldActionHeldTime;
	FDataTableRowHandle CurrentlyHeldAction;
	TWeakObjectPtr<UUserWidget> CurrentlyHeldActionWidget;

	TArray<FCommonGlobalInputHandlerData> GlobalInputHandlersList;
};