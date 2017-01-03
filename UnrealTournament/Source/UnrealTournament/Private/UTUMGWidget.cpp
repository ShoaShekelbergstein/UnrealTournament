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

void UUTUMGWidget::ShowParticalSystem(UParticleSystem* ParticalSystem, FVector2D ScreenLocation, bool bRelativeCoords, FVector LocationModifier, FVector DirectionModifier)
{
	if (UTPlayerOwner == nullptr || UTPlayerOwner->PlayerController == nullptr) return; // Quick out.  We need a local player to do this

	if (bRelativeCoords)
	{
		FVector2D ViewportSize = FVector2D(1.f, 1.f);
		UTPlayerOwner->ViewportClient->GetViewportSize(ViewportSize);	
		ScreenLocation *= ViewportSize;
	}

	FVector WorldLocation, WorldDirection;
	if ( UGameplayStatics::DeprojectScreenToWorld(UTPlayerOwner->PlayerController, ScreenLocation, WorldLocation, WorldDirection) )
	{
		WorldDirection += DirectionModifier;
		UGameplayStatics::SpawnEmitterAtLocation(UTPlayerOwner->PlayerController, ParticalSystem, WorldLocation + (WorldDirection.GetSafeNormal() * LocationModifier), WorldDirection.ToOrientationRotator());		
	}
}