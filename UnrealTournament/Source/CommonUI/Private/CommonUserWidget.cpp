// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonUserWidget.h"

UCommonUserWidget::UCommonUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	// 99% of game user widgets do not support keyboard focus
	// We turn off focus by default and let widgets decide this on a case by case 
	// basis.
	bIsFocusable = false;

#if WITH_EDITORONLY_DATA
	PaletteCategory = FText::FromString(TEXT("Common UI"));
#endif
}

void UCommonUserWidget::NativePreConstruct()
{
	bool bDesignTime;
#if WITH_EDITOR
	bDesignTime = IsDesignTime();
#else
	bDesignTime = false;
#endif
	PreConstruct(bDesignTime);
}

void UCommonUserWidget::OnWidgetRebuilt()
{
	NativePreConstruct();

	// this is expected to call construct in non-design cases
	Super::OnWidgetRebuilt();
}

FReply UCommonUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UCommonUserWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UCommonUserWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply UCommonUserWidget::NativeOnTouchGesture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnTouchGesture(InGeometry, InGestureEvent);
}

FReply UCommonUserWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply UCommonUserWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply UCommonUserWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

FReply UCommonUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return bConsumePointerInput ? FReply::Handled() : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}