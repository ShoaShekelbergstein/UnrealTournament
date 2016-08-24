// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTHUDWidget_WeaponBar.h"
#include "UTWeapon.h"
#include "UTProfileSettings.h"
#include "UTWeap_Translocator.h"
#include "UTHUDWidgetMessage.h"

UUTHUDWidget_WeaponBar::UUTHUDWidget_WeaponBar(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DesignedResolution = 1080.0f;
	VerticalPosition = FVector2D(-8.0f, 0.0f);
	HorizontalPosition = FVector2D(0.0f, -8.0f);

	VerticalScreenPosition = FVector2D(1.0f, 0.5f);
	HorizontalScreenPosition = FVector2D(0.5f, 1.0f);

	Position=FVector2D(-8.0f, 0.0f);

	SelectedCellScale=1.1f;
	SelectedAnimRate=0.3f;

	AmmoBarSizePct = FVector2D(0.8f, 0.20f);

	SelectedCellScale = 1.0f;
	SelectedAnimRate = 0.0f;

	static ConstructorHelpers::FObjectFinder<UTexture2D> WeaponIconAtlas(TEXT("Texture2D'/Game/RestrictedAssets/UI/WeaponAtlas01.WeaponAtlas01'"));
	DefaultWeaponIconAtlas = WeaponIconAtlas.Object;
	WeaponIcon.Atlas = DefaultWeaponIconAtlas;

	static ConstructorHelpers::FObjectFinder<UTexture2D> WhiteTextureObj(TEXT("Texture2D'/Engine/EngineResources/WhiteSquareTexture'"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> BlackTextureObj(TEXT("Texture2D'/Engine/EngineResources/Black'"));

	ActiveBackgroundUVs = FTextureUVs(13,958, 112, 46);
	InactiveBackgroundUVs = FTextureUVs(129,958, 112, 46);
	SelectedBackgroundUVs = FTextureUVs(247,958, 112, 46);

	CellBackground.Atlas = WeaponIconAtlas.Object;
	CellBackground.RenderColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);

	AmmoBarBackground.Atlas = BlackTextureObj.Object;
	AmmoBarBackground.UVs = FTextureUVs(0.0f,0.0f,1.0f,1.0f);

	AmmoBarFill.Atlas = WhiteTextureObj.Object;
	AmmoBarFill.UVs = FTextureUVs(0.0f,0.0f,1.0f,1.0f);

	SelectedOpacity = 0.75f;
	ActiveOpacity = 0.3f;
	InactiveOpacity = 0.3f;

	LastSelectedWeapon = nullptr;

}

void UUTHUDWidget_WeaponBar::InitializeWidget(AUTHUD* Hud)
{
	Super::InitializeWidget(Hud);

	checkSlow(Hud != nullptr);
	if (Hud != nullptr)
	{
		GroupText.Font = Hud->TinyFont;
		GroupText.RenderColor = FLinearColor::White;
		GroupText.HorzPosition = ETextHorzPos::Center;
		GroupText.VertPosition = ETextVertPos::Center;
		GroupText.bDrawShadow = true;
		GroupText.ShadowDirection = FVector2D(1.0f, 1.0f);
		GroupText.ShadowColor = FLinearColor::Black;
	}
}

void UUTHUDWidget_WeaponBar::UpdateGroups(AUTHUD* Hud)
{
	// Find all of the weapons currently in memory to figure out the max # of slots we will need.

	if (Hud->UTPlayerOwner->AreMenusOpen()) return;

	UUTProfileSettings* PlayerProfile = Hud->UTPlayerOwner->GetProfileSettings();

	// There are 10 groups
	KnownWeaponMap.Empty();
	KnownWeaponMap.SetNumZeroed(11);

	// grant all weapons that are in memory
	for (TObjectIterator<UClass> It; It; ++It)
	{
		// make sure we don't use abstract, deprecated, or blueprint skeleton classes
		if (It->IsChildOf(AUTWeapon::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) && !It->GetName().StartsWith(TEXT("SKEL_")) && !It->IsChildOf(AUTWeap_Translocator::StaticClass()))
		{
			UClass* WeaponClass = *It;
			if (!WeaponClass->IsPendingKill())
			{
				AUTWeapon* DefaultWeaponObj = Cast<AUTWeapon>(WeaponClass->GetDefaultObject());
				NumWeaponsToDraw++;

				int32 WeaponGroup = (DefaultWeaponObj != nullptr) ? DefaultWeaponObj->DefaultGroup : 0;
				int32 GroupPriority = 0;
				if (PlayerProfile)
				{
					PlayerProfile->GetWeaponGroup(DefaultWeaponObj, WeaponGroup, GroupPriority);
				}

				if (KnownWeaponMap.IsValidIndex(WeaponGroup))
				{
					// hacky for bio launcher: if we have both a base class and its direct subclass, only record the subclass for now
					// this will get updated if necessary at runtime if both weapons are really around
					bool bOk = true;
					if (KnownWeaponMap[WeaponGroup].WeaponClasses.Num() > 0 && KnownWeaponMap[WeaponGroup].WeaponClasses[0] != nullptr)
					{
						if (KnownWeaponMap[WeaponGroup].WeaponClasses[0]->IsChildOf(WeaponClass))
						{
							bOk = false;
						}
					}
					if (bOk)
					{
						KnownWeaponMap[WeaponGroup].UpdateWeapon(WeaponClass, nullptr, WeaponGroup);
					}
				}
			}
		}
	}
}

/**
	The WeaponBar scaling size is incorrect due to it having a dynamic size. We need to adjust the end RenderPosition to account for its size to keep it centered.
*/
void UUTHUDWidget_WeaponBar::PreDraw(float DeltaTime, AUTHUD* InUTHUDOwner, UCanvas* InCanvas, FVector2D InCanvasCenter)
{
	if (InUTHUDOwner->GetWorld() == nullptr) return;

	UpdateGroups(InUTHUDOwner);

	MaxSize = FVector2D(InCanvas->ClipX * 0.925f, InCanvas->ClipY * 0.8f);

	// Built at the start of each frame then emptied.
	TArray<FWeaponGroupInfo> WeaponMap = KnownWeaponMap;

	NumWeaponsToDraw = 0;
	Cells.Empty();

	int32 GroupCount = 0; // How many different weapon groups are there
	AUTCharacter* UTCharacterOwner = Cast<AUTCharacter>(InUTHUDOwner->UTPlayerOwner->GetPawn());
	if (UTCharacterOwner != nullptr)
	{
		// Look for activity
		AUTWeapon* CurrentWeapon = UTCharacterOwner->GetPendingWeapon();
		if (CurrentWeapon == nullptr) CurrentWeapon = UTCharacterOwner->GetWeapon();
		if (CurrentWeapon != LastSelectedWeapon)
		{
			LastActiveTime = InUTHUDOwner->GetWorld()->GetTimeSeconds();
			LastSelectedWeapon = CurrentWeapon;
		}

		UUTProfileSettings* PlayerProfile = InUTHUDOwner->UTPlayerOwner->GetProfileSettings();
		bVerticalLayout = PlayerProfile == nullptr ? UUTProfileSettings::StaticClass()->GetDefaultObject<UUTProfileSettings>()->bVerticalWeaponBar : PlayerProfile->bVerticalWeaponBar;

		AUTGameState* UTGameState = InUTHUDOwner->GetWorld()->GetGameState<AUTGameState>();
		if (UTGameState)
		{
			CellBackground.bUseTeamColors = false; // UTGameState->bTeamGame;	
		}

		// grabs the coords via the layout
		ScreenPosition = (bVerticalLayout) ? VerticalScreenPosition : HorizontalScreenPosition;
		Position = (bVerticalLayout) ? VerticalPosition : HorizontalPosition;

		for (int32 i=0; i < WeaponMap.Num();i++)
		{
			if (WeaponMap[i].WeaponClasses.Num() > 0)
			{
				GroupCount++;
			}
		}

		// Now look at the player's Inventory and find all of the weapons they have and update the map
		for (TInventoryIterator<AUTWeapon> It(UTCharacterOwner); It; ++It)
		{
			AUTWeapon* Weapon = *It;
			if (Weapon != nullptr)
			{
				int32 WeaponGroup = Weapon->DefaultGroup;
				int32 GroupPriority = Weapon->GroupSlot;
				if (PlayerProfile)
				{
					PlayerProfile->GetWeaponGroup(Weapon, WeaponGroup, GroupPriority);
				}
				if (WeaponMap.IsValidIndex(WeaponGroup))
				{
					WeaponMap[WeaponGroup].UpdateWeapon(Weapon->GetClass(), Weapon, WeaponGroup);
					// update default state if necessary (e.g. weapon was loaded after HUD was initialized)
					if (!KnownWeaponMap[WeaponGroup].WeaponClasses.Contains(Weapon->GetClass()))
					{
						KnownWeaponMap[WeaponGroup].AddWeapon(Weapon->GetClass(), nullptr, WeaponGroup);
					}
				}
			}
		}



	}

	FVector2D CellSize = FVector2D(0.0f, 0.0f);
	FVector2D FinalSize = FVector2D(0.0f, 0.0f);

	float Padding = bVerticalLayout ? CELL_PADDING_VERT : CELL_PADDING_HORZ;
	float GroupPadding = bVerticalLayout ? GROUP_PADDING_VERT : GROUP_PADDING_HORZ;

	if (bVerticalLayout)
	{
		FinalSize.Y = ( NumWeaponsToDraw * DEFUALT_CELL_HEIGHT ) + ( Padding * (NumWeaponsToDraw - 1) ) + ( GroupPadding * (GroupCount -1));
		if (FinalSize.Y < MaxSize.Y)
		{
			CellSize.Y = DEFUALT_CELL_HEIGHT;
		}
		else
		{
			CellSize.Y = DEFUALT_CELL_HEIGHT * (MaxSize.Y / FinalSize.Y);
			Padding *= (MaxSize.Y / FinalSize.Y);
			GroupPadding *= (MaxSize.Y / FinalSize.Y);
			FinalSize *= (MaxSize.Y / FinalSize.Y);
		}

		CellSize.X = CellSize.Y * CELL_ASPECT_RATIO;
	}
	else
	{
		FinalSize.X = (NumWeaponsToDraw * DEFAULT_CELL_WIDTH) + (Padding * (NumWeaponsToDraw - 2)) + (GroupPadding * (GroupCount-1));
		if (FinalSize.X < MaxSize.X)
		{
			CellSize.X = DEFAULT_CELL_WIDTH;
		}
		else
		{
			CellSize.X = DEFAULT_CELL_WIDTH * (MaxSize.X / FinalSize.X);
			Padding *= (MaxSize.X / FinalSize.X);
			GroupPadding *= (MaxSize.X / FinalSize.X);
			FinalSize *= (MaxSize.X / FinalSize.X);
		}
		CellSize.Y = CellSize.X / CELL_ASPECT_RATIO;
	}

	CellSize.X = float(int(CellSize.X));
	CellSize.Y = float(int(CellSize.Y));

	Padding = float(int(Padding));

	FVector2D DrawOffset = FVector2D(0.0f, 0.0f);
	for (int32 i=0; i < WeaponMap.Num(); i++)
	{
		FWeaponGroupInfo* GroupInfo = &WeaponMap[i];
		if (GroupInfo->WeaponClasses.Num() > 0)
		{
			for (int32 WeapIdx = 0; WeapIdx < GroupInfo->WeaponClasses.Num(); WeapIdx++)
			{
				if (GroupInfo->WeaponClasses[WeapIdx] != nullptr)
				{
					Cells.Add(FWeaponBarCell(DrawOffset, CellSize, GroupInfo->Weapons[WeapIdx], GroupInfo->WeaponClasses[WeapIdx], GroupInfo->Group));
					
					if (bVerticalLayout)
					{
						DrawOffset.Y += CellSize.Y + (i < WeaponMap.Num() -1 ? Padding : 0);
					}
					else
					{
						DrawOffset.X += CellSize.X + (i < WeaponMap.Num() -1 ? Padding : 0);
					}
				}
			}

			if (i < WeaponMap.Num() -1)
			{
				if (bVerticalLayout)
				{
					DrawOffset.Y += GroupPadding;
				}
				else
				{
					DrawOffset.X += GroupPadding;
				}
			}
		}
	}
	
	// Center it
	if (bVerticalLayout)
	{
		FinalSize = FVector2D(CellSize.X, DrawOffset.Y);
	}
	else
	{
		FinalSize = FVector2D(DrawOffset.X, CellSize.Y);
	}

	Origin = bVerticalLayout ? FVector2D(1.0f, 0.5f) : FVector2D(0.5f, 1.0f);
	Size = FinalSize;
	Super::PreDraw(DeltaTime, InUTHUDOwner, InCanvas, InCanvasCenter);
}

/**
 *	We aren't going to use DrawAllRenderObjects.  Instead we are going to have a nice little custom bit of drawing based on what weapon group this is.
 **/
void UUTHUDWidget_WeaponBar::Draw_Implementation(float DeltaTime)
{
	AUTWeapon* CurrentWeapon = nullptr;
	if (UTHUDOwner && UTHUDOwner->UTPlayerOwner)
	{
		AUTCharacter* UTCharacter = Cast<AUTCharacter>(UTHUDOwner->UTPlayerOwner->GetPawn());
		CurrentWeapon = UTCharacter != nullptr ? UTCharacter->GetPendingWeapon() : nullptr;
		if (CurrentWeapon == nullptr)
		{
			CurrentWeapon = UTCharacter != nullptr ? UTCharacter->GetWeapon() : nullptr;
		}
	}

	float LastActiveDelta = GetWorld()->GetTimeSeconds() - LastActiveTime;
	InactiveFadePerc = (LastActiveDelta < ACTIVE_FADE_DELAY) ? 1.0f : FMath::Clamp<float>( 1.0f - ((LastActiveDelta-ACTIVE_FADE_DELAY) / ACTIVE_FADE_TIME), 0.0f, 1.0f);

	for (int32 i=0; i < Cells.Num();i++)
	{
		bool bIsCurrentWeapon = false;
		if (Cells[i].Weapon != nullptr)
		{
			if (Cells[i].Weapon == CurrentWeapon)
			{
				bIsCurrentWeapon = true;
				CellBackground.RenderOpacity =  SelectedOpacity;
				CellBackground.UVs = SelectedBackgroundUVs;
			}
			else
			{
				CellBackground.RenderOpacity = ActiveOpacity;
				CellBackground.UVs = ActiveBackgroundUVs;
			}
			 
		}
		else
		{
			CellBackground.RenderOpacity = InactiveOpacity;
			CellBackground.UVs = InactiveBackgroundUVs;
		}

		// Figure out the general opacity of the slot

		FVector2D CurrentWeaponPositionMod = FVector2D(0.0f, 0.0f);
		FVector2D CurrentWeaponSizeMod = FVector2D(0.0f, 0.0f);

		if (bIsCurrentWeapon)
		{
			Opacity = 1.0f;
			CurrentWeaponPositionMod = bVerticalLayout ? FVector2D(-10.0f, 0.0f) : FVector2D(0.0f, -10.0f);
			CurrentWeaponSizeMod = bVerticalLayout ? FVector2D(10.0f, 0.0f) : FVector2D(0.0f, 10.0f);
			CellBackground.RenderColor = FLinearColor(0.25f,0.25f,0.05f,1.0f);
		
		}
		else
		{
			CellBackground.RenderColor = FLinearColor(0.1f,0.1f,0.1f,1.0f);

			float DesiredOpacity = Cells[i].Weapon == nullptr ? UTHUDOwner->GetHUDWidgetWeaponBarEmptyOpacity() : UTHUDOwner->GetHUDWidgetWeaponbarInactiveOpacity();
			Opacity =  FMath::Lerp(DesiredOpacity,1.0f, InactiveFadePerc);
		}

		// Draw the background
		CellBackground.Size = Cells[i].DrawSize + CurrentWeaponSizeMod;
		RenderObj_Texture(CellBackground, Cells[i].DrawPosition + CurrentWeaponPositionMod);

		AUTWeapon* ThisWeapon = Cells[i].Weapon != nullptr ? Cells[i].Weapon : Cells[i].WeaponClass->GetDefaultObject<AUTWeapon>();

		if (ThisWeapon != nullptr)
		{
			FVector2D AmmoBarSize = FVector2D(Cells[i].DrawSize.X * AmmoBarSizePct.X, Cells[i].DrawSize.Y * AmmoBarSizePct.Y);
			FVector2D AmmoBarPosition = FVector2D( (Cells[i].DrawSize.X * 0.5f) -(AmmoBarSize.X * 0.5f), Cells[i].DrawSize.Y - 4 - AmmoBarSize.Y);

			// Draw the fill

			float AmmoPerc = 0.0f;
			if (Cells[i].Weapon != nullptr)
			{
				AmmoPerc = ThisWeapon->MaxAmmo > 0 ? float(ThisWeapon->Ammo) / float(ThisWeapon->MaxAmmo) : 0.f;
			}

			FVector2D FillSize = AmmoBarSize;
			FillSize.X *= AmmoPerc;
			
			if (AmmoPerc > 0.0f)
			{
				// Draw the ammo bar background
				AmmoBarBackground.Size = AmmoBarSize;
				RenderObj_Texture(AmmoBarBackground, Cells[i].DrawPosition + AmmoBarPosition);

				FVector2D FillPosition = AmmoBarPosition;
				//FillPosition.X += (AmmoBarSize.X - FillSize.X);
				AmmoBarFill.Size = FillSize;
		
				if (AmmoPerc <= 0.33f) AmmoBarFill.RenderColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
				else if (AmmoPerc < 0.66f) AmmoBarFill.RenderColor = FLinearColor(1.0f, 1.0f, 0.4f, 1.0f);
				else AmmoBarFill.RenderColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

				if (!bIsCurrentWeapon)
				{
					AmmoBarFill.RenderColor *= 0.5;
				}

				RenderObj_Texture(AmmoBarFill, Cells[i].DrawPosition + FillPosition);
			}

			WeaponIcon.UVs = Cells[i].Weapon != nullptr ? ThisWeapon->WeaponBarSelectedUVs : ThisWeapon->WeaponBarSelectedUVs; // ThisWeapon->WeaponBarInactiveUVs;
			WeaponIcon.RenderColor = UTHUDOwner->GetUseWeaponColors() ? ThisWeapon->IconColor : FLinearColor::White;

			if (Cells[i].Weapon == nullptr)
			{
				WeaponIcon.RenderColor = CellBackground.bUseTeamColors ? UTHUDOwner->GetWidgetTeamColor() : CellBackground.RenderColor;
			}

			float WeaponIconWidth = WeaponIcon.UVs.UL;
			float WeaponIconHeight = WeaponIcon.UVs.VL;

			if (WeaponIconWidth > Cells[i].DrawSize.X * 0.9f)
			{
				WeaponIconWidth = Cells[i].DrawSize.X * 0.9f;
				WeaponIconHeight = WeaponIconWidth * (WeaponIcon.UVs.VL / WeaponIcon.UVs.UL);
			}

			if (WeaponIconHeight > AmmoBarPosition.Y * 0.95f)
			{
				WeaponIconHeight = AmmoBarPosition.Y * 0.95f;
				WeaponIconWidth = WeaponIconHeight * (WeaponIcon.UVs.UL / WeaponIcon.UVs.VL);
			}

			FVector2D WeaponIconPosition = Cells[i].Weapon != nullptr ?
					FVector2D( (Cells[i].DrawSize.X * 0.5f) - (WeaponIconWidth * 0.5), (AmmoBarPosition.Y * 0.5f) - (WeaponIconHeight * 0.5)) :
					FVector2D( (Cells[i].DrawSize.X * 0.5f) - (WeaponIconWidth * 0.5), (Cells[i].DrawSize.Y * 0.5f) - (WeaponIconHeight * 0.5));

			WeaponIcon.Size = FVector2D(WeaponIconWidth, WeaponIconHeight);
			RenderObj_Texture(WeaponIcon, Cells[i].DrawPosition + WeaponIconPosition);
		}
	}

	// Draw the group labels...


	float LabelPos = 0;
	float LabelStart = 0;
	float LabelEnd = 0;
	int32 LabelGroup = -1;

	CellBackground.UVs = FTextureUVs(30.0f,970.0f,1.0f,1.0f);

	for (int32 i=0; i < Cells.Num(); i++)
	{
		AUTWeapon* ThisWeapon = Cells[i].Weapon != nullptr ? Cells[i].Weapon : Cells[i].WeaponClass->GetDefaultObject<AUTWeapon>();
		if (ThisWeapon != nullptr)
		{
			if (LabelGroup < 0 || Cells[i].WeaponGroup != LabelGroup)
			{
				// Draw this label
				if (LabelGroup >= 0)
				{
					DrawLabel(LabelPos, LabelStart, LabelEnd, LabelGroup);
				}

				LabelGroup = Cells[i].WeaponGroup;
				LabelPos = bVerticalLayout ? Cells[i].DrawPosition.X : Cells[i].DrawPosition.Y;
				LabelStart = bVerticalLayout ? Cells[i].DrawPosition.Y : Cells[i].DrawPosition.X;
				LabelEnd = bVerticalLayout ? Cells[i].DrawPosition.Y + Cells[i].DrawSize.Y : Cells[i].DrawPosition.X + Cells[i].DrawSize.X;
			}
			else if (Cells[i].WeaponGroup == LabelGroup)
			{
				LabelEnd = bVerticalLayout ? Cells[i].DrawPosition.Y + Cells[i].DrawSize.Y : Cells[i].DrawPosition.X + Cells[i].DrawSize.X;
			}
		}
	}

	DrawLabel(LabelPos, LabelStart, LabelEnd, LabelGroup);
}

void UUTHUDWidget_WeaponBar::DrawLabel(float LabelPos, float LabelStart, float LabelEnd, float LabelGroup)
{
	Opacity =  FMath::Lerp(UTHUDOwner->GetHUDWidgetWeaponBarEmptyOpacity(),1.0f, InactiveFadePerc);

	CellBackground.RenderColor = FLinearColor(0.25f,0.25f,0.25f,1.0f);

	float CharXSize, CharYSize;
	GroupText.Font->GetCharSize(TEXT('Q'), CharXSize, CharYSize);

	FVector2D LabelDrawPosition;
	FVector2D BarAPosition;
	FVector2D BarBPosition;
	FVector2D BarSize;

	if (bVerticalLayout)
	{
		LabelPos -= CharXSize * 0.5;
		BarSize = FVector2D( 3.0f, (LabelEnd - LabelStart - CharYSize) * 0.5f);
		LabelDrawPosition = FVector2D( LabelPos, LabelStart + ( (LabelEnd - LabelStart) * 0.5f));
		BarAPosition = FVector2D(LabelPos - 1.0f, LabelStart);
		BarBPosition = FVector2D(LabelPos - 1.0f, LabelDrawPosition.Y + CharYSize * 0.5f);
	}
	else
	{
		LabelPos -= CharYSize * 0.5;
		BarSize = FVector2D( (LabelEnd - LabelStart - CharXSize) * 0.5f, 3.0f);
		LabelDrawPosition = FVector2D( LabelStart + ( (LabelEnd - LabelStart) * 0.5f), LabelPos);
		BarAPosition = FVector2D(LabelStart, LabelPos - 1.0f);
		BarBPosition = FVector2D(LabelDrawPosition.X + CharXSize * 0.5f, LabelPos - 1.0f);
	}
					
	// Draw everything...

	CellBackground.Size = BarSize; 
	CellBackground.RenderOpacity = 1.0;
	RenderObj_Texture(CellBackground, BarAPosition);
	RenderObj_Texture(CellBackground, BarBPosition);

	FString* GroupKey = UTHUDOwner->UTPlayerOwner->WeaponGroupKeys.Find(LabelGroup);
	if ((GroupKey == nullptr) ||GroupKey->IsEmpty())
	{
		GroupText.Text = (LabelGroup == 10) ? FText::FromString(TEXT("0")) : FText::AsNumber(LabelGroup);
	}
	else
	{
		GroupText.Text = (GroupKey->Len() == 1) ? FText::FromString(*GroupKey) : FText::FromString(TEXT(""));
	}

	GroupText.RenderOpacity = 1.0f;
	GroupText.RenderColor = FLinearColor::White;
	RenderObj_Text(GroupText, LabelDrawPosition);
}




float UUTHUDWidget_WeaponBar::GetDrawScaleOverride()
{
	return UTHUDOwner->GetHUDWidgetScaleOverride() * UTHUDOwner->GetHUDWidgetWeaponBarScaleOverride();
}

bool UUTHUDWidget_WeaponBar::ShouldDraw_Implementation(bool bShowScores)
{
	return Super::ShouldDraw_Implementation(bShowScores) && (!UTHUDOwner->bDrawMinimap || (UTHUDOwner->UTPlayerOwner && UTHUDOwner->UTPlayerOwner->UTPlayerState && !UTHUDOwner->UTPlayerOwner->UTPlayerState->bOnlySpectator && !UTHUDOwner->UTPlayerOwner->UTPlayerState->bOutOfLives));
}
