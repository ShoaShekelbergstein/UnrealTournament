// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTHUD_CTF.h"
#include "UTGauntletHUD.generated.h"

class AUTGauntletGameState;

UCLASS()
class UNREALTOURNAMENT_API AUTGauntletHUD : public AUTHUD_CTF
{
	GENERATED_UCLASS_BODY()

	void DrawHUD();
	virtual void NotifyMatchStateChange() override;

protected:
	
	virtual void DrawScoreSummary(AUTGauntletGameState* GauntletGameState);
	virtual void DrawFadeToBlack(AUTGauntletGameState* GauntletGameState);
	virtual void DrawRoundAnnounce(AUTGauntletGameState* GauntletGameState);

	UPROPERTY()
	TArray<UTexture2D*> RoundMarkers;

	float FadeToBlackDuration;
	TArray<float> RoundAnnounceDurations;
	float AnimTimer;
	int32 AnimPhase;

	void DrawRoundImage(AUTGauntletGameState* GauntletGameState, float RoundScale, float GeneralOpacity, float FillOpacity);

	virtual void DrawPlayerIcons(AUTGauntletGameState* GauntletGameState);
	virtual void DrawPlayerIcon(AUTGauntletGameState* GauntletGameState, AUTPlayerState* PlayerState, float LiveScaling, float XOffset, float YOffset, float IconSize);
	virtual void GetPlayerListForIcons(AUTGauntletGameState* GauntletGameState, TArray<AUTPlayerState*>& SortedPlayers);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	FCanvasIcon RedTeamIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	FCanvasIcon BlueTeamIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	FCanvasIcon RedTeamOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	FCanvasIcon BlueTeamOverlay;

	int32 RedPlayerCount;
	int32 BluePlayerCount;


};