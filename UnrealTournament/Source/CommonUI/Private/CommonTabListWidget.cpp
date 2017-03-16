// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonTabListWidget.h"

#include "CommonUIContext.h"
#include "CommonButtonGroup.h"
#include "CommonWidgetSwitcher.h"
#include "WidgetLayoutLibrary.h"
#include "BlueprintContextLibrary.h"
#include "CommonInputManager.h"
#include "CommonGlobalInputHandler.h"
#include "ICommonUIModule.h"

UCommonTabListWidget::UCommonTabListWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoListenForInput(false)
	, LinkedSwitcher(nullptr)
	, RegisteredTabsByID()
	, TabButtonGroup(nullptr)
	, ActiveTabID(NAME_None)
	, bIsListeningForInput(false)
{
}
 
void UCommonTabListWidget::SetLinkedSwitcher(UCommonWidgetSwitcher* CommonSwitcher)
{
	if (LinkedSwitcher != CommonSwitcher)
	{
		HandlePreLinkedSwitcherChanged();
		LinkedSwitcher = CommonSwitcher;
		HandlePostLinkedSwitcherChanged();
	}
}

UCommonWidgetSwitcher* UCommonTabListWidget::GetLinkedSwitcher() const
{
	return LinkedSwitcher;
}

bool UCommonTabListWidget::RegisterTab(FName TabNameID, TSubclassOf<UCommonButton> ButtonWidgetType, UWidget* ContentWidget)
{
	bool AreParametersValid = true;

	// Early out on redundant tab registration.
	if (!ensure(!RegisteredTabsByID.Contains(TabNameID)))
	{
		AreParametersValid = false;
	}

	// Early out on invalid tab button type.
	if (!ensure(ButtonWidgetType))
	{
		AreParametersValid = false;
	}

	if (!AreParametersValid)
	{
		return false;
	}

	UCommonButton* const NewTabButton = CreateWidget<UCommonButton>(GetOwningPlayer(), ButtonWidgetType);
	if (!ensureMsgf(NewTabButton, TEXT("Failed to create tab button. Aborting tab registration.")))
	{
		return false;
	}

	// Tab book-keeping.
	FCommonRegisteredTabInfo NewTabInfo;
	NewTabInfo.TabIndex = RegisteredTabsByID.Num();
	NewTabInfo.TabButton = NewTabButton;
	NewTabInfo.ContentInstance = ContentWidget;
	RegisteredTabsByID.Add(TabNameID, NewTabInfo);

	// Enforce the "contract" that tab buttons require - single-selectability, but not toggleability.
	NewTabButton->SetIsSelectable(true);
	NewTabButton->SetIsToggleable(false);
	// NOTE: Adding the button to the group may change it's selection, which raises an event we listen to,
	// which can only properly be handled if we already know that this button is associated with a registered tab.
	TabButtonGroup->AddWidget(NewTabButton);

	// Callbacks.
	HandleTabCreated(TabNameID, NewTabInfo.TabButton);
	OnTabButtonCreated.Broadcast(TabNameID, NewTabInfo.TabButton);

	return true;
}


bool UCommonTabListWidget::RemoveTab(FName TabNameID)
{
	FCommonRegisteredTabInfo* const TabInfo = RegisteredTabsByID.Find(TabNameID);
	if (!TabInfo)
	{
		return false;
	}

	UCommonButton* const TabButton = TabInfo->TabButton;
	if (TabButton)
	{
		TabButton->RemoveFromParent();
	}
	RegisteredTabsByID.Remove(TabNameID);

	// Callbacks
	HandleTabRemoved(TabNameID, TabButton);
	OnTabButtonRemoved.Broadcast(TabNameID, TabButton);

	return true;
}

void UCommonTabListWidget::RemoveAllTabs()
{
	for (const auto& Pair : RegisteredTabsByID)
	{
		RemoveTab(Pair.Key);
	}
}

void UCommonTabListWidget::SetListeningForInput(bool bShouldListen)
{
	if (bShouldListen && !TabButtonGroup)
	{
		// If there's no tab button group, it means we haven't been constructed and we shouldn't listen to anything
		return;
	}

	if (bShouldListen != bIsListeningForInput)
	{
		bIsListeningForInput = bShouldListen;

		FCommonActionCommited PreviousTabInputDelegate;
		PreviousTabInputDelegate.BindDynamic(this, &ThisClass::HandlePreviousTabInputAction);
		SetListeningForInputInternal(bIsListeningForInput, PreviousTabInputActionData, PreviousTabInputDelegate);

		FCommonActionCommited NextTabInputDelegate;
		NextTabInputDelegate.BindDynamic(this, &ThisClass::HandleNextTabInputAction);
		SetListeningForInputInternal(bIsListeningForInput, NextTabInputActionData, NextTabInputDelegate);
	}
}

void UCommonTabListWidget::SetListeningForInputInternal(bool bListenForInput, FDataTableRowHandle InputActionRow, FCommonActionCommited& CommitedEvent)
{
	UCommonUIContext* CommonUIContext = GetContext<UCommonUIContext>();
	UCommonInputManager* InputManager = CommonUIContext ? CommonUIContext->GetInputManager() : nullptr;
	UCommonGlobalInputHandler* GlobalInputHandler = InputManager ? InputManager->GetGlobalInputHandler() : nullptr;

	if (InputManager)
	{
		if (bListenForInput)
		{
			GlobalInputHandler->RegisterInputAction(this, InputActionRow, CommitedEvent);
		}
		else
		{
			GlobalInputHandler->UnregisterInputAction(this, InputActionRow);
		}
	}
}

bool UCommonTabListWidget::SelectTabByID(FName TabNameID, bool bSuppressClickFeedback)
{
	for (auto& TabPair : RegisteredTabsByID)
	{
		if (TabPair.Key == TabNameID && ensure(TabPair.Value.TabButton))
		{
			TabPair.Value.TabButton->SetIsSelected(true, !bSuppressClickFeedback);
			return true;
		}
	}

	return false;
}

void UCommonTabListWidget::SetTabEnabled(FName TabNameID, bool bEnable)
{
	for (auto& TabPair : RegisteredTabsByID)
	{
		if (TabPair.Key == TabNameID && ensure(TabPair.Value.TabButton))
		{
			if (bEnable)
			{
				TabPair.Value.TabButton->EnableButton();
			}
			else
			{
				TabPair.Value.TabButton->DisableButton();
			}

			break;
		}
	}
}

void UCommonTabListWidget::DisableTabWithReason(FName TabNameID, const FText& Reason)
{
	for (auto& TabPair : RegisteredTabsByID)
	{
		if (TabPair.Key == TabNameID && ensure(TabPair.Value.TabButton))
		{
			TabPair.Value.TabButton->DisableButtonWithReason(Reason);
			break;
		}
	}
}

UCommonButton* UCommonTabListWidget::GetTabButtonByID(FName TabNameID)
{
	if (FCommonRegisteredTabInfo* TabInfo = RegisteredTabsByID.Find(TabNameID))
	{
		return TabInfo->TabButton;
	}

	return nullptr;
}

void UCommonTabListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Create the base button group
	TabButtonGroup = NewObject<UCommonButtonGroup>(this);
	TabButtonGroup->SetSelectionRequired(true);
	TabButtonGroup->OnSelectedButtonChanged.AddDynamic(this, &UCommonTabListWidget::HandleTabButtonSelected);

	if (bAutoListenForInput)
	{
		SetListeningForInput(true);
	}
}

void UCommonTabListWidget::NativeDestruct()
{
	Super::NativeDestruct();
	SetListeningForInput(false);

	TabButtonGroup->RemoveAll();
	TabButtonGroup->MarkPendingKill();
	TabButtonGroup = nullptr;
	ActiveTabID = NAME_None;
	RemoveAllTabs();
	LinkedSwitcher = nullptr;
}

void UCommonTabListWidget::HandlePreLinkedSwitcherChanged()
{
	HandlePreLinkedSwitcherChanged_BP();
}

void UCommonTabListWidget::HandlePostLinkedSwitcherChanged()
{
	HandlePostLinkedSwitcherChanged_BP();
}

void UCommonTabListWidget::HandleTabCreated_Implementation(FName TabNameID, UCommonButton* TabButton)
{
}

void UCommonTabListWidget::HandleTabRemoved_Implementation(FName TabNameID, UCommonButton* TabButton)
{
}

void UCommonTabListWidget::HandleTabButtonSelected(UCommonButton* SelectedTabButton, int32 ButtonIndex)
{
	for (auto& TabPair : RegisteredTabsByID)
	{
		FCommonRegisteredTabInfo& TabInfo = TabPair.Value;
			
		if (TabInfo.TabButton == SelectedTabButton)
		{
			ActiveTabID = TabPair.Key;

			if (TabInfo.ContentInstance || LinkedSwitcher)
			{
				if (ensureMsgf(TabInfo.ContentInstance, TEXT("A CommonTabListWidget tab button lacks a tab content widget to set its linked switcher to.")) &&
					ensureMsgf(LinkedSwitcher, TEXT("A CommonTabListWidget has a registered tab with a content widget to switch to, but has no linked activatable widget switcher. Did you forget to call SetLinkedSwitcher to establish the association?")))
				{
					// There's already an instance of the widget to display, so go for it
					LinkedSwitcher->SetActiveWidget(TabInfo.ContentInstance);
				}
			}

			OnTabSelected.Broadcast(TabPair.Key);
		}
	}
}

void UCommonTabListWidget::HandlePreviousTabInputAction(bool& bPassThrough)
{
	if (ensure(TabButtonGroup))
	{
		TabButtonGroup->SelectPreviousButton();
	}
}

void UCommonTabListWidget::HandleNextTabInputAction(bool& bPassThrough)
{
	if (ensure(TabButtonGroup))
	{
		TabButtonGroup->SelectNextButton();
	}
}

