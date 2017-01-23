// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTUMGWidget.h"
#include "UTLocalPlayer.h"

UUTUMGWidget::UUTUMGWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayZOrder = 1.0f;
	WidgetTag = NAME_None;

	bUniqueUMG = false;
	StackingOffset = FVector2D(0.0f, -16.0f);
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

void UUTUMGWidget::ShowParticleSystem(UParticleSystem* ParticleSystem, FVector2D ScreenLocation, bool bRelativeCoords, FVector LocationModifier, FRotator DirectionModifier)
{
	if (UTPlayerOwner == nullptr || UTPlayerOwner->PlayerController == nullptr) return; // Quick out.  We need a local player to do this

	//TODO: Make the screenlocation work with the UMG/Slate scaling system

	FVector2D ViewportSize = FVector2D(1.f, 1.f);
	UTPlayerOwner->ViewportClient->GetViewportSize(ViewportSize);	

	if (bRelativeCoords)
	{
		ScreenLocation *= ViewportSize;
	}
	else
	{
		float Aspect = 1.7777777f;	// 16:9
		float XScale = ViewportSize.X / 1920.0f;
		float YScale =XScale / Aspect;
	
		ScreenLocation.X *= XScale;
		ScreenLocation.Y *= YScale;
	}

	FVector WorldLocation, WorldDirection;
	if ( UGameplayStatics::DeprojectScreenToWorld(UTPlayerOwner->PlayerController, ScreenLocation, WorldLocation, WorldDirection) )
	{
		FVector FinalLocation = WorldLocation + WorldDirection.ToOrientationQuat().RotateVector(LocationModifier);
		FRotator FinalRotation = (WorldDirection + DirectionModifier.Vector()).ToOrientationRotator();
		UGameplayStatics::SpawnEmitterAtLocation(UTPlayerOwner->PlayerController, ParticleSystem, FinalLocation, FinalRotation);		
	}
}