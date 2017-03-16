// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonActivatablePanel.h"

#include "CommonUIContext.h"
#include "CommonInputManager.h"

UCommonActivatablePanel::UCommonActivatablePanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bConsumeAllActions(false)
	, bExposeActionsExternally(true)
	, bIsActive(false)
	, CurrentlyHeldActionHeldTime(0.f)
{
}

uint32 UCommonActivatablePanel::FInputActionRowFuncs::GetKeyHash(const FDataTableRowHandle& InputActionRow)
{
	uint32 Hash = 0;
	Hash = HashCombine(Hash, GetTypeHash(InputActionRow.RowName));
	Hash = HashCombine(Hash, GetTypeHash(InputActionRow.DataTable));
	return Hash;
}

void UCommonActivatablePanel::NativeDestruct()
{
	if (CommonInputManager.IsValid())
	{
		CommonInputManager->PopActivatablePanel(this);
	}

	OnWidgetActivated.Clear();
	OnWidgetDeactivated.Clear();
	Super::NativeDestruct();
}

void UCommonActivatablePanel::UpdateCurrentlyHeldAction(float InDeltaTime)
{
	if (!CurrentlyHeldAction.IsNull())
	{
		CurrentlyHeldActionHeldTime += InDeltaTime;
		const FCommonInputActionData* InputActionData = GetInputActionData(CurrentlyHeldAction);
		if (InputActionData && InputActionData->HoldTime > 0.f)
		{
			const FCommonInputActionHandlerData* InputActionHandler = InputActionHandlers.Find(CurrentlyHeldAction);
			if (InputActionHandler)
			{
				float HeldPercent = FMath::Clamp(CurrentlyHeldActionHeldTime / InputActionData->HoldTime, 0.f, 1.f);
				InputActionHandler->OnActionProgress.Broadcast(HeldPercent);

				if (CurrentlyHeldActionHeldTime > InputActionData->HoldTime)
				{
					// For hold actions, we just don't really do anything with the pass through so it is currently
					// passed in as a dummy variable.
					bool bDummyPassThrough;
					InputActionHandler->OnActionComplete.Broadcast();
					InputActionHandler->OnActionCommited.ExecuteIfBound(bDummyPassThrough);
					ClearCurrentlyHeldAction();
				}
			}			
		}
	}	
}

void UCommonActivatablePanel::BeginActivate(UCommonInputManager* InCommonInputManager)
{
	CommonInputManager = InCommonInputManager;
	OnBeginActivate();
}

void UCommonActivatablePanel::OnBeginActivate_Implementation()
{	
	EndActivate();
}

void UCommonActivatablePanel::EndActivate()
{
	if (!bIsActive)
	{
		bIsActive = true;		
		NativeOnActivated();
	}
}

void UCommonActivatablePanel::BeginDeactivate()
{
	if (bIsActive)
	{
		bIsActive = false;
		ClearCurrentlyHeldAction();
		OnBeginDeactivate();
	}
}

void UCommonActivatablePanel::OnBeginDeactivate_Implementation()
{
	EndDeactivate();
}

void UCommonActivatablePanel::EndDeactivate()
{
	NativeOnDeactivated();
}

void UCommonActivatablePanel::PopPanel()
{
	if (CommonInputManager.IsValid())
	{
		CommonInputManager->PopActivatablePanel(this);
	}
}

bool UCommonActivatablePanel::GetInputActions(TArray<FDataTableRowHandle>& InputActionDataRows) const
{
	if (bExposeActionsExternally)
	{
		InputActionHandlers.GenerateKeyArray(InputActionDataRows);
		return InputActionDataRows.Num() > 0;
	}

	return false;
}

bool UCommonActivatablePanel::TriggerInputActionHandler(const FDataTableRowHandle& InputActionDataRow)
{
	const FCommonInputActionHandlerData* InputActionHandler = InputActionHandlers.Find(InputActionDataRow);
	if (IsActivated() && InputActionHandler)
	{
		// For actions that are triggered by buttons or other widgets, we just don't really do 
		// anything with the pass through so it is currently passed in as a dummy variable.
		bool bPassthroughDummy;
		InputActionHandler->OnActionCommited.ExecuteIfBound(bPassthroughDummy);
		return true;
	}

	return false;
}

bool UCommonActivatablePanel::HandleHeldAction(const FDataTableRowHandle& InputActionRow, const FCommonInputActionHandlerData& InputActionHandler, const FCommonInputActionData& InputActionData, EInputEvent EventType, bool& bPassThrough)
{
	if (EventType == IE_Pressed)
	{
		SetCurrentlyHeldAction(InputActionRow);
		return true;
	}
	else if (EventType == IE_Released)
	{
		if (IsCurrentlyHeldAction(InputActionRow))
		{
			if (CurrentlyHeldActionHeldTime < InputActionData.HoldTime)
			{
				ClearCurrentlyHeldAction();
				InputActionHandler.OnActionProgress.Broadcast(0.f);
			}
			return true;
		}
	}

	return false;
}

bool UCommonActivatablePanel::IsCurrentlyHeldAction(const FDataTableRowHandle& InputActionRow) const
{
	if (CommonInputManager.IsValid())
	{
		TScriptInterface<ICommonActionHandlerInterface> ActionHandler = CommonInputManager->GetHeldInputActionHandler();
		bool IsCurrentlyHeldActionPanel = ActionHandler.GetObject() == this;
		return IsCurrentlyHeldActionPanel && InputActionRow == CurrentlyHeldAction && CurrentlyHeldActionHeldTime > 0.f;
	}
	return false;
}

void UCommonActivatablePanel::SetCurrentlyHeldAction(const FDataTableRowHandle& InputActionRow)
{
	if (CommonInputManager.IsValid())
	{
		CurrentlyHeldAction = InputActionRow;
		CurrentlyHeldActionHeldTime = 0.f;
		CommonInputManager->SetHeldInputActionHandler(this);
	}
}

void UCommonActivatablePanel::ClearCurrentlyHeldAction()
{
	if (CommonInputManager.IsValid())
	{
		CurrentlyHeldActionHeldTime = 0.f;
		CurrentlyHeldAction = FDataTableRowHandle();
		CommonInputManager->SetHeldInputActionHandler(nullptr);
	}	
}

bool UCommonActivatablePanel::HandleInput(FKey KeyPressed, EInputEvent EventType, bool& bPassThrough)
{
	if (!IsActivated())
	{
		return false;
	}
	
	for (auto Iterator = InputActionHandlers.CreateConstIterator(); Iterator; ++Iterator)
	{
		if (IsKeyBoundToInputActionData(KeyPressed, Iterator.Key()))
		{
			const FCommonInputActionData* InputActionData = GetInputActionData(Iterator.Key());
			if (InputActionData)
			{
				if (InputActionData->bActionRequiresHold)
				{
					return HandleHeldAction(Iterator.Key(), Iterator.Value(), *InputActionData, EventType, bPassThrough);
				}
				else if (EventType == IE_Pressed)
				{
					Iterator.Value().OnActionCommited.ExecuteIfBound(bPassThrough);
					return true;
				}
			}
		}		
	}

	if (bConsumeAllActions)
	{
		bPassThrough = false;
		return true;
	}

	return false;
}

void UCommonActivatablePanel::NativeOnActivated()
{
	OnActivated();
	OnWidgetActivated.Broadcast(this);
}

void UCommonActivatablePanel::NativeOnDeactivated()
{
	OnDeactivated();
	OnWidgetDeactivated.Broadcast(this);
}

bool UCommonActivatablePanel::StartListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent)
{
	FCommonInputActionHandlerData* ExistingHandler = InputActionHandlers.Find(InputActionDataRow);
	const FCommonInputActionData* InputActionData = GetInputActionData(InputActionDataRow);
	if (ExistingHandler && ensure(InputActionData) && InputActionData->bActionRequiresHold)
	{		
		ExistingHandler->OnActionComplete.Add(CompleteEvent);
		ExistingHandler->OnActionProgress.Add(ProgressEvent);
		return true;
	}

	return false;
}

bool UCommonActivatablePanel::StopListeningForExistingHeldAction(const FDataTableRowHandle& InputActionDataRow, const FCommonActionCompleteSingle& CompleteEvent, const FCommonActionProgressSingle& ProgressEvent)
{
	FCommonInputActionHandlerData* ExistingHandler = InputActionHandlers.Find(InputActionDataRow);
	const FCommonInputActionData* InputActionData = GetInputActionData(InputActionDataRow);
	if (ExistingHandler && ensure(InputActionData) && InputActionData->bActionRequiresHold)
	{
		ExistingHandler->OnActionComplete.Remove(CompleteEvent);
		ExistingHandler->OnActionProgress.Remove(ProgressEvent);
		return true;
	}

	return false;
}

void UCommonActivatablePanel::SetInputActionHandler(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent)
{
	SetInputActionHandlerWithProgress(InputActionRow, CommitedEvent, FCommonActionProgressSingle());
}

void UCommonActivatablePanel::SetInputActionHandlerWithProgress(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent)
{
	if (InputActionRow.IsNull())
	{
		UE_LOG(LogCommonUI, Warning, TEXT("Setting an input action handler to an invalid input action row!"));
		return;
	}

	FCommonInputActionHandlerData* ExistingHandler = InputActionHandlers.Find(InputActionRow);

	if (!ensure(ExistingHandler == nullptr))
	{
		UE_LOG(LogCommonUI, Warning, TEXT("Setting an input action handler to an already existing input action."));
		return;
	}
	InputActionHandlers.Add(InputActionRow, FCommonInputActionHandlerData(ProgressEvent, CommitedEvent));
}

bool UCommonActivatablePanel::ShouldBypassStack()
{
	if (bShouldBypassStack)
	{
		return ensure(InputActionHandlers.Num() == 0);
	}
	
	return false;
}