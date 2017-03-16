// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonInputManager.h"

#include "CommonActivatablePanel.h"
#include "CommonGlobalInputHandler.h"

UCommonInputManager::UCommonInputManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, GlobalInputHandler(nullptr)
	, bPendingActivatablePanelOperation(false)
{
}

void UCommonInputManager::PostInitProperties()
{
	Super::PostInitProperties();

	GlobalInputHandler = NewObject<UCommonGlobalInputHandler>(this);
}

void UCommonInputManager::HandleRerouteInput(int32 ControllerId, FKey Key, EInputEvent EventType, FReply& Reply)
{
	Reply = FReply::Unhandled();
	bool bPassThrough = false;

	if (!bPendingActivatablePanelOperation)
	{
		for (int32 Index = ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
		{
			UCommonActivatablePanel* ActivatablePanel = ActivatablePanelStack[Index];
			if (ActivatablePanel)
			{
				if (ActivatablePanel->HandleInput(Key, EventType, bPassThrough))
				{
					Reply = bPassThrough ? FReply::Unhandled() : FReply::Handled();
					return;
				}
				else if (ActivatablePanel->ShouldBypassStack())
				{
					break;
				}
			}
		}

		/**
		*	If after going through the stack of action handler panels, we still haven't handled the input,
		*	we will give the global input handler a chance at handling the input event
		*/
		if (GlobalInputHandler->HandleInput(Key, EventType, bPassThrough))
		{
			Reply = bPassThrough ? FReply::Unhandled() : FReply::Handled();
		}
	}
}

void UCommonInputManager::PushActivatablePanel(UCommonActivatablePanel* ActivatablePanel)
{
	if (ensure(ActivatablePanel))
	{
		if (!ActivatablePanelStack.Contains(ActivatablePanel))
		{
			bPendingActivatablePanelOperation = true;
			ActivatablePanel->OnWidgetActivated.AddDynamic(this, &ThisClass::OnPanelActivated);
			ActivatablePanelStack.AddUnique(ActivatablePanel);
			ActivatablePanel->BeginActivate(this);
			OnActivatablePanelPushed.Broadcast();
		}
	}
}

void UCommonInputManager::OnPanelActivated(UCommonActivatablePanel* ActivatedPanel)
{
	if (ensure(ActivatedPanel))
	{
		if (ensure(ActivatablePanelStack.Contains(ActivatedPanel)))
		{
			bPendingActivatablePanelOperation = false;
			ActivatedPanel->OnWidgetActivated.RemoveAll(this);
		}
	}
}


void UCommonInputManager::PopActivatablePanel(UCommonActivatablePanel* ActivatablePanel)
{
	if (ensure(ActivatablePanel))
	{
		if (ActivatablePanelStack.Contains(ActivatablePanel))
		{
			bPendingActivatablePanelOperation = true;
			ActivatablePanel->OnWidgetDeactivated.AddDynamic(this, &ThisClass::OnPanelDeactivated);
			ActivatablePanel->BeginDeactivate();
		}
	}
}

void UCommonInputManager::OnPanelDeactivated(UCommonActivatablePanel* DeactivatedPanel)
{
	if (ensure(DeactivatedPanel))
	{
		if (ActivatablePanelStack.Contains(DeactivatedPanel))
		{
			bPendingActivatablePanelOperation = false;
			DeactivatedPanel->OnWidgetDeactivated.RemoveAll(this);
			ActivatablePanelStack.Remove(DeactivatedPanel);
			OnActivatablePanelPopped.Broadcast(DeactivatedPanel);
		}
	}
}

void UCommonInputManager::Tick(float DeltaTime)
{
	if (CurrentlyHeldActionInputHandler)
	{
		CurrentlyHeldActionInputHandler->UpdateCurrentlyHeldAction(DeltaTime);
	}	
}

bool UCommonInputManager::StartListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent)
{
	for (int32 Index = ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
	{
		UCommonActivatablePanel* ActivatablePanel = ActivatablePanelStack[Index];
		if (ActivatablePanel && ActivatablePanel->StartListeningForExistingHeldAction(InputActionDataRow, CompleteEvent, ProgressEvent))
		{
			return true;
		}
	}

	return false;
}

bool UCommonInputManager::StopListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent)
{
	for (int32 Index = ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
	{
		UCommonActivatablePanel* ActivatablePanel = ActivatablePanelStack[Index];
		if (ActivatablePanel && ActivatablePanel->StopListeningForExistingHeldAction(InputActionDataRow, CompleteEvent, ProgressEvent))
		{
			return true;
		}
	}

	return false;
}

bool UCommonInputManager::GetAvailableInputActions(TArray<FDataTableRowHandle>& AvailableInputActions)
{
	for (int32 Index = ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
	{
		UCommonActivatablePanel* ActivatablePanel = ActivatablePanelStack[Index];
		TArray<FDataTableRowHandle> PanelInputActions;
		if (ActivatablePanel && ActivatablePanel->GetInputActions(PanelInputActions))
		{
			AvailableInputActions.Append(PanelInputActions);
		}
	}

	return AvailableInputActions.Num() > 0;
}

void UCommonInputManager::TriggerInputActionHandler(const FDataTableRowHandle& InputActionDataRow)
{
	for (int32 Index = ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
	{
		UCommonActivatablePanel* ActivatablePanel = ActivatablePanelStack[Index];
		if (ActivatablePanel && ActivatablePanel->TriggerInputActionHandler(InputActionDataRow))
		{
			break;
		}
	}
}