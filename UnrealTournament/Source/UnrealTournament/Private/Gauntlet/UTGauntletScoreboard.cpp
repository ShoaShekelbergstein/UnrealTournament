// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTGauntletScoreboard.h"
#include "UTGauntletGameState.h"

UUTGauntletScoreboard::UUTGauntletScoreboard(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
/*
	BadgeNumberUVs.Add(FVector2D(248,183));
	BadgeNumberUVs.Add(FVector2D(283,183));
	BadgeNumberUVs.Add(FVector2D(318,183));
	BadgeNumberUVs.Add(FVector2D(353,183));
	BadgeNumberUVs.Add(FVector2D(388,183));
	BadgeNumberUVs.Add(FVector2D(423,183));
	BadgeNumberUVs.Add(FVector2D(458,183));
	BadgeNumberUVs.Add(FVector2D(248,219));
	BadgeNumberUVs.Add(FVector2D(283,219));

	BadgeUVs.Add(FVector2D(423,219));
	BadgeUVs.Add(FVector2D(388,219));
	BadgeUVs.Add(FVector2D(353,219));
	BadgeUVs.Add(FVector2D(318,219));

	StarUVs.Add(FVector2D(280,136));
	StarUVs.Add(FVector2D(314,136));
	StarUVs.Add(FVector2D(348,136));
	StarUVs.Add(FVector2D(382,136));
	StarUVs.Add(FVector2D(416,136));

	CellWidth = 600.f;
*/
}

/*
void UUTGauntletScoreboard::DrawGameOptions(float RenderDelta, float& YOffset)
{
	if (UTGameState)
	{
		FText StatusText = UTGameState->GetGameStatusText(true);
		if (!StatusText.IsEmpty())
		{
			DrawText(StatusText, Canvas->ClipX - 100.f*RenderScale, YOffset + 48.f*RenderScale, UTHUDOwner->SmallFont, RenderScale, 1.f, FLinearColor::Yellow, ETextHorzPos::Right, ETextVertPos::Center);
		}
		else if (UTGameState->GoalScore > 0)
		{
			// Draw Game Text
			FText Score = NSLOCTEXT("UTGuantletGame","ScoreCap","3 Stars to Win");
			DrawText(Score, Canvas->ClipX - 100.f*RenderScale, YOffset + 48.f*RenderScale, UTHUDOwner->SmallFont, RenderScale, 1.f, FLinearColor::Yellow, ETextHorzPos::Right, ETextVertPos::Center);
		}

		float DisplayedTime = UTGameState ? UTGameState->GetClockTime() : 0.f;
		FText Timer = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), DisplayedTime, false, true, true);
		DrawText(Timer, Canvas->ClipX - 100.f*RenderScale, YOffset + 22.f*RenderScale, UTHUDOwner->NumberFont, RenderScale, 1.f, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
	}
}

bool UUTGauntletScoreboard::ShouldDrawScoringStats()
{
	return UTGameState && UTGameState->HasMatchEnded();	
}


void UUTGauntletScoreboard::DrawMinimap(float RenderDelta)
{
}


void UUTGauntletScoreboard::DrawTeamPanel(float RenderDelta, float& YOffset)
{
	if (UTGameState == NULL || UTGameState->Teams.Num() < 2 || UTGameState->Teams[0] == NULL || UTGameState->Teams[1] == NULL) return;

	float Width = 0.5f * (Size.X - 400.f) * RenderScale;

	float FrontSize = 35.f * RenderScale;
	float EndSize = 16.f * RenderScale;
	float MiddleSize = Width - FrontSize - EndSize;
	float BackgroundY = YOffset + 22.f * RenderScale;
	float TeamTextY = YOffset + 40.f * RenderScale;
	float TeamScoreY = YOffset + 36.f * RenderScale;
	float BackgroundHeight = 65.f * RenderScale;
	float TeamEdgeSize = 40.f * RenderScale;
	float NamePosition = TeamEdgeSize + FrontSize + 0.15f*MiddleSize;

	// Draw the Background
	DrawTexture(UTHUDOwner->ScoreboardAtlas, TeamEdgeSize, BackgroundY, FrontSize, BackgroundHeight, 0, 188, 36, 65, 1.0, FLinearColor::Red);
	DrawTexture(UTHUDOwner->ScoreboardAtlas, TeamEdgeSize + FrontSize, BackgroundY, MiddleSize, BackgroundHeight, 39,188,64,65, 1.0, FLinearColor::Red);
	DrawTexture(UTHUDOwner->ScoreboardAtlas, TeamEdgeSize + FrontSize + MiddleSize, BackgroundY, EndSize, BackgroundHeight, 39,188,64,65, 1.0, FLinearColor::Red);

	DrawText(RedTeamText, NamePosition, TeamTextY, UTHUDOwner->HugeFont, RenderScale, 1.f, FLinearColor::White, ETextHorzPos::Left, ETextVertPos::Center);

	float ScorePosition = TeamEdgeSize + FrontSize + MiddleSize - EndSize;

	DrawText(FText::AsNumber(UTGameState->Teams[0]->Score), TeamEdgeSize + FrontSize + MiddleSize - EndSize, TeamScoreY, UTHUDOwner->HugeFont, false, FVector2D(0, 0), FLinearColor::Black, true, FLinearColor::Black, 1.5f*RenderScale, 1.f, FLinearColor::White, FLinearColor(0.0f,0.0f,0.0f,0.0f), ETextHorzPos::Right, ETextVertPos::Center);

	float LeftEdge = Canvas->ClipX - TeamEdgeSize - FrontSize - MiddleSize - EndSize;

	DrawTexture(UTHUDOwner->ScoreboardAtlas, LeftEdge + EndSize + MiddleSize, BackgroundY, FrontSize, BackgroundHeight, 196, 188, 36, 65 , 1.f, FLinearColor::Blue);
	DrawTexture(UTHUDOwner->ScoreboardAtlas, LeftEdge + EndSize, BackgroundY, MiddleSize, BackgroundHeight, 130,188,64,65, 1.f, FLinearColor::Blue);
	DrawTexture(UTHUDOwner->ScoreboardAtlas, LeftEdge, BackgroundY, EndSize, BackgroundHeight, 117, 188, 16, 65, 1.f, FLinearColor::Blue);

	DrawText(BlueTeamText, Canvas->ClipX - NamePosition, TeamTextY, UTHUDOwner->HugeFont, RenderScale, 1.f, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
	DrawText(FText::AsNumber(UTGameState->Teams[1]->Score), LeftEdge + 2.f*EndSize, TeamScoreY, UTHUDOwner->HugeFont, false, FVector2D(0.f, 0.f), FLinearColor::Black, true, FLinearColor::Black, 1.5f*RenderScale, 1.f, FLinearColor::White, FLinearColor(0.0f,0.0f,0.0f,0.0f), ETextHorzPos::Left, ETextVertPos::Center);
	YOffset += 96.f * RenderScale;
}


void UUTGauntletScoreboard::DrawScoreHeaders(float RenderDelta, float& YOffset)
{
	float XOffset = ScaledEdgeSize;
	float Height = 28.f * RenderScale;

	for (int32 i = 0; i < 2; i++)
	{
		// Draw the background Border
		DrawTexture(UTHUDOwner->ScoreboardAtlas, XOffset, YOffset, ScaledCellWidth, Height, 149, 138, 32, 32, 1.0, FLinearColor(0.72f, 0.72f, 0.72f, 0.85f));
		DrawText(CH_PlayerName, XOffset + (ScaledCellWidth * ColumnHeaderPlayerX), YOffset + ColumnHeaderY, UTHUDOwner->MediumFont, RenderScale, 1.0f, FLinearColor::Black, ETextHorzPos::Left, ETextVertPos::Center);

		if (UTGameState && !UTGameState->HasMatchStarted())
		{
			DrawText((GetWorld()->GetNetMode() == NM_Standalone) ? CH_Skill : CH_Ping, XOffset + (ScaledCellWidth - (5 * RenderScale)), YOffset + ColumnHeaderY, UTHUDOwner->MediumFont, RenderScale, 1.0f, FLinearColor::Black, ETextHorzPos::Right, ETextVertPos::Center);
		}

		XOffset = Canvas->ClipX - ScaledCellWidth - ScaledEdgeSize;
	}
	YOffset += Height + 4;
}

void UUTGauntletScoreboard::DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset)
{
	Super::DrawPlayer(Index, PlayerState, RenderDelta, XOffset, YOffset);

	if (UTGameState && !UTGameState->HasMatchStarted())
	{
		DrawBadge(PlayerState, XOffset, YOffset);
	}
}



void UUTGauntletScoreboard::DrawPlayerScore(AUTPlayerState* PlayerState, float XOffset, float YOffset, float Width, FLinearColor DrawColor)
{
	Super::DrawPlayerScore(PlayerState,XOffset, YOffset, Width, DrawColor);
}

void UUTGauntletScoreboard::DrawBadge(AUTPlayerState* PlayerState, float XOffset, float YOffset)
{
	AUTGameMode* DefaultGame = UTGameState && UTGameState->GameModeClass ? UTGameState->GameModeClass->GetDefaultObject<AUTGameMode>() : NULL;
	bool bRankedSession = UTGameState ? UTGameState->bRankedSession : false;
	if (DefaultGame)
	{
		float MedalPosition = 0.5f * FlagX;

		XOffset -= 32 * RenderScale;

		int32 Badge = 0;
		int32 Level = 0;
		int32 Stars = 0;
		PlayerState->GetBadgeFromELO(DefaultGame, bRankedSession, Badge, Level);
		UUTLocalPlayer::GetStarsFromXP(GetLevelForXP(PlayerState->GetPrevXP()), Stars);
		Badge = FMath::Clamp<int32>(Badge, 0, 3);
		Level = FMath::Clamp<int32>(Level, 0, 8);

		FLinearColor BadgeColor = FLinearColor(0.36f, 0.8f, 0.34f, 1.0f);
		if (Badge == 1) BadgeColor = FLinearColor(0.4f, 0.235f, 0.07f, 1.0f);
		else if (Badge == 2) BadgeColor = FLinearColor(0.96f, 0.96f, 0.96f, 1.0f);
		else if (Badge == 3) BadgeColor = FLinearColor(1.0f, 0.95f, 0.42f, 1.0f);

		DrawTexture(UTHUDOwner->ScoreboardAtlas, XOffset + (ScaledCellWidth * MedalPosition), YOffset + 12.f*RenderScale, 32.f*RenderScale, 32.f*RenderScale, BadgeUVs[Badge].X, BadgeUVs[Badge].Y, 32, 32, 1.0, BadgeColor, FVector2D(0.5f, 0.5f));
		DrawTexture(UTHUDOwner->ScoreboardAtlas, XOffset + (ScaledCellWidth * MedalPosition), YOffset + 12.f*RenderScale, 32.f*RenderScale, 32.f*RenderScale, BadgeNumberUVs[Level].X, BadgeNumberUVs[Level].Y, 32, 32, 1.0, FLinearColor::White, FVector2D(0.5f, 0.5f));

		if (Stars > 0 && Stars <= 5)
		{
			DrawTexture(UTHUDOwner->ScoreboardAtlas, XOffset + (ScaledCellWidth * MedalPosition), YOffset + 16.f*RenderScale, 32.f*RenderScale, 32.f*RenderScale, StarUVs[Stars-1].X, StarUVs[Stars-1].Y, 32, 32, 1.0, FLinearColor(1.0f, 0.95f, 0.42f, 1.0f), FVector2D(0.5f, 0.5f));
		}
	}
	

}

*/