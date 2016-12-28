// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTUMGWidget_Toast.h"

UUTUMGWidget_Toast::UUTUMGWidget_Toast(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayZOrder = 30000;
	Duration = 1.5f;
}

