// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTGauntletGameState.h"
#include "UTGauntletGame.h"
#include "UTGauntletHUD.h"

AUTGauntletHUD::AUTGauntletHUD(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM1(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round1.Round1'"));
	RoundMarkers.Add(RM1.Object);
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM2(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round2.Round2'"));
	RoundMarkers.Add(RM2.Object);
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM3(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round3.Round3'"));
	RoundMarkers.Add(RM3.Object);
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM4(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round4.Round4'"));
	RoundMarkers.Add(RM4.Object);
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM5(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round5.Round5'"));
	RoundMarkers.Add(RM5.Object);
	static ConstructorHelpers::FObjectFinder<UTexture2D> RM6(TEXT("Texture2D'/Game/RestrictedAssets/UI/RoundMarkers/Round6.Round6'"));
	RoundMarkers.Add(RM6.Object);

	AnimPhase = -1;
	FadeToBlackDuration = 1.2f;
	RoundAnnounceDurations.Add(0.7f);	// Show Round and Background full white
	RoundAnnounceDurations.Add(1.5f);   // Fade out white, scale out round

	RedTeamIcon.U = 5.f;
	RedTeamIcon.V = 5.f;
	RedTeamIcon.UL = 224.f;
	RedTeamIcon.VL = 310.f;
	RedTeamIcon.Texture = CharacterPortraitAtlas;

	BlueTeamIcon.U = 237.f;
	BlueTeamIcon.V = 5.f;
	BlueTeamIcon.UL = 224.f;
	BlueTeamIcon.VL = 310.f;
	BlueTeamIcon.Texture = CharacterPortraitAtlas;

	BlueTeamOverlay.U = 237.0f;
	BlueTeamOverlay.V = 330.0f;
	BlueTeamOverlay.UL = 224.0f;
	BlueTeamOverlay.VL = 310.0f;
	BlueTeamOverlay.Texture = CharacterPortraitAtlas;

	RedTeamOverlay.U = 5.0f;
	RedTeamOverlay.V = 330.0f;
	RedTeamOverlay.UL = 224.0f;
	RedTeamOverlay.VL = 310.0f;
	RedTeamOverlay.Texture = CharacterPortraitAtlas;


}

void AUTGauntletHUD::NotifyMatchStateChange()
{
	Super::NotifyMatchStateChange();

	AUTGauntletGameState* GS = Cast<AUTGauntletGameState>(GetWorld()->GetGameState());

	if (GS && !GS->IsPendingKillPending())
	{
		FName MatchState = GS->GetMatchState();
		if (MatchState == MatchState::GauntletFadeToBlack || MatchState == MatchState::GauntletRoundAnnounce)
		{
			if (MatchState == MatchState::GauntletRoundAnnounce)
			{
				ClearAllUMGWidgets();
			}
			AnimPhase = 0;
			AnimTimer = 0.0f;
		}
	}
}

void AUTGauntletHUD::DrawHUD()
{
	if (!bShowUTHUD || UTPlayerOwner == nullptr || (!bShowHUD && UTPlayerOwner && UTPlayerOwner->bCinematicMode))
	{
		return;
	}

	AUTGauntletGameState* GauntletGameState = GetWorld()->GetGameState<AUTGauntletGameState>();
	if (GauntletGameState != nullptr)
	{
		Canvas->SetDrawColor(255,0,0,255);

		if (GauntletGameState->GetMatchState() == MatchState::GauntletScoreSummary)
		{
			DrawScoreSummary(GauntletGameState);
		}
		if (GauntletGameState->GetMatchState() == MatchState::GauntletFadeToBlack)
		{
			DrawFadeToBlack(GauntletGameState);
		}
		else if (GauntletGameState->GetMatchState() == MatchState::GauntletRoundAnnounce)
		{
			DrawRoundAnnounce(GauntletGameState);
		}
		else if (GauntletGameState->GetMatchState() == MatchState::InProgress)
		{
			DrawPlayerIcons(GauntletGameState);
		}
	}

	Super::DrawHUD();
}

void AUTGauntletHUD::DrawScoreSummary(AUTGauntletGameState* GauntletGameState)
{
}

void AUTGauntletHUD::DrawFadeToBlack(AUTGauntletGameState* GauntletGameState)
{
	AnimTimer += RenderDelta;
	float Alpha = FMath::Clamp<float>(AnimTimer / FadeToBlackDuration, 0.0f, 1.0f);

	Canvas->SetDrawColor(0,0,0,uint8(255.0f * Alpha));
	Canvas->DrawTile(GEngine->DefaultTexture, 0.0, 0.0, Canvas->ClipX, Canvas->ClipY, 0,0,1,1);
}

void AUTGauntletHUD::DrawRoundAnnounce(AUTGauntletGameState* GauntletGameState)
{
	if (AnimPhase < 0 || !RoundAnnounceDurations.IsValidIndex(AnimPhase)) return;

	float Alpha = AnimTimer / RoundAnnounceDurations[AnimPhase];
	if (AnimPhase == 0)
	{
		DrawRoundImage(GauntletGameState, 1.0f, 1.0f, 1.0f);
	}
	else if (AnimPhase == 1)
	{
		float RoundScale = FMath::InterpEaseIn(1.0f, 8.0f, Alpha, 2.0f);
		float Opacity = 1.0f - Alpha;
		DrawRoundImage(GauntletGameState, RoundScale, Opacity > 0.5f ? 1.0f : Opacity * 2.0f, Opacity);
	}

	AnimTimer += RenderDelta;
	if (AnimTimer >= RoundAnnounceDurations[AnimPhase])
	{
		AnimPhase++;
		AnimTimer = 0.0f;
	}
}

void AUTGauntletHUD::DrawRoundImage(AUTGauntletGameState* GauntletGameState, float RoundScale, float GeneralOpacity, float FillOpacity)
{
	float RenderScale = Canvas->ClipY / 1080.0f;
	FVector2D RoundPosition = FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	FVector2D RoundSize = FVector2D(1024.0f, 1024.0f);

	RoundSize *= RoundScale * RenderScale;

	RoundPosition.X -= RoundSize.X * 0.5f;
	RoundPosition.Y -= RoundSize.Y * 0.5f;

	Canvas->SetDrawColor(255,255,0,uint8(255.0f * FillOpacity * GeneralOpacity));
	Canvas->DrawTile(GEngine->DefaultTexture, RoundPosition.X, RoundPosition.Y, RoundSize.X, RoundSize.Y,0,0,1,1);

	// Draw the Upper Strip

	Canvas->SetDrawColor(255,255,255,uint8(255.0f * GeneralOpacity));

	FVector2D P = FVector2D(0,0);
	FVector2D S = FVector2D(Canvas->ClipX, RoundPosition.Y);

	Canvas->DrawTile(RoundMarkers[0], P.X, P.Y, S.X, S.Y, 0,0,2,2);
	P.Y = RoundPosition.Y + RoundSize.Y;
	S.Y = Canvas->ClipY;
	Canvas->DrawTile(RoundMarkers[0], P.X, P.Y, S.X, S.Y, 0,0,2,2);

	P.Y = 0;
	S.Y = Canvas->ClipY;
	S.X = RoundPosition.X;
	Canvas->DrawTile(RoundMarkers[0], P.X, P.Y, S.X, S.Y, 0,0,2,2);

	P.X = RoundPosition.X + RoundSize.X;
	S.X = Canvas->ClipX;
	Canvas->DrawTile(RoundMarkers[0], P.X, P.Y, S.X, S.Y, 0,0,2,2);

	Canvas->DrawTile(RoundMarkers[GauntletGameState->CTFRound], RoundPosition.X, RoundPosition.Y, RoundSize.X, RoundSize.Y,0,0,1024,1024);

}
void AUTGauntletHUD::DrawPlayerIcons(AUTGauntletGameState* GauntletGameState)
{
	if ( !ScoreboardIsUp() )
	{

		int32 OldRedCount = RedPlayerCount;
		int32 OldBlueCount = BluePlayerCount;
		RedPlayerCount = 0;
		BluePlayerCount = 0;

		const float RenderScale = float(Canvas->SizeY) / 1080.0f;

		float TeammateScale = VerifyProfileSettings() ? CachedProfileSettings->HUDTeammateScaleOverride : UUTProfileSettings::StaticClass()->GetDefaultObject<UUTProfileSettings>()->HUDTeammateScaleOverride;

		float BasePipSize = (32 + (64 * TeammateScale)) * GetHUDWidgetScaleOverride() * RenderScale;  // 96 - 32px in size
		float XAdjust = BasePipSize * 1.1;
		float XOffsetRed = 0.4f * Canvas->ClipX - XAdjust - BasePipSize;
		float XOffsetBlue = 0.6f * Canvas->ClipX + XAdjust;
		float YOffset = 0.005f * Canvas->ClipY * GetHUDWidgetScaleOverride() * RenderScale;
		float XOffsetText = 0.f;

		TArray<AUTPlayerState*> LivePlayers;
		GetPlayerListForIcons(GauntletGameState, LivePlayers);
		for (AUTPlayerState* UTPS : LivePlayers)
		{
			float OwnerPipScaling = 1.f;
			float PipSize = BasePipSize * OwnerPipScaling;
			float LiveScaling = FMath::Clamp(((UTPS->RespawnTime > 0.f) && (UTPS->RespawnWaitTime > 0.f) && !UTPS->GetUTCharacter()) ? 1.f - UTPS->RespawnTime / UTPS->RespawnWaitTime : 1.f, 0.f, 1.f);

			if (UTPS->bOutOfLives) LiveScaling = 0.0f;

			if (UTPS->Team->TeamIndex == 0)
			{
				RedPlayerCount++;
				DrawPlayerIcon(GauntletGameState, UTPS, LiveScaling, XOffsetRed, YOffset, PipSize);
				XOffsetRed -= 1.1f*PipSize;
			}
			else
			{
				BluePlayerCount++;
				DrawPlayerIcon(GauntletGameState, UTPS, LiveScaling, XOffsetBlue, YOffset, PipSize);
				XOffsetBlue += 1.1f*PipSize;
			}
		}
	}
}

void AUTGauntletHUD::GetPlayerListForIcons(AUTGauntletGameState* GauntletGameState, TArray<AUTPlayerState*>& SortedPlayers)
{
	for (APlayerState* PS : GauntletGameState->PlayerArray)
	{
		AUTPlayerState* UTPS = Cast<AUTPlayerState>(PS);
		if (UTPS != NULL && UTPS->Team != NULL && !UTPS->bOnlySpectator && !UTPS->bIsInactive)
		{
			SortedPlayers.Add(UTPS);
		}
	}
	SortedPlayers.Sort([](AUTPlayerState& A, AUTPlayerState& B) { return A.PlayerName > B.PlayerName; });
}

void AUTGauntletHUD::DrawPlayerIcon(AUTGauntletGameState* GauntletGameState, AUTPlayerState* PlayerState, float LiveScaling, float XOffset, float YOffset, float PipSize)
{
	const FCanvasIcon& CharIcon = PlayerState->GetHUDIcon();
	if (CharIcon.Texture != nullptr)
	{
		FLinearColor BackColor = FLinearColor::Black;
		BackColor.A = 0.5f;

		Canvas->SetLinearDrawColor(FLinearColor::White);

		float PipHeight = PipSize * (320.0f / 224.0f);

		// Draw the background.
		const FCanvasIcon* BGIcon = PlayerState->GetTeamNum() == 1 ? &BlueTeamIcon : &RedTeamIcon;
		Canvas->DrawTile(BGIcon->Texture, XOffset, YOffset, PipSize, PipHeight, BGIcon->U, BGIcon->V, BGIcon->UL, BGIcon->VL);

		if (LiveScaling < 1.f)
		{
			Canvas->SetLinearDrawColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f));
		}

		BGIcon = &CharIcon;
		if (PlayerState->GetTeamNum() == 1)
		{
			Canvas->DrawTile(BGIcon->Texture, XOffset, YOffset, PipSize, PipHeight, BGIcon->U + BGIcon->UL, BGIcon->V, BGIcon->UL * -1.0f, BGIcon->VL);
		}
		else
		{
			Canvas->DrawTile(BGIcon->Texture, XOffset, YOffset, PipSize, PipHeight, BGIcon->U, BGIcon->V, BGIcon->UL, BGIcon->VL);
		}


		if (LiveScaling < 1.f)
		{
			Canvas->SetLinearDrawColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
			Canvas->DrawTile(Canvas->DefaultTexture, XOffset + LiveScaling*PipSize, YOffset, PipSize - LiveScaling * PipSize, PipHeight, 0, 0, 1, 1, BLEND_Translucent);
		}


		Canvas->SetLinearDrawColor(FLinearColor::White);

		BGIcon = PlayerState->GetTeamNum() == 1 ? &BlueTeamOverlay : &RedTeamOverlay;
		Canvas->DrawTile(BGIcon->Texture, XOffset, YOffset, PipSize, PipHeight, BGIcon->U, BGIcon->V, BGIcon->UL, BGIcon->VL);

		const float FontRenderScale = float(Canvas->SizeY) / 1080.0f;
		FFontRenderInfo TextRenderInfo;

		// draw pips for players alive on each team @TODO move to widget
		TextRenderInfo.bEnableShadow = true;

		FString LivesRemaining = FString::Printf(TEXT("%i"), PlayerState->RemainingLives);
		float XL, YL;
		Canvas->StrLen(SmallFont, LivesRemaining, XL, YL);

		Canvas->SetLinearDrawColor(PlayerState->RemainingLives == 1 ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(SmallFont, FText::FromString(LivesRemaining), XOffset + (PipSize * 0.5f) - (XL * 0.5f), YOffset + (PipSize * (320.0f / 224.0f)), FontRenderScale, FontRenderScale, TextRenderInfo);

		if (PlayerState->bOutOfLives)
		{
			Canvas->SetLinearDrawColor(FLinearColor::White);
			Canvas->DrawTile(HUDAtlas, XOffset + (PipSize * 0.5f) - (13 * FontRenderScale), YOffset + (PipHeight * 0.5f) - (18 * FontRenderScale),  27 * FontRenderScale, 36 * FontRenderScale, 725, 0, 27 , 36);
		}

	}
}

