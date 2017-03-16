// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonGlobalInputHandler.h"
#include "CommonInputManager.h"
#include "CommonButton.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"

UCommonGlobalInputHandler::UCommonGlobalInputHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCommonGlobalInputHandler::UpdateCurrentlyHeldAction(float InDeltaTime)
{
	if (CurrentlyHeldActionWidget.IsValid() && CurrentlyHeldActionWidget->GetCachedWidget().IsValid())
	{
		if (IsWidgetVisible(CurrentlyHeldActionWidget))
		{
			if (!CurrentlyHeldAction.IsNull())
			{
				CurrentlyHeldActionHeldTime += InDeltaTime;
				FCommonGlobalInputHandlerData TempHandler(CurrentlyHeldAction, CurrentlyHeldActionWidget.Get());
				int32 Index = GlobalInputHandlersList.Find(TempHandler);
				const FCommonInputActionData* InputActionData = GetInputActionData(CurrentlyHeldAction);				
				if (Index != INDEX_NONE && InputActionData && InputActionData->HoldTime > 0.f)
				{
					const FCommonGlobalInputHandlerData& GlobalHandlerData = GlobalInputHandlersList[Index];
					float HeldPercent = FMath::Clamp(CurrentlyHeldActionHeldTime / InputActionData->HoldTime, 0.f, 1.f);
					GlobalHandlerData.InputActionHandlerData.OnActionProgress.Broadcast(HeldPercent);

					if (CurrentlyHeldActionHeldTime > InputActionData->HoldTime)
					{
						// For hold actions, we just don't really do anything with the pass through so it is currently
						// passed in as a dummy variable.
						bool bDummyPassThrough;
						GlobalHandlerData.InputActionHandlerData.OnActionComplete.Broadcast();
						GlobalHandlerData.InputActionHandlerData.OnActionCommited.ExecuteIfBound(bDummyPassThrough);
						ClearCurrentlyHeldAction();
					}
				}
			}
		}
	}
	else
	{
		ClearCurrentlyHeldAction();
	}	
}

bool UCommonGlobalInputHandler::HandleHeldAction(const FCommonGlobalInputHandlerData& GlobalInputHandlerData, const FCommonInputActionData& InputActionData, EInputEvent EventType, bool& bPassThrough)
{	
	if (EventType == IE_Pressed)
	{
		SetCurrentlyHeldAction(GlobalInputHandlerData.UserWidget, GlobalInputHandlerData.InputActionDataRow);
		return true;
	}
	else if (EventType == IE_Released)
	{
		if (IsCurrentlyHeldAction(GlobalInputHandlerData.UserWidget, GlobalInputHandlerData.InputActionDataRow))
		{
			if (CurrentlyHeldActionHeldTime < InputActionData.HoldTime)
			{
				ClearCurrentlyHeldAction();
				GlobalInputHandlerData.InputActionHandlerData.OnActionProgress.Broadcast(0.f);
			}
			return true;
		}
	}
	return false;
}

bool UCommonGlobalInputHandler::IsCurrentlyHeldAction(const TWeakObjectPtr<UUserWidget>& UserWidget, const FDataTableRowHandle& InputActionRow) const
{
	UCommonInputManager* CommonInputManager = Cast<UCommonInputManager>(GetOuter());
	if (CommonInputManager)
	{
		TScriptInterface<ICommonActionHandlerInterface> ActionHandler = CommonInputManager->GetHeldInputActionHandler();
		bool IsCurrentlyHeldActionPanel = ActionHandler.GetObject() == this;
		return IsCurrentlyHeldActionPanel && InputActionRow == CurrentlyHeldAction && CurrentlyHeldActionWidget == UserWidget && CurrentlyHeldActionHeldTime > 0.f;
	}
	return false;
}

void UCommonGlobalInputHandler::SetCurrentlyHeldAction(const TWeakObjectPtr<UUserWidget>& UserWidget, const FDataTableRowHandle& InputActionRow)
{
	UCommonInputManager* CommonInputManager = Cast<UCommonInputManager>(GetOuter());
	if (CommonInputManager)
	{
		CurrentlyHeldActionWidget = UserWidget;
		CurrentlyHeldAction = InputActionRow;
		CurrentlyHeldActionHeldTime = 0.f;
		CommonInputManager->SetHeldInputActionHandler(this);
	}
}

void UCommonGlobalInputHandler::ClearCurrentlyHeldAction()
{
	UCommonInputManager* CommonInputManager = Cast<UCommonInputManager>(GetOuter());
	if (CommonInputManager)
	{
		CurrentlyHeldActionWidget = nullptr;
		CurrentlyHeldAction = FDataTableRowHandle();
		CurrentlyHeldActionHeldTime = 0.f;
		CommonInputManager->SetHeldInputActionHandler(this);
	}
}


bool UCommonGlobalInputHandler::IsWidgetVisible(TWeakObjectPtr<UUserWidget>& UserWidget)
{
	if (!UserWidget->GetIsEnabled())
	{
		return false;
	}
	else
	{
		UCommonButton* CommonButton = Cast<UCommonButton>(UserWidget.Get());
		if (CommonButton && !CommonButton->IsInteractionEnabled())
		{
			return false;
		}
	}

	FWidgetPath WidgetPath;
	FSlateApplication& Application = FSlateApplication::Get();
	TSharedRef<SWidget> SlateWidget = UserWidget->GetCachedWidget().ToSharedRef();
	if (Application.GeneratePathToWidgetUnchecked(SlateWidget, WidgetPath, EVisibility::Visible))
	{
		if (WidgetPath.ContainsWidget(SlateWidget))
		{
			return true;
		}
	}
	
	return false;
}


bool UCommonGlobalInputHandler::HandleInput(FKey KeyPressed, EInputEvent EventType, bool& bPassThrough)
{
	for (int32 Index = 0; Index < GlobalInputHandlersList.Num(); ++Index)
	{
		FCommonGlobalInputHandlerData& HandlerData = GlobalInputHandlersList[Index];
		TWeakObjectPtr<UUserWidget>& UserWidget = HandlerData.UserWidget;
		if (UserWidget.IsValid() && UserWidget->GetCachedWidget().IsValid())
		{
			if (IsWidgetVisible(HandlerData.UserWidget))
			{
				if (IsKeyBoundToInputActionData(KeyPressed, HandlerData.InputActionDataRow))
				{
					const FCommonInputActionData* InputActionData = GetInputActionData(HandlerData.InputActionDataRow);
					if (InputActionData)
					{
						if (InputActionData->bActionRequiresHold)
						{
							HandleHeldAction(HandlerData, *InputActionData, EventType, bPassThrough);
						}
						else if (EventType == IE_Pressed)
						{
							HandlerData.InputActionHandlerData.OnActionCommited.ExecuteIfBound(bPassThrough);
							return true;
						}
					}
				}
			}
		}		
		else
		{
			ensureMsgf(false, TEXT("Did some widget not clean up?"));
			GlobalInputHandlersList.RemoveAt(Index);
			--Index;
		}
	}

	return false;
}

void UCommonGlobalInputHandler::RegisterInputAction(const UUserWidget* UserWidget, FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, FCommonActionCompleteSingle CompleteEvent, FCommonActionProgressSingle ProgressEvent)
{
	if (InputActionRow.IsNull())
	{
		UE_LOG(LogCommonUI, Warning, TEXT("UCommonActivatableManager::RegisterGlobalInputDelegate, Trying to add an invalid input action to listen for input on global input handlers."));
		return;
	}
	if (UserWidget == nullptr)
	{
		UE_LOG(LogCommonUI, Warning, TEXT("UCommonActivatableManager::RegisterGlobalInputDelegate, Trying to add an invalid Widget to listen for input on global input handlers."));
		return;
	}

	FCommonGlobalInputHandlerData Handler(InputActionRow, UserWidget);
	int32 FoundIndex = GlobalInputHandlersList.Find(Handler);
	if (FoundIndex != INDEX_NONE)
	{
		GlobalInputHandlersList[FoundIndex].InputActionHandlerData.OnActionCommited = CommitedEvent;
		GlobalInputHandlersList[FoundIndex].InputActionHandlerData.OnActionComplete.Add(CompleteEvent);
		GlobalInputHandlersList[FoundIndex].InputActionHandlerData.OnActionProgress.Add(ProgressEvent);
	}
	else
	{
		Handler.InputActionHandlerData.OnActionCommited = CommitedEvent;
		Handler.InputActionHandlerData.OnActionProgress.Add(ProgressEvent);
		Handler.InputActionHandlerData.OnActionComplete.Add(CompleteEvent);
		GlobalInputHandlersList.Add(Handler);
	}

	GlobalInputHandlersList.Sort([](const FCommonGlobalInputHandlerData& A, const FCommonGlobalInputHandlerData& B)
	{
		if (A.InputActionDataRow != B.InputActionDataRow)
		{
			return A.InputActionDataRow.RowName.Compare(B.InputActionDataRow.RowName) < 0;
		}

		if (!A.UserWidget.IsValid())
		{
			return false;
		}
		else if (!B.UserWidget.IsValid())
		{
			return true;
		}
		return A.UserWidget->Priority > B.UserWidget->Priority;
	});
}

void UCommonGlobalInputHandler::UnregisterInputAction(const UUserWidget* UserWidget, FDataTableRowHandle InputActionRow)
{
	if (InputActionRow.IsNull())
	{
		UE_LOG(LogCommonUI, Warning, TEXT("UCommonActivatableManager::RegisterGlobalInputDelegate, Trying to remove an invalid input action to listen for input on global input handlers."));
		return;
	}
	if (UserWidget == nullptr)
	{
		UE_LOG(LogCommonUI, Warning, TEXT("UCommonActivatableManager::RegisterGlobalInputDelegate, Trying to remove an invalid Widget to listen for input on global input handlers."));
		return;
	}

	FCommonGlobalInputHandlerData HandlerToFind(InputActionRow, UserWidget);
	int32 RemovedIndex = GlobalInputHandlersList.Remove(HandlerToFind);
	if (RemovedIndex == INDEX_NONE)
	{
		UE_LOG(LogCommonUI, Warning, TEXT("UCommonActivatableManager::RegisterGlobalInputDelegate, Trying to remove an entry that does not exist."));
	}
}
