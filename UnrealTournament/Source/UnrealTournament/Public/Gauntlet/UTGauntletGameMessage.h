// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTLocalMessage.h"
#include "UTGauntletGameMessage.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTGauntletGameMessage : public UUTLocalMessage
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FText FlagSpawnMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FText InitialFlagSpawnMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FText YouAreOnOffenseMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FText YouAreOnDefenseMessage;

	virtual FText GetText(int32 Switch = 0, bool bTargetsPlayerState1 = false, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL) const override;

	virtual FString GetAnnouncementUMGClassname(int32 Switch, const UObject* OptionalObject) const override;

	virtual float GetLifeTime(int32 Switch) const;

};


