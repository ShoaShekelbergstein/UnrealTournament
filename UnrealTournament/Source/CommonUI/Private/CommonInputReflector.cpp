// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonInputReflector.h"

#include "CommonUIContext.h"
#include "CommonButton.h"
#include "CommonInputManager.h"

UCommonInputReflector::UCommonInputReflector(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	
}

void UCommonInputReflector::NativeConstruct()
{
	Super::NativeConstruct();
	ensureMsgf(ButtonType, TEXT("Button type must be set in order for this widget to create a button properly"));

	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
	if (CommonInputManager)
	{
		CommonInputManager->OnActivatablePanelPushed.AddUObject(this, &ThisClass::HandleActivatablePanelPushed);
		CommonInputManager->OnActivatablePanelPopped.AddUObject(this, &ThisClass::HandleActivatablePanelPopped);
	}

	UpdateActiveButtonsToShow();
}

void UCommonInputReflector::NativeDestruct()
{
	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
	if (CommonInputManager)
	{
		CommonInputManager->OnActivatablePanelPushed.RemoveAll(this);
		CommonInputManager->OnActivatablePanelPopped.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UCommonInputReflector::UpdateActiveButtonsToShow(UCommonActivatablePanel* PoppedPanel)
{
	ClearButtons();
	InactiveButtons.Append(ActiveButtons);
	ActiveButtons.Empty();

	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	UCommonInputManager* CommonInputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
	if (CommonInputManager)
	{
		TArray<FDataTableRowHandle> AvailableInputActions;
		CommonInputManager->GetAvailableInputActions(AvailableInputActions);
		
		/** If we have any actions, we will update/add buttons and add them to the active buttons list */
		if (AvailableInputActions.Num() > 0)
		{
			if (ActiveButtons.Num() + InactiveButtons.Num() < AvailableInputActions.Num())
			{
				int32 NumButtonsToCreate = AvailableInputActions.Num() - (ActiveButtons.Num() + InactiveButtons.Num());
				for (int32 Index = 0; Index < NumButtonsToCreate; ++Index)
				{
					UCommonButton* Button = CreateWidget<UCommonButton>(GetOwningPlayer(), ButtonType);
					InactiveButtons.Add(Button);
				}
			}

			for (int32 Index = 0; Index < AvailableInputActions.Num(); ++Index)
			{
				UCommonButton* Button = InactiveButtons[Index];
				Button->SetTriggeredInputAction(AvailableInputActions[Index], PoppedPanel);
				ActiveButtons.Add(Button);
				OnButtonAdded(Button);
			}

			InactiveButtons.RemoveAt(0, AvailableInputActions.Num());
		}
	}
}

void UCommonInputReflector::HandleActivatablePanelPushed()
{
	UpdateActiveButtonsToShow();
}

void UCommonInputReflector::HandleActivatablePanelPopped(UCommonActivatablePanel* PoppedPanel)
{
	UpdateActiveButtonsToShow(PoppedPanel);
}