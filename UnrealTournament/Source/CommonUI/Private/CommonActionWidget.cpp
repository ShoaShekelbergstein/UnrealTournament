// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonActionWidget.h"
#include "CommonUIContext.h"
#include "IBlueprintContextModule.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"

UCommonActionWidget::UCommonActionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

TSharedRef<SWidget> UCommonActionWidget::RebuildWidget()
{
	MyKeyBox = SNew(SBox);
	return MyKeyBox.ToSharedRef();
}

void UCommonActionWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	MyKeyBox.Reset();
	ListenToInputMethodChanged(false);
	Super::ReleaseSlateResources(bReleaseChildren);
}

void UCommonActionWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyKeyBox.IsValid() && IsDesignTime())
	{
		UpdateActionWidget();
	}
}

FSlateBrush UCommonActionWidget::GetIcon() const
{
	return GetInputActionIcon(InputActionDataRow, GetCurrentInputType());
}

FText UCommonActionWidget::GetDisplayText() const
{
	const FCommonInputActionData* InputActionData = GetInputActionData(InputActionDataRow);
	if (InputActionData)
	{
		return InputActionData->DisplayName;
	}
	return FText();
}

bool UCommonActionWidget::IsHeldAction() const
{
	const FCommonInputActionData* InputActionData = GetInputActionData(InputActionDataRow);
	if (InputActionData)
	{
		return InputActionData->bActionRequiresHold;
	}
	return false;
}

void UCommonActionWidget::SetInputAction(FDataTableRowHandle InputActionRow)
{
	InputActionDataRow = InputActionRow;
	UpdateActionWidget();
}

void UCommonActionWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UpdateActionWidget();
	ListenToInputMethodChanged();
}

ECommonInputType UCommonActionWidget::GetCurrentInputType() const
{
	ECommonInputType InputType = ECommonInputType::MouseAndKeyboard;
	bool bNotDesignTime = !IsDesignTime();
	UWorld* World = GetWorld();
	// TODO: Find out why context is null!
	UCommonUIContext* CommonUIContext = CommonUIUtils::GetContext<UCommonUIContext>(this);
	if (bNotDesignTime && World && CommonUIContext)
	{
		if (CommonUIContext->IsUsingGamepad())
		{
#if PLATFORM_PS4
			InputType = ECommonInputType::PS4Controller;
#else
			InputType = ECommonInputType::XboxOneController;
#endif
		}
	}
	return InputType;
}

void UCommonActionWidget::UpdateActionWidget()
{
	if (!IsDesignTime() && GetWorld())
	{
		if (InputActionDataRow.IsNull())
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			ECommonInputType InputType = GetCurrentInputType();
			Icon = GetInputActionIcon(InputActionDataRow, InputType);
			if (Icon.DrawAs == ESlateBrushDrawType::NoDrawType)
			{
				SetVisibility(ESlateVisibility::Collapsed);
			}
			else if (MyKeyBox.IsValid())
			{
				TSharedPtr<SImage> IconImage = SNew(SImage)
					.Image(&Icon);
				MyKeyBox->SetContent(IconImage.ToSharedRef());

				// Should be done when we set content but needed since no invalidate is
				// currently happening
				MyKeyBox->Invalidate(EInvalidateWidget::LayoutAndVolatility);

				SetVisibility(ESlateVisibility::SelfHitTestInvisible);			
			}
		}
	}
}

void UCommonActionWidget::ListenToInputMethodChanged(bool bListen)
{
	bool bNotDesignTime = !IsDesignTime();
	UWorld* World = GetWorld();

	// TODO: Find out why context is null!
	UCommonUIContext* CommonUIContext = CommonUIUtils::GetContext<UCommonUIContext>(this);
	if (bNotDesignTime && World && CommonUIContext)
	{
		CommonUIContext->OnInputMethodChangedNative.RemoveAll(this);
		if (bListen)
		{
			CommonUIContext->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		}
	}
}

void UCommonActionWidget::HandleInputMethodChanged(bool bUsingGamepad)
{
	UpdateActionWidget();
	OnInputMethodChanged.Broadcast(bUsingGamepad);
}