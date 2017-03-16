// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CommonWidgetSwitcher.h"
#include "CommonUITypes.h"
#include "CommonInputManager.h"

#include "CommonTabListWidget.generated.h"

class UCommonButton;
class UCommonButtonGroup;
class UCommonWidgetSwitcher;

/** Information about a registered tab in the tab list */
USTRUCT()
struct FCommonRegisteredTabInfo
{
	GENERATED_BODY()

public:
	/** The index of the tab in the list */
	UPROPERTY()
	int32 TabIndex;
	
	/** The actual button widget that represents this tab on-screen */
	UPROPERTY()
	UCommonButton* TabButton;

	/** The actual instance of the content widget to display when this tab is selected. Can be null if a load is required. */
	UPROPERTY()
	UWidget* ContentInstance;

	FCommonRegisteredTabInfo()
		: TabIndex(INDEX_NONE)
		, TabButton(nullptr)
		, ContentInstance(nullptr)
	{}
};

/** Base class for a list of selectable tabs that correspondingly activate and display an arbitrary widget in a linked switcher */
UCLASS(Abstract, Blueprintable, ClassGroup = UI, meta = (Category = "Common UI"))
class COMMONUI_API UCommonTabListWidget : public UCommonUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** Delegate broadcast when a new tab is selected. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabSelected, FName, TabId);

	/** Broadcasts when a new tab is selected. */
	UPROPERTY(BlueprintAssignable, Category = TabList)
	FOnTabSelected OnTabSelected;

	/** Delegate broadcast when a new tab is created. Allows hook ups after creation. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabButtonCreated, FName, TabId, UCommonButton*, TabButton);

	/** Broadcasts when a new tab is created. */
	UPROPERTY(BlueprintAssignable, Category = TabList)
	FOnTabButtonCreated OnTabButtonCreated;

	/** Delegate broadcast when a tab is being removed. Allows clean ups after destruction. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabButtonRemoved, FName, TabId, UCommonButton*, TabButton);

	/** Broadcasts when a new tab is created. */
	UPROPERTY(BlueprintAssignable, Category = TabList)
	FOnTabButtonRemoved OnTabButtonRemoved;

	/** @return The currently active (selected) tab */
	UFUNCTION(BlueprintCallable, Category = TabList)
	FName GetActiveTab() const { return ActiveTabID; }

	/**
	 * Establishes the activatable widget switcher instance that this tab list should interact with
	 * @param CommonSwitcher The switcher that this tab list should be associated with and manipulate
	 */
	UFUNCTION(BlueprintCallable, Category = TabList)
	void SetLinkedSwitcher(UCommonWidgetSwitcher* CommonSwitcher);

	/** @return The switcher that this tab list is associated with and manipulates */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = TabList)
	UCommonWidgetSwitcher* GetLinkedSwitcher() const;

	/**
	 * Registers and adds a new tab to the list that corresponds to a given widget instance. If not present in the linked switcher, it will be added.
	 * @param TabID The name ID used to keep track of this tab. Attempts to register a tab under a duplicate ID will fail.
	 * @param ButtonWidgetType The widget type to create for this tab
	 * @param ContentWidget The widget to associate with the registered tab
	 * @return True if the new tab registered successfully and there were no name ID conflicts
	 */
	UFUNCTION(BlueprintCallable, Category = TabList)
	bool RegisterTab(FName TabNameID, TSubclassOf<UCommonButton> ButtonWidgetType, UWidget* ContentWidget);

	UFUNCTION(BlueprintCallable, Category = TabList)
	bool RemoveTab(FName TabNameID);

	UFUNCTION(BlueprintCallable, Category = TabList)
	void RemoveAllTabs();

	/** 
	 * Selects the tab registered under the provided name ID
	 * @param TabNameID The name ID for the tab given when registered
	 */
	UFUNCTION(BlueprintCallable, Category = TabList)
	bool SelectTabByID(FName TabNameID, bool bSuppressClickFeedback = false );

	/** Sets whether the tab associated with the given ID is enabled/disabled */
	UFUNCTION(BlueprintCallable, Category = TabList)
	void SetTabEnabled(FName TabNameID, bool bEnable);

	/** Disables the tab associated with the given ID with a reason */
	UFUNCTION(BlueprintCallable, Category = TabList)
	void DisableTabWithReason(FName TabNameID, const FText& Reason);

	UFUNCTION(BlueprintCallable, Category = TabList)
	void SetListeningForInput(bool bShouldListen);

protected:
	// UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// End UUserWidget

	UFUNCTION(BlueprintImplementableEvent, Category = TabList, meta = (BlueprintProtected = "true"))
	void HandlePreLinkedSwitcherChanged_BP();

	virtual void HandlePreLinkedSwitcherChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = TabList, meta = (BlueprintProtected = "true"))
	void HandlePostLinkedSwitcherChanged_BP();

	virtual void HandlePostLinkedSwitcherChanged();

	UFUNCTION(BlueprintNativeEvent, Category = TabList, meta = (BlueprintProtected = "true"))
	void HandleTabCreated(FName TabNameID, UCommonButton* TabButton);

	UFUNCTION(BlueprintNativeEvent, Category = TabList, meta = (BlueprintProtected = "true"))
	void HandleTabRemoved(FName TabNameID, UCommonButton* TabButton);

	/** Returns the tab button matching the ID, if found */
	UFUNCTION(BlueprintCallable, Category = TabList, meta = (BlueprintProtected = "true"))
	UCommonButton* GetTabButtonByID(FName TabNameID);

	/** The input action to listen for causing the next tab to be selected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TabList, meta = (RowType = CommonInputActionData))
	FDataTableRowHandle NextTabInputActionData;

	/** The input action to listen for causing the previous tab to be selected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TabList, meta = (RowType = CommonInputActionData))
	FDataTableRowHandle PreviousTabInputActionData;

	/** Whether to register to handle tab list input immediately upon construction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TabList, meta = (ExposeOnSpawn = "true"))
	bool bAutoListenForInput;

private:
	UFUNCTION()
	void HandleTabButtonSelected(UCommonButton* SelectedTabButton, int32 ButtonIndex);
	
	void SetListeningForInputInternal(bool bListenForInput, FDataTableRowHandle InputActionRow, FCommonActionCommited& Delegate);
	
	UFUNCTION()
	void HandlePreviousTabInputAction(bool& bPassthrough);
	
	UFUNCTION()
	void HandleNextTabInputAction(bool& bPassthrough);

	/** The activatable widget switcher that this tab list is associated with and manipulates */
	UPROPERTY()
	UCommonWidgetSwitcher* LinkedSwitcher;

	/** Info about each of the currently registered tabs organized by a given registration name ID */
	UPROPERTY()
	TMap<FName, FCommonRegisteredTabInfo> RegisteredTabsByID;

	/** The button group that manages all the created tab buttons */
	UPROPERTY()
	UCommonButtonGroup* TabButtonGroup;

	/** The registration ID of the currently active tab */
	FName ActiveTabID;

	/** Is the tab list currently listening for tab input actions? */
	bool bIsListeningForInput;
};