// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTRadialMenu_BoostPowerup.h"
#include "UTFlagRunPvEHUD.h"

UUTRadialMenu_BoostPowerup::UUTRadialMenu_BoostPowerup(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> MenuAtlas(TEXT("Texture2D'/Game/RestrictedAssets/UI/HUDAtlas04.HUDAtlas04'"));
	InnerCircleTemplate.QuickSet(MenuAtlas.Object, FTextureUVs(0.0f, 206.0f, 236.0f, 236.0f));

	SegmentTemplate.QuickSet(MenuAtlas.Object, FTextureUVs(275.0f, 200.0f, 189.0f, 113.0f));
	HighlightedSegmentTemplate.QuickSet(MenuAtlas.Object, FTextureUVs(275.0f, 319.0f, 189.0f, 113.0f));

	static ConstructorHelpers::FObjectFinder<UTexture2D> WeaponAtlas(TEXT("Texture2D'/Game/RestrictedAssets/UI/WeaponAtlas01.WeaponAtlas01'"));
	WeaponIconTemplate.Atlas = WeaponAtlas.Object;

	CaptionTemplate.TextScale = 1.0f;
}

void UUTRadialMenu_BoostPowerup::InitializeWidget(AUTHUD* Hud)
{
	Super::InitializeWidget(Hud);
	AutoLayout(8);
}

bool UUTRadialMenu_BoostPowerup::ShouldDraw_Implementation(bool bShowScores)
{
	AUTFlagRunPvEHUD* HUD = Cast<AUTFlagRunPvEHUD>(UTHUDOwner);
	return HUD != nullptr && HUD->bShowBoostWheel;
}

void UUTRadialMenu_BoostPowerup::Draw_Implementation(float RenderDelta)
{
	FVector2D ScreenCenter = FVector2D(Canvas->ClipX * 0.5, Canvas->ClipY * 0.8);

	if (BounceTimer > 0.0f)
	{
		BounceTimer -= RenderDelta;
	}

	DrawMenu(ScreenCenter, RenderDelta);
}

void UUTRadialMenu_BoostPowerup::DrawMenu(FVector2D ScreenCenter, float RenderDelta)
{
	AUTGameState* GS = UTHUDOwner->GetWorld()->GetGameState<AUTGameState>();
	if (GS != nullptr && UTPlayerOwner != nullptr && UTPlayerOwner->UTPlayerState != nullptr)
	{
		const FVector2D CenterPoint = FVector2D(0.0f, -250.0f);
		Opacity = UTPlayerOwner->UTPlayerState->GetRemainingBoosts() > 0 ? 1.0f : 0.33f;
		CaptionTemplate.Text = FText();
		TArray<TSubclassOf<AUTInventory>> OffsetItems;
		{
			TSubclassOf<AUTInventory> NextItem;
			int i;
			for (i = 0, NextItem = GS->GetSelectableBoostByIndex(UTPlayerOwner->UTPlayerState, i); NextItem != nullptr; i++, NextItem = GS->GetSelectableBoostByIndex(UTPlayerOwner->UTPlayerState, i))
			{
				OffsetItems.Add(NextItem);
			}
			if (OffsetItems.Num() > 1)
			{
				for (i = 0; i < SelectedPowerup; i++)
				{
					TSubclassOf<AUTInventory> FirstItem = OffsetItems[0];
					for (int32 j = 1; j < OffsetItems.Num(); j++)
					{
						OffsetItems[j - 1] = OffsetItems[j];
					}
					OffsetItems.Last() = FirstItem;
				}
			}
		}
		for (int32 i = FMath::Min<int32>(OffsetItems.Num() - 1, 1); i >= 0; i--)
		{
			TSubclassOf<AUTInventory> NextItem = OffsetItems[i];

			const bool bCurrent = i == 0;

			float Angle = GetMidSegmentAngle(i);
			FVector2D DrawScreenPosition = Rotate(CenterPoint, Angle) + FVector2D(0.0f, 400.0f);
			SegmentTemplate.RenderScale = bCurrent ? 1.2f : 0.6f; 
			RenderObj_TextureAtWithRotation(SegmentTemplate, DrawScreenPosition, Angle);
			if (bCurrent)
			{
				HighlightedSegmentTemplate.RenderScale = bCurrent ? 1.2f : 0.6f; 
				RenderObj_TextureAtWithRotation(HighlightedSegmentTemplate, DrawScreenPosition, Angle);
			}

			FVector2D IconRenderPosition = DrawScreenPosition;
			WeaponIconTemplate.Atlas = NextItem.GetDefaultObject()->HUDIcon.Texture;
			WeaponIconTemplate.UVs.U = NextItem.GetDefaultObject()->HUDIcon.U;
			WeaponIconTemplate.UVs.UL = NextItem.GetDefaultObject()->HUDIcon.UL;
			WeaponIconTemplate.UVs.V = NextItem.GetDefaultObject()->HUDIcon.V;
			WeaponIconTemplate.UVs.VL = NextItem.GetDefaultObject()->HUDIcon.VL;
			WeaponIconTemplate.RenderOffset = FVector2D(0.5f,0.5f);
			WeaponIconTemplate.RenderScale = bCurrent ? 1.0f : 0.75f;

			// Draw it in black a little bigger
			WeaponIconTemplate.RenderColor = FLinearColor::Black;
			RenderObj_TextureAt(WeaponIconTemplate, IconRenderPosition.X, IconRenderPosition.Y, WeaponIconTemplate.UVs.UL * WeaponIconTemplate.RenderScale * 1.55f, WeaponIconTemplate.UVs.VL * WeaponIconTemplate.RenderScale * 1.05f);
			
			// Draw it colorized
			WeaponIconTemplate.RenderColor = NextItem.GetDefaultObject()->IconColor;
			RenderObj_TextureAt(WeaponIconTemplate, IconRenderPosition.X, IconRenderPosition.Y, WeaponIconTemplate.UVs.UL * WeaponIconTemplate.RenderScale * 1.5f, WeaponIconTemplate.UVs.VL * WeaponIconTemplate.RenderScale * 1.0f);

			if (i == 0)
			{
				CaptionTemplate.Text = NextItem.GetDefaultObject()->DisplayName;
			}
		}

		if (!CaptionTemplate.Text.IsEmpty())
		{
			RenderObj_Text(CaptionTemplate, FVector2D(0.0f, 300.0f));

			if (UTPlayerOwner->UTPlayerState->GetRemainingBoosts() == 0)
			{
				CaptionTemplate.Text = NSLOCTEXT("PowerupWheel","Charging","<Charging...>");
				CaptionTemplate.TextScale = 0.75f;
				RenderObj_Text(CaptionTemplate, FVector2D(0.0f, 335.0f));
			}
		}
	}
}

void UUTRadialMenu_BoostPowerup::TriggerPowerup()
{
	if (SelectedPowerup != INDEX_NONE && UTPlayerOwner->UTPlayerState != nullptr)
	{
		AUTGameState* GS = UTHUDOwner->GetWorld()->GetGameState<AUTGameState>();
		if (GS->GetSelectableBoostByIndex(UTPlayerOwner->UTPlayerState, SelectedPowerup) != nullptr)
		{
			UTPlayerOwner->UTPlayerState->ServerSetBoostItem(SelectedPowerup);
			UTPlayerOwner->ServerActivatePowerUpPress();
		}
	}
}
void UUTRadialMenu_BoostPowerup::Execute()
{
	TriggerPowerup();
}

