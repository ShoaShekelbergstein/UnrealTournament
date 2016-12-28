// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTUMGWidget.h"
#include "UTLocalPlayer.h"

UUTUMGWidget::UUTUMGWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayZOrder = 1.0f;
	WidgetTag = NAME_None;
}

void UUTUMGWidget::AssociateLocalPlayer(UUTLocalPlayer* NewLocalPlayer)
{
	UTPlayerOwner = NewLocalPlayer;
}

UUTLocalPlayer* UUTUMGWidget::GetPlayerOwner()
{
	return UTPlayerOwner;
}

void UUTUMGWidget::CloseWidget()
{
	if (UTPlayerOwner != nullptr)
	{
		UTPlayerOwner->CloseUMGWidget(this);
	}
}

