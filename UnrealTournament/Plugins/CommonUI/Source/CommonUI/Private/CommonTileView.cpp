// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonTileView.h"
#include "CommonObjectListItem.h"

UCommonTileView::UCommonTileView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemAlignment(EItemAlignment::EvenlyDistributed)
	, ItemWidth(128.f)
{
}

void UCommonTileView::ReleaseSlateResources( bool bReleaseChildren )
{
	Super::ReleaseSlateResources( bReleaseChildren );

	MyTileView.Reset();
}

void UCommonTileView::BeginDestroy()
{
	Super::BeginDestroy();

	if ( MyTileView.IsValid() )
	{
		MyTileView.Reset();
	}
}

void UCommonTileView::SetItemWidth(float NewWidth)
{
	ItemWidth = NewWidth;
	if (MyTileView.IsValid())
	{
		MyTileView->SetItemWidth(GetTotalItemWidth());
	}
}

TSharedRef<SListView<TWeakObjectPtr<UObject>>> UCommonTileView::RebuildListWidget()
{
	MyListView = MyTileView = SNew(STileView<TWeakObjectPtr<UObject>>)
		.HandleGamepadEvents(false)
		.SelectionMode(SelectionMode)
		.ListItemsSource(&DataProvider->AsArray())
		.ClearSelectionOnClick(bClearSelectionOnClick)
		.ItemWidth(GetTotalItemWidth())
		.ItemHeight(GetTotalItemHeight())
		.ConsumeMouseWheel(ConsumeMouseWheel)
		.ItemAlignment(AsListItemAlignment(ItemAlignment))
		.OnGenerateTile_UObject(this, &ThisClass::HandleGenerateRow)
		.OnMouseButtonClick_UObject(this, &ThisClass::HandleItemClicked)
		.OnSelectionChanged_UObject(this, &ThisClass::HandleSelectionChanged)
		.OnTileReleased_UObject(this, &ThisClass::HandleRowReleased);

	return MyTileView.ToSharedRef();
}

float UCommonTileView::GetTotalItemWidth() const
{
	return ItemWidth + DesiredItemPadding.Left + DesiredItemPadding.Right;
}
