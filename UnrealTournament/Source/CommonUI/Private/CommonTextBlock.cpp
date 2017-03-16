// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CommonUIPrivatePCH.h"
#include "CommonTextBlock.h"
#include "Containers/Ticker.h"
#include "CommonUISettings.h"
#include "Widgets/Text/STextBlock.h"

UCommonTextStyle::UCommonTextStyle()
	: Color(FLinearColor::Black)
	, LineHeightPercentage(1.0f)
{
}

void UCommonTextStyle::GetFont(FSlateFontInfo& OutFont) const
{
	OutFont = Font;
}

void UCommonTextStyle::GetColor(FLinearColor& OutColor) const
{
	OutColor = Color;
}

void UCommonTextStyle::GetMargin(FMargin& OutMargin) const
{
	OutMargin = Margin;
}

float UCommonTextStyle::GetLineHeightPercentage() const
{
	return LineHeightPercentage;
}

void UCommonTextStyle::GetShadowOffset(FVector2D& OutShadowOffset) const
{
	OutShadowOffset = ShadowOffset;
}

void UCommonTextStyle::GetShadowColor(FLinearColor& OutColor) const
{
	OutColor = ShadowColor;
}

void UCommonTextStyle::ToTextBlockStyle(FTextBlockStyle& OutTextBlockStyle)
{
	OutTextBlockStyle.Font = Font;
	OutTextBlockStyle.ColorAndOpacity = Color;
}

UCommonTextBlock::UCommonTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ActiveState(EStart)
	, TimeElapsed(0.f)
	, ScrollOffset(0.f)
	, FontAlpha(1.f)
	, bPlaying(true)
{
	Style = GetDefault<UCommonUISettings>()->DefaultTextStyle;

	Visiblity_DEPRECATED = Visibility = ESlateVisibility::SelfHitTestInvisible;
}

void UCommonTextBlock::SetWrapTextWidth(int32 InWrapTextAt)
{
	WrapTextAt = InWrapTextAt;
	SynchronizeProperties();
}

void UCommonTextBlock::SetStyle(TSubclassOf<UCommonTextStyle> InStyle)
{
	Style = InStyle;
	SynchronizeProperties();
}

void UCommonTextBlock::SetScrollStyle(TSubclassOf<UCommonTextScrollStyle> InScrollStyle)
{
	ScrollStyle = InScrollStyle;
	SynchronizeProperties();
}

void UCommonTextBlock::SetProperties(TSubclassOf<UCommonTextStyle> InStyle, TSubclassOf<UCommonTextScrollStyle> InScrollStyle)
{
	Style          = InStyle;
	ScrollStyle	   = InScrollStyle;
	SynchronizeProperties();
}

void UCommonTextBlock::SynchronizeProperties()
{
	if (const UCommonTextStyle* TextStyle = GetStyleCDO())
	{
		// Update our styling to match the assigned style
		Font                 = TextStyle->Font;
		Margin               = TextStyle->Margin;
		LineHeightPercentage = TextStyle->LineHeightPercentage;

		ColorAndOpacity      = TextStyle->Color;

		if (TextStyle->bUsesDropShadow)
		{
			ShadowOffset          = TextStyle->ShadowOffset;
			ShadowColorAndOpacity = TextStyle->ShadowColor;
		}
		else
		{
			ShadowOffset          = FVector2D::ZeroVector;
			ShadowColorAndOpacity = FLinearColor::Transparent;
		}
	}

	Super::SynchronizeProperties();
}

TSharedRef<SWidget> UCommonTextBlock::RebuildWidget()
{
	const UCommonTextScrollStyle* TextScrollStyle = UCommonTextBlock::GetScrollStyleCDO();
	if (!TextScrollStyle)
	{
		return Super::RebuildWidget();
	}

	// clang-format off
	ScrollBar = SNew(SScrollBar);
	ScrollBox = SNew(SScrollBox)
	                .ScrollBarVisibility(EVisibility::Collapsed)
	                .Orientation(Orient_Horizontal)
	                .ExternalScrollbar(ScrollBar)
					+ SScrollBox::Slot()
					[ 
						Super::RebuildWidget() 
					];
	// clang-format on

	TickHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::OnTick));
	return BuildDesignTimeWidget(ScrollBox.ToSharedRef());
}

const UCommonTextStyle* UCommonTextBlock::GetStyleCDO() const
{
	if ( Style )
	{
		if (const UCommonTextStyle* TextStyle = Cast<UCommonTextStyle>(Style->ClassDefaultObject))
		{
			return TextStyle;
		}
	}
	return nullptr;
}

const UCommonTextScrollStyle* UCommonTextBlock::GetScrollStyleCDO() const
{
	if (ScrollStyle)
	{
		if (const UCommonTextScrollStyle* TextScrollStyle = Cast<UCommonTextScrollStyle>(ScrollStyle->ClassDefaultObject))
		{
			return TextScrollStyle;
		}
	}
	return nullptr;
}

bool UCommonTextBlock::OnTick( float Delta )
{
	if ( !bPlaying )
	{
		return true;
	}

	const UCommonTextScrollStyle* TextScrollStyle = UCommonTextBlock::GetScrollStyleCDO();
	check(TextScrollStyle);

	const float ContentSize = ScrollBox->GetDesiredSize().X;
	TimeElapsed += Delta;

	switch (ActiveState)
	{
		case EFadeIn:
		{
			if (!ScrollBar->IsNeeded())
			{
				FontAlpha = 1.f;
				break;
			}
			else
			{
				FontAlpha = FMath::Clamp<float>(TimeElapsed / TextScrollStyle->FadeInDelay, 0.f, 1.f);
				if (TimeElapsed >= TextScrollStyle->FadeInDelay)
				{
					FontAlpha = 1.f;
					TimeElapsed = 0.f;
					ScrollOffset = 0.f;
					ActiveState = EStart;
				}
			}
			break;
		}
		case EStart:
		{
			if (!ScrollBar->IsNeeded())
			{
				break;
			}
			else
			{
				TimeElapsed = 0.f;
				ScrollOffset = 0.f;
				ActiveState = EStartWait;
			}
			break;
		}
		case EStartWait: 
		{
			if (TimeElapsed >= TextScrollStyle->StartDelay)
			{
				TimeElapsed = 0.f;
				ScrollOffset = 0.f;
				ActiveState = EScrolling;
			}
			break;
		}
		case EScrolling:
		{
			ScrollOffset += TextScrollStyle->Speed * Delta;
			if (FMath::IsNearlyZero(ScrollBar->DistanceFromBottom()))
			{
				ScrollOffset = ContentSize;
				TimeElapsed  = 0.0f;
				ActiveState  = EStop;
			}
			break;
		}
		case EStop: 
		{
			if (!ScrollBar->IsNeeded())
			{
				break;
			}
			else
			{
				TimeElapsed = 0.f;
				ActiveState = EStopWait;
			}
			break;
		}
		case EStopWait: 
		{
			if (TimeElapsed >= TextScrollStyle->EndDelay)
			{
				TimeElapsed = 0.f;
				ActiveState = EFadeOut;
			}
			break;
		}
		case EFadeOut: 
		{
			if (!ScrollBar->IsNeeded())
			{
				FontAlpha = 1.f;
				break;
			}
			else
			{
				FontAlpha = 1.0f - FMath::Clamp<float>(TimeElapsed / TextScrollStyle->FadeOutDelay, 0.f, 1.f);
				if (TimeElapsed >= TextScrollStyle->FadeOutDelay)
				{
					FontAlpha = 0.0f;
					TimeElapsed = 0.0f;
					ScrollOffset = 0.0f;
					ActiveState = EFadeIn;
				}
			}
			break;
		}
	}

	if (ScrollBox.IsValid())
	{
		ScrollBox->SetScrollOffset(ScrollOffset);
	}

	if (MyTextBlock.IsValid())
	{
		const FLinearColor& Color = ColorAndOpacity.GetSpecifiedColor();
		MyTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(Color.R, Color.G, Color.B, Color.A * FontAlpha)));
	}

	return true;
}
