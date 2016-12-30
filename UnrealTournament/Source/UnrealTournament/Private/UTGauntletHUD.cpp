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
