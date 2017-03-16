// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonListView.h"
#include "CommonObjectListItem.h"
#include "CommonButton.h"
#include "Algo/Transform.h"
#include "CommonWidgetPaletteCategories.h"
#include "TimerManager.h"

namespace ListViewText
{
	static const FText skFailedToGenerate =
	    NSLOCTEXT( "Common.ListView", "FailedToGenerate",
	               "The widget you've provided doesn't implement the List Item interface or your data is invalid." );
}

UCommonListView::UCommonListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemHeight(128.f)
	, ListItemClass(nullptr)
	, SelectionMode(ESelectionMode::Single)
	, ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
	, bClearSelectionOnClick(false)
	, NumPreAllocatedEntries(1)
{
	Visibility = ESlateVisibility::Visible;
}

TSharedRef<SWidget> UCommonListView::RebuildWidget()
{
	DataProvider = MakeShareable(new TDataProvider<TWeakObjectPtr<UObject>>);

	if (!IsDesignTime())
	{
		if (ListItemClass)
		{
			UClass* ItemClass = ListItemClass != nullptr ? *ListItemClass : UUserWidget::StaticClass();
			ItemWidgetsFactory = TWidgetFactory<UUserWidget>(
				ItemClass,
				[this]() -> UGameInstance*
			{
				return GetWorld()->GetGameInstance();
			});

			ItemWidgetsFactory.PreConstruct(NumPreAllocatedEntries);
		}
		else
		{
			UE_LOG(LogCommonUI, Error, TEXT("[%s] has no row widget class specified!"), *GetName());
		}
	}

	return BuildDesignTimeWidget( RebuildListWidget() );
}

void UCommonListView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Clear();
	MyListView.Reset();

	ItemWidgetsFactory.Reset(bReleaseChildren);
}

void UCommonListView::BeginDestroy()
{
	Super::BeginDestroy();

	if (MyListView.IsValid())
	{
		MyListView.Reset();
	}

	DataProvider.Reset();
}


void UCommonListView::AddItem(UObject* Item) const
{
	TWeakObjectPtr<UObject>& NewObject = DataProvider->Add();
	NewObject = Item;
	
	if (MyListView.IsValid())
	{
		MyListView->RequestListRefresh();
	}
}

UObject* UCommonListView::GetItemAt(int32 Index) const
{
	return DataProvider->IsValidIndex(Index) ? DataProvider->At(Index).Get() : nullptr;
}

int32 UCommonListView::GetIndexForItem( UObject* Item ) const
{
	for ( int32 ItemIdx = 0; ItemIdx < DataProvider->Num(); ++ItemIdx )
	{
		if ( Item == DataProvider->At(ItemIdx) )
		{
			return ItemIdx;
		}
	}

	return INDEX_NONE;
}

void UCommonListView::SetItemHeight(float NewHeight)
{
	ItemHeight = NewHeight;
	if ( MyListView.IsValid() )
	{
		MyListView->SetItemHeight( GetTotalItemHeight() );
	}
}

void UCommonListView::SetDesiredItemPadding(const FMargin& DesiredPadding)
{
	DesiredItemPadding = DesiredPadding;
	
	if (MyListView.IsValid())
	{
		MyListView->RequestListRefresh();
	}
}

TEnumAsByte<ESelectionMode::Type> UCommonListView::GetSelectionModeBP() const
{
	return SelectionMode;
}

ESelectionMode::Type UCommonListView::GetSelectionMode() const
{
	return SelectionMode;
}

void UCommonListView::SetSelectionMode(TEnumAsByte<ESelectionMode::Type> NewSelectionMode)
{
	// Early out on redundant set.
	if (SelectionMode == NewSelectionMode)
	{
		return;
	}

	if (NewSelectionMode == ESelectionMode::None)
	{
		// Clear any current selection.
		ClearSelection();
	}
	else if (NewSelectionMode == ESelectionMode::Single || NewSelectionMode == ESelectionMode::SingleToggle)
	{
		// Try to preserve the last selected item.
		TArray<UObject*> CurrentlySelectedItems;
		GetSelectedItems(CurrentlySelectedItems);
		UObject* const LastSelectedItem = CurrentlySelectedItems.Num() > 0 ? CurrentlySelectedItems.Last(0) : nullptr;
		if (LastSelectedItem)
		{
			ClearSelection();
			SetSelectedItem(LastSelectedItem);
		}
	}
	else if (NewSelectionMode == ESelectionMode::Multi)
	{
		// No selection changes needed.
	}

	SelectionMode = NewSelectionMode;

	// Update toggleability of item widgets that are buttons.
	for (const TWeakObjectPtr<UUserWidget> ItemWidget : ItemWidgets)
	{
		UCommonButton* const BaseButton = Cast<UCommonButton>(ItemWidget.Get());
		if (BaseButton)
		{
			BaseButton->SetIsToggleable(SelectionMode == ESelectionMode::SingleToggle || SelectionMode == ESelectionMode::Multi);
		}
	}
}

int32 UCommonListView::GetNumItemsSelected() const
{
	return MyListView.IsValid() ? MyListView->GetNumItemsSelected() : 0;
}

UObject* UCommonListView::GetSelectedItem() const
{
	if (MyListView.IsValid())
	{
		TArray<TWeakObjectPtr<UObject>> SelectedItems = MyListView->GetSelectedItems();
		return SelectedItems.Num() > 0 ? SelectedItems[0].Get() : nullptr;
	}

	return nullptr;
}

bool UCommonListView::GetSelectedItems(TArray<UObject*>& Items) const
{
	if (MyListView.IsValid())
	{
		const TArray<TWeakObjectPtr<UObject>>& WeakSelectedItems = MyListView->GetSelectedItems();
		Items.Empty(WeakSelectedItems.Num());
		Algo::Transform(WeakSelectedItems, Items, [](TWeakObjectPtr<UObject> WeakItem) -> UObject* { return WeakItem.Get(); });
		return true;
	}

	return false;
}

TArray<TWeakObjectPtr<UObject>> UCommonListView::GetAllItems() const
{
	return DataProvider->AsArray();
}

bool UCommonListView::IsItemVisible(UObject* Item) const
{
	return MyListView.IsValid() ? MyListView->IsItemVisible(Item) : false;
}

void UCommonListView::ScrollIntoView( UObject* Item )
{
	if ( MyListView.IsValid() )
	{
		MyListView->RequestScrollIntoView( Item );
	}
}

bool UCommonListView::SetSelectedItem(UObject* Item, bool bWaitIfPendingRefresh)
{
	if (MyListView.IsValid() && DataProvider->AsArray().Contains(Item))
	{
		if (bWaitIfPendingRefresh)
		{
			// Take the slow route to make sure we wait for any pending refresh to complete
			ItemToSelectAfterRefresh = Item;
			SelectItemAfterRefresh();
		}
		else
		{
			// Just go for it
			SetSelectedItemInternal(Item);
		}
		
		return true;
	}

	return false;
}

bool UCommonListView::SetSelectedItem(const UObject* Item, bool bWaitIfPendingRefresh)
{
	return SetSelectedItem(const_cast<UObject*>(Item), bWaitIfPendingRefresh);
}

bool UCommonListView::SetSelectedIndex(int32 Index)
{
	return SetSelectedItem( GetItemAt(Index) );
}

void UCommonListView::SetItemSelection( UObject* Item, bool bSelected )
{
	if ( MyListView.IsValid() && DataProvider->AsArray().Contains( Item ) )
	{
		MyListView->SetItemSelection(Item, bSelected);
	}
}

void UCommonListView::ClearSelection()
{
	if ( MyListView.IsValid() )
	{
		MyListView->ClearSelection();
	}
}

void UCommonListView::Clear()
{
	if (DataProvider.IsValid())
	{
		DataProvider->Clear();
	}
}

bool UCommonListView::IsRefreshPending() const
{
	return MyListView.IsValid() ? MyListView->IsPendingRefresh() : false;
}

void UCommonListView::SetDataProvider(const TArray<UObject*>& InDataProvider)
{
	DataProvider->FromArray(InDataProvider);

	if (MyListView.IsValid())
	{
		MyListView->RequestListRefresh();
	}
}

#if WITH_EDITOR
const FText UCommonListView::GetPaletteCategory()
{
	return CommonWidgetPaletteCategories::Default;
}
#endif

TSharedRef<SListView<TWeakObjectPtr<UObject>>> UCommonListView::RebuildListWidget()
{
	MyListView = SNew(SListView<TWeakObjectPtr<UObject>>)
		.HandleGamepadEvents(false)
		.SelectionMode_UObject(this, &ThisClass::GetSelectionMode)
		.ListItemsSource(&DataProvider->AsArray())
		.ClearSelectionOnClick(bClearSelectionOnClick)
		.ItemHeight(GetTotalItemHeight())
		.ConsumeMouseWheel(ConsumeMouseWheel)
		.OnGenerateRow_UObject(this, &ThisClass::HandleGenerateRow)
		.OnMouseButtonClick_UObject(this, &ThisClass::HandleItemClicked)
		.OnSelectionChanged_UObject(this, &ThisClass::HandleSelectionChanged)
		.OnRowReleased_UObject(this, &ThisClass::HandleRowReleased);

	return MyListView.ToSharedRef();
}

TSharedRef<ITableRow> UCommonListView::HandleGenerateRow(TWeakObjectPtr<UObject> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	UUserWidget* Widget = ItemWidgetsFactory.Acquire();
	check(Widget);
	ItemWidgets.Add(Widget);

	// Make sure we take the widget first BEFORE setting data on it
	// This ensures that the widget has had a chance to construct before we set data on it
	TSharedRef<ITableRow> GeneratedRow = ItemWidgetsFactory.TakeAndCacheRow<ObjectRowType>(Widget, OwnerTable);

	if (!Widget->Implements<UCommonObjectListItem>())
	{
		UE_LOG(LogCommonUI, Warning, TEXT("[%s] does not implement CommonListItem interface, cannot set data."), *Widget->GetClass()->GetName());
	}
	else
	{
		Widget->SetPadding(DesiredItemPadding);
		ICommonObjectListItem::Execute_SetData(Widget, Item.Get());

		// If the list item is a base button, override the selectability of it
		// The list view is in charge of selection, and it's up to the widget to respond accordingly
		if (UCommonButton* BaseButton = Cast<UCommonButton>(Widget))
		{
			BaseButton->SetIsSelectable(false);
			BaseButton->SetIsToggleable(SelectionMode == ESelectionMode::SingleToggle || SelectionMode == ESelectionMode::Multi);
			BaseButton->OnButtonClicked.AddDynamic(this, &ThisClass::DynamicHandleItemClickedCommonButton);
		}
		else
		{
			FOnItemClicked OnClicked;
			OnClicked.BindDynamic(this, &ThisClass::DynamicHandleItemClickedUserWidget);
			ICommonObjectListItem::Execute_RegisterOnClicked(Widget, OnClicked);
		}
	}

	OnItemWidgetCreated.Broadcast(Widget);

	return GeneratedRow;
}

void UCommonListView::HandleItemClicked(TWeakObjectPtr<UObject> Item)
{
	OnItemClicked.Broadcast(Item.Get());
}

void UCommonListView::HandleSelectionChanged(TWeakObjectPtr<UObject> Item, ESelectInfo::Type SelectInfo)
{
	const bool bItemSelected = MyListView.IsValid() ? MyListView->Private_IsItemSelected(Item) : false;
	OnItemSelected.Broadcast(Item.Get(), bItemSelected);
}

void UCommonListView::HandleRowReleased(const TSharedRef<ITableRow>& Row)
{
	// Get the actual UserWidget from the released row
	const TSharedRef<ObjectRowType>& ObjectRow = StaticCastSharedRef<ObjectRowType>(Row);
	UUserWidget* ItemWidget = Cast<UUserWidget>(ObjectRow->GetWidgetObject());

	if (ensure(ItemWidget->Implements<UCommonObjectListItem>()))
	{
		ICommonObjectListItem::Execute_Reset(ItemWidget);
	}

	// If the list item is a base button, override the selectability of it
	// The list view is in charge of selection, and it's up to the widget to respond accordingly
	if (UCommonButton* BaseButton = Cast<UCommonButton>(ItemWidget))
	{
		BaseButton->OnButtonClicked.RemoveDynamic(this, &ThisClass::DynamicHandleItemClickedCommonButton);
	}

	ItemWidgets.RemoveSwap(ItemWidget);
	ItemWidgetsFactory.Release(ItemWidget);
}

float UCommonListView::GetTotalItemHeight() const
{
	return ItemHeight + DesiredItemPadding.Bottom + DesiredItemPadding.Top;
}

void UCommonListView::DynamicHandleItemClickedCommonButton(UCommonButton* Button)
{
	DynamicHandleItemClickedUserWidget(Button);
}

void UCommonListView::DynamicHandleItemClickedUserWidget(UUserWidget* Widget)
{
	if (Widget && ensure(Widget->Implements<UCommonObjectListItem>()))
	{
		UObject* Data = ICommonObjectListItem::Execute_GetData(Widget);

		const bool bWasItemAlreadySelected = MyListView->Private_IsItemSelected(Data);
		if (SelectionMode == ESelectionMode::Single || SelectionMode == ESelectionMode::SingleToggle)
		{
			MyListView->ClearSelection();
			if (SelectionMode == ESelectionMode::Single || !bWasItemAlreadySelected)
			{
				MyListView->SetItemSelection(Data, !bWasItemAlreadySelected);
			}
		}
		else if (SelectionMode == ESelectionMode::Multi)
		{
			MyListView->SetItemSelection(Data, !bWasItemAlreadySelected);
		}

		HandleItemClicked(Data);
	}
}

void UCommonListView::SelectItemAfterRefresh()
{
	if (MyListView.IsValid() && ItemToSelectAfterRefresh.IsValid())
	{
		if (MyListView->IsPendingRefresh())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimerForNextTick(this, &UCommonListView::SelectItemAfterRefresh);
			}
		}
		else
		{
			SetSelectedItemInternal(ItemToSelectAfterRefresh.Get());
		}
	}
	else
	{
		ItemToSelectAfterRefresh.Reset();
	}
}

void UCommonListView::SetSelectedItemInternal(UObject* Item)
{
	check(MyListView.IsValid());
	MyListView->SetSelection(Item, ESelectInfo::OnNavigation);
	MyListView->RequestScrollIntoView(Item);

	ItemToSelectAfterRefresh.Reset();
}

