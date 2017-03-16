// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonWidgetCarousel.h"
#include "CommonWidgetPaletteCategories.h"
#include "Containers/Ticker.h"

UCommonWidgetCarousel::UCommonWidgetCarousel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;
}

void UCommonWidgetCarousel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyCommonWidgetCarousel.Reset();
	MyCarouselNavBar.Reset();
	CachedSlotWidgets.Empty();

	EndAutoScrolling();
}

void UCommonWidgetCarousel::BeginAutoScrolling(float ScrollInterval)
{
	EndAutoScrolling();
	TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UCommonWidgetCarousel::AutoScrollCallback), ScrollInterval);
}

void UCommonWidgetCarousel::EndAutoScrolling()
{
	if ( TickerHandle.IsValid() )
	{
		FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}

bool UCommonWidgetCarousel::AutoScrollCallback(float DeltaTime)
{
	if ( MyCommonWidgetCarousel.IsValid() )
	{
		MyCommonWidgetCarousel->SetNextWidget();
	}

	return true;
}

void UCommonWidgetCarousel::NextPage()
{
	if ( MyCommonWidgetCarousel.IsValid() && Slots.Num() > 1 )
	{
		MyCommonWidgetCarousel->SetPreviousWidget();
	}
}

void UCommonWidgetCarousel::PreviousPage()
{
	if ( MyCommonWidgetCarousel.IsValid() && Slots.Num() > 1 )
	{
		MyCommonWidgetCarousel->SetNextWidget();
	}
}

int32 UCommonWidgetCarousel::GetActiveWidgetIndex() const
{
	if ( MyCommonWidgetCarousel.IsValid() )
	{
		return MyCommonWidgetCarousel->GetWidgetIndex();
	}

	return ActiveWidgetIndex;
}

void UCommonWidgetCarousel::SetActiveWidgetIndex(int32 Index)
{
	ActiveWidgetIndex = Index;
	if ( MyCommonWidgetCarousel.IsValid() )
	{
		// Ensure the index is clamped to a valid range.
		int32 SafeIndex = FMath::Clamp(ActiveWidgetIndex, 0, FMath::Max(0, Slots.Num() - 1));
		MyCommonWidgetCarousel->SetActiveWidgetIndex(SafeIndex);
	}
}

void UCommonWidgetCarousel::SetActiveWidget(UWidget* Widget)
{
	ActiveWidgetIndex = GetChildIndex(Widget);
	if ( MyCommonWidgetCarousel.IsValid() )
	{
		// Ensure the index is clamped to a valid range.
		int32 SafeIndex = FMath::Clamp(ActiveWidgetIndex, 0, FMath::Max(0, Slots.Num() - 1));
		MyCommonWidgetCarousel->SetActiveWidgetIndex(SafeIndex);
	}
}

UWidget* UCommonWidgetCarousel::GetWidgetAtIndex( int32 Index ) const
{
	if ( Slots.IsValidIndex( Index ) )
	{
		return Slots[ Index ]->Content;
	}

	return nullptr;
}

UClass* UCommonWidgetCarousel::GetSlotClass() const
{
	return UPanelSlot::StaticClass();
}

void UCommonWidgetCarousel::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
}

void UCommonWidgetCarousel::OnSlotRemoved(UPanelSlot* InSlot)
{
	Super::OnSlotRemoved(InSlot);
}

TSharedRef<SWidget> UCommonWidgetCarousel::RebuildWidget()
{
	MyCommonWidgetCarousel = SNew(SWidgetCarousel<UPanelSlot*>)
		.WidgetItemsSource(&Slots)
		.FadeRate(0)
		.SlideValueLeftLimit(-1)
		.SlideValueRightLimit(1)
		.MoveSpeed(5.f)
		.OnGenerateWidget_UObject(this, &UCommonWidgetCarousel::OnGenerateWidgetForCarousel)
		.OnPageChanged_UObject(this, &UCommonWidgetCarousel::HandlePageChanged);

	for (UPanelSlot* PanelSlot : Slots)
	{
		PanelSlot->Parent = this;
		if (PanelSlot->Content)
		{
			CachedSlotWidgets.AddUnique(PanelSlot->Content->TakeWidget());
		}
	}

	MyCarouselNavBar = SNew(SCarouselNavigationBar)
		.Style(&NavigationStyle)
		.ItemCount(Slots.Num())
		.Visibility(Slots.Num() > 1 ? EVisibility::Visible : EVisibility::Collapsed);

	return SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MyCommonWidgetCarousel.ToSharedRef()
		]
	
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			SNullWidget::NullWidget
			//MyCarouselNavBar.ToSharedRef()
		];
}

TSharedRef<SWidget> UCommonWidgetCarousel::OnGenerateWidgetForCarousel(UPanelSlot* PanelSlot)
{
	if ( UWidget* Content = PanelSlot->Content )
	{
		return Content->TakeWidget();
	}

	return SNullWidget::NullWidget;
}

void UCommonWidgetCarousel::HandlePageChanged(int32 PageIndex)
{
	OnCurrentPageIndexChanged.Broadcast(this, PageIndex);
}

void UCommonWidgetCarousel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	SetActiveWidgetIndex(ActiveWidgetIndex);
}

#if WITH_EDITOR

const FText UCommonWidgetCarousel::GetPaletteCategory()
{
	return CommonWidgetPaletteCategories::Default;
}

void UCommonWidgetCarousel::OnDescendantSelected(UWidget* DescendantWidget)
{
	// Temporarily sets the active child to the selected child to make
	// dragging and dropping easier in the editor.
	UWidget* SelectedChild = UWidget::FindChildContainingDescendant(this, DescendantWidget);
	if ( SelectedChild )
	{
		int32 OverrideIndex = GetChildIndex(SelectedChild);
		if ( OverrideIndex != -1 && MyCommonWidgetCarousel.IsValid() )
		{
			MyCommonWidgetCarousel->SetActiveWidgetIndex(OverrideIndex);
		}
	}
}

void UCommonWidgetCarousel::OnDescendantDeselected(UWidget* DescendantWidget)
{
	SetActiveWidgetIndex(ActiveWidgetIndex);
}

void UCommonWidgetCarousel::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	ActiveWidgetIndex = FMath::Clamp(ActiveWidgetIndex, 0, FMath::Max(0, Slots.Num() - 1));

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif