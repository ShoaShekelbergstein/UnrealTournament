// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCTFScoreboard.h"
#include "UTGauntletScoreboard.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTGauntletScoreboard : public UUTCTFScoreboard
{
	GENERATED_UCLASS_BODY()
/*
	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	TArray<FVector2D> BadgeNumberUVs;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	TArray<FVector2D> BadgeUVs;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	TArray<FVector2D> StarUVs;


	void DrawGameOptions(float RenderDelta, float& YOffset) override;
	virtual void DrawMinimap(float RenderDelta) override;
	virtual bool ShouldDrawScoringStats() override;

	virtual void DrawTeamPanel(float RenderDelta, float& YOffset);
	virtual void DrawScoreHeaders(float RenderDelta, float& YOffset);
	virtual void DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset);
	virtual void DrawPlayerScore(AUTPlayerState* PlayerState, float XOffset, float YOffset, float Width, FLinearColor DrawColor) override;

protected:
	void DrawBadge(AUTPlayerState* PlayerState, float XOffset, float YOffset);
*/
};