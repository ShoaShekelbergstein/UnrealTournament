// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonLoadGuard.h"
#include "CommonTextBlock.h"
#include "CommonWidgetPaletteCategories.h"
#include "Components/SizeBoxSlot.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"

UCommonLoadGuard::UCommonLoadGuard(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, ThrobberAlignment(EHorizontalAlignment::HAlign_Center)
	, LoadingText(NSLOCTEXT("LoadGuard", "Loading", "Loading..."))
	, bIsLoading(false)
{
	if (TextStyleClass.IsValid())
	{
		static ConstructorHelpers::FClassFinder<UCommonTextStyle> BaseTextStyleClassFinder(*TextStyleClass.GetLongPackageName());
		TextStyle = BaseTextStyleClassFinder.Class;
	}

#if WITH_EDITORONLY_DATA
	bShowLoading = false;
#endif
}

void UCommonLoadGuard::SetLoadingText(const FText& InLoadingText)
{
	LoadingText = InLoadingText;
	RefreshText();
}

void UCommonLoadGuard::SetIsLoading(bool bInIsLoading)
{
	SetIsLoadingInternal(bInIsLoading, false);
}

bool UCommonLoadGuard::IsLoading() const
{
	return bIsLoading;
}

void UCommonLoadGuard::BP_GuardAndLoadAsset(const TAssetPtr<UObject>& InLazyAsset, const FOnAssetLoaded& OnAssetLoaded)
{
	GuardAndLoadAsset<UObject>(InLazyAsset, 
		[OnAssetLoaded] (UObject* Asset) 
		{
			OnAssetLoaded.ExecuteIfBound(Asset); 
		});
}

void UCommonLoadGuard::SetContent(const TSharedRef<SWidget>& Content)
{
	if (MyContentBox.IsValid())
	{
		MyContentBox->SetContent(Content);
	}
}

TSharedRef<SWidget> UCommonLoadGuard::RebuildWidget()
{
	MyContentBox = SNew(SBox);
	if (GetChildrenCount() > 0)
	{
		Cast<USizeBoxSlot>(GetContentSlot())->BuildSlot(MyContentBox.ToSharedRef());
	}
	
	Text_LoadingText = NewObject<UCommonTextBlock>(this);
	RefreshText();

	MyGuardOverlay = 
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MyContentBox.ToSharedRef()
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MyGuardBox, SBox)
			.HAlign(ThrobberAlignment.GetValue())
			.VAlign(VAlign_Center)
			.Padding(ThrobberPadding)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, Text_LoadingText ? 18.f : 0.f, 0.f)
				[
					SNew(SImage)
					//.Image(&UCommonGameUIData::Get().LoadingSpinner)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					Text_LoadingText ? Text_LoadingText->TakeWidget() : SNullWidget::NullWidget
				]
			]
		];

	SetIsLoadingInternal(bIsLoading);

	return BuildDesignTimeWidget(MyGuardOverlay.ToSharedRef());
}

void UCommonLoadGuard::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	RefreshText();

#if WITH_EDITORONLY_DATA
	SetIsLoadingInternal(bShowLoading);
#endif
}

UClass* UCommonLoadGuard::GetSlotClass() const
{
	return USizeBoxSlot::StaticClass();
}

void UCommonLoadGuard::OnSlotAdded(UPanelSlot* AddedSlot)
{
	if (MyContentBox.IsValid())
	{
		MyContentBox->SetContent(SNullWidget::NullWidget);
	}
}

void UCommonLoadGuard::OnSlotRemoved(UPanelSlot* RemovedSlot)
{
	if (MyContentBox.IsValid())
	{
		Cast<USizeBoxSlot>(RemovedSlot)->BuildSlot(MyContentBox.ToSharedRef());
	}
}

#if WITH_EDITOR
const FText UCommonLoadGuard::GetPaletteCategory()
{
	return CommonWidgetPaletteCategories::Default;
}
#endif

void UCommonLoadGuard::SetIsLoadingInternal(bool bInIsLoading, bool bForce)
{
	if (bForce || (bInIsLoading != bIsLoading))
	{
		bIsLoading = bInIsLoading;

		if (OnLoadingStateChanged.IsBound())
		{
			OnLoadingStateChanged.Broadcast(bInIsLoading);
		}

		if (MyGuardOverlay.IsValid())
		{
			MyGuardOverlay->SetVisibility(bIsLoading ? EVisibility::HitTestInvisible : EVisibility::SelfHitTestInvisible);
		}
		if (MyGuardBox.IsValid())
		{
			MyGuardBox->SetVisibility(bIsLoading ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		}

		if (MyContentBox.IsValid())
		{
			MyContentBox->SetVisibility(bIsLoading ? EVisibility::Hidden : EVisibility::SelfHitTestInvisible);
		}
	}
}

void UCommonLoadGuard::RefreshText()
{
	if (Text_LoadingText)
	{
		if (LoadingText.IsEmpty())
		{
			Text_LoadingText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Text_LoadingText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Text_LoadingText->SetProperties(TextStyle);
			Text_LoadingText->SetText(LoadingText);
		}
	}
}

void UCommonLoadGuard::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	LazyAsset.Reset();

	MyGuardOverlay.Reset();
	MyGuardBox.Reset();
	MyContentBox.Reset();

	if (Text_LoadingText)
	{
		Text_LoadingText->ReleaseSlateResources(bReleaseChildren);
		Text_LoadingText = nullptr;
	}
}