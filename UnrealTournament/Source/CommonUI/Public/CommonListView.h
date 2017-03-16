// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/Views/SListView.h"
#include "DataProvider.h"
#include "SObjectTableRow.h"
#include "WidgetFactory.h"
#include "ICommonUIModule.h"

#include "CommonListView.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnListViewItemClicked, UObject*, Item );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnListViewItemSelected, UObject*, Item, bool, bIsSelected );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnListViewItemWidgetCreated, UUserWidget*, Widget );

UCLASS(DisplayName = "Common List View")
class COMMONUI_API UCommonListView : public UWidget
{
	GENERATED_BODY()

	/** The object row specialization used for the tiles */
	using ObjectRowType = SObjectTableRow<TWeakObjectPtr<UObject>>;

public:
	UCommonListView(const FObjectInitializer& ObjectInitializer);

	/** Called when an item is clicked, parameter is the item clicked. */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnListViewItemClicked OnItemClicked;

	/** Called when an item is selected, parameter is the item selected. */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnListViewItemSelected OnItemSelected;

	/** Called when a widget is created for an item, parameter is the widget. May be used for additional initialization upon the created widgets. */
	UPROPERTY(BlueprintAssignable, Category = Events)
	FOnListViewItemWidgetCreated OnItemWidgetCreated;

public:
	virtual TSharedRef<class SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void BeginDestroy() override;

	/** Adds an the item to the list */
	UFUNCTION( BlueprintCallable, Category = ListView )
	void AddItem( UObject* Item ) const;

	/** Returns the item at the given index */
	UFUNCTION( BlueprintCallable, Category = ListView )
	UObject* GetItemAt( int32 Index ) const;

	/** Returns the index that the specified item is at. Will return the first found, or -1 for not found */
	UFUNCTION( BlueprintCallable, Category = ListView )
	int32 GetIndexForItem( UObject* Item ) const;

	/** Sets the height of every item in the list */
	UFUNCTION( BlueprintCallable, Category = TileView )
	void SetItemHeight( float NewHeight );

	/** Sets the padding around every item in the list */
	UFUNCTION( BlueprintCallable, Category = TileView )
	void SetDesiredItemPadding( const FMargin& DesiredPadding );

	/** Gets the current selection mode. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = ListView, Meta = (DisplayName = "Get Selection Mode"))
	TEnumAsByte<ESelectionMode::Type> GetSelectionModeBP() const;

	/** Gets the current selection mode. */
	ESelectionMode::Type GetSelectionMode() const;

	/** Sets the new selection mode, preserving the current selection where possible. */
	UFUNCTION(BlueprintCallable, Category = ListView)
	void SetSelectionMode(TEnumAsByte<ESelectionMode::Type> SelectionMode);

	/** Gets the number of items currently selected in the list */
	UFUNCTION( BlueprintCallable, Category = ListView )
	int32 GetNumItemsSelected() const;

	/** Gets the first selected item, if any; recommended that you only use this for single selection lists. */
	UFUNCTION( BlueprintCallable, Category = ListView )
	UObject* GetSelectedItem() const;

	/** Gets a list of all the currently selected items */
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = ListView )
	bool GetSelectedItems( TArray<UObject*>& Items ) const;

	/** Gets a list of all items. */
	TArray<TWeakObjectPtr<UObject>> GetAllItems() const;

	/** Gets whether the entry for the given object is currently visible in the list */
	UFUNCTION( BlueprintCallable, Category = ListView )
	bool IsItemVisible( UObject* Item ) const;

	/** Requests that the entry for the given object is scrolled into view */
	UFUNCTION( BlueprintCallable, Category = ListView )
	void ScrollIntoView( UObject* Item );

	/** Removes all items from the list */
	UFUNCTION( BlueprintCallable, Category = ListView )
	void Clear();

	/** @return True if a refresh is pending and the list will be rebuilt on the next tick */
	UFUNCTION( BlueprintCallable, Category = ListView )
	bool IsRefreshPending() const;

	/** 
	 * Manually sets the entry for the given item as the sole selected item. Will modify the first item found.
	 * If a refresh is pending, can optionally hold off on making the selection until after the refresh.
	 * Generally this is most useful when you would like to reset the contents of the list, then immediately set an item to select.
	 * @return True if an entry was found. 
	 */
	UFUNCTION( BlueprintCallable, Category = ListView )
	bool SetSelectedItem( UObject* Item, bool bWaitIfPendingRefresh = false );
	bool SetSelectedItem(const UObject* Item, bool bWaitIfPendingRefresh = false);

	/** Manually sets the entry at the given index as the sole selected item. Returns true if an entry was found. */
	UFUNCTION( BlueprintCallable, Category = ListView )
	bool SetSelectedIndex( int32 Index );

	/** Manually sets whether the entry for the given item is selected. */
	UFUNCTION( BlueprintCallable, Category = ListView )
	void SetItemSelection( UObject* Item, bool bSelected );

	/** Clear selection */
	UFUNCTION( BlueprintCallable, Category = ListView )
	void ClearSelection();

	/** Sets the array of objects to display items for in the list */
	UFUNCTION( BlueprintCallable, Category = TileView )
	void SetDataProvider( const TArray<UObject*>& InDataProvider );

	/** @return The list item widget that corresponds to the given data item */
	template <typename ListItemWidgetType = UUserWidget>
	ListItemWidgetType* GetWidgetFromItem(UObject* Item)
	{
		if (MyListView.IsValid())
		{
			TSharedPtr<ObjectRowType> ObjectRow = StaticCastSharedPtr<ObjectRowType>(MyListView->WidgetFromItem(Item));
			if (ObjectRow.IsValid())
			{
				return Cast<ListItemWidgetType>(ObjectRow->GetWidgetObject());
			}
		}
		return nullptr;
	}

	template <class DataType, class AllocatorType = FDefaultAllocator> 
	void SetDataProvider( const TArray<DataType, AllocatorType>& InDataProvider )
	{
		DataProvider->FromArray<DataType, AllocatorType>( InDataProvider );

		if (MyListView.IsValid())
		{
			MyListView->RequestListRefresh();
		}
	}

	bool IsValid() const { return DataProvider.IsValid(); }

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	/** Creates the SListView - override to create a different type of list */
	virtual TSharedRef<SListView<TWeakObjectPtr<UObject>>> RebuildListWidget();

	TSharedRef<ITableRow> HandleGenerateRow(TWeakObjectPtr<UObject> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleItemClicked(TWeakObjectPtr<UObject> Item);
	void HandleSelectionChanged(TWeakObjectPtr<UObject> Item, ESelectInfo::Type SelectInfo);
	void HandleRowReleased(const TSharedRef<ITableRow>& Row);

	float GetTotalItemHeight() const;

	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = ItemLayout, meta = ( DesignerRebuild = true ) )
	float ItemHeight;

	/** The padding desired between each element in the list. Optional for a given list item to listen. */
	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = ItemLayout, meta = ( DesignerRebuild = true ) )
	FMargin DesiredItemPadding;

	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = ListView, meta = ( DesignerRebuild = true ) )
	TSubclassOf<UUserWidget> ListItemClass;

	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = ListView )
	TEnumAsByte<ESelectionMode::Type> SelectionMode;

	UPROPERTY( EditInstanceOnly, Category = Properties )
	EConsumeMouseWheel ConsumeMouseWheel;

	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = ListView )
	bool bClearSelectionOnClick;

	/** 
	 * How many tile widgets should be pre-allocated when the list is created? 
	 * This is about the number of tiles that you expect to see at any given time.
	 */
	UPROPERTY( EditInstanceOnly, Category = ListView )
	int32 NumPreAllocatedEntries;

	TSharedPtr<SListView<TWeakObjectPtr<UObject>>> MyListView;
	TSharedPtr<TDataProvider<TWeakObjectPtr<UObject>>> DataProvider;
	TWidgetFactory<UUserWidget> ItemWidgetsFactory;
	TArray<TWeakObjectPtr<UUserWidget>> ItemWidgets;

private:
	UFUNCTION()
	void DynamicHandleItemClickedCommonButton(UCommonButton* Button);

	UFUNCTION()
	void DynamicHandleItemClickedUserWidget(UUserWidget* Widget);

	void SelectItemAfterRefresh();
	TWeakObjectPtr<UObject> ItemToSelectAfterRefresh;

	void SetSelectedItemInternal(UObject* Item);
};