// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "Net/UnrealNetwork.h"
#include "UTCTFFlag.h"
#include "UTCTFGameMessage.h"

static FName NAME_Wipe(TEXT("Wipe"));

AUTCTFFlag::AUTCTFFlag(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bTeamPickupSendsHome = true;
	bEnemyCanPickup = true;
	MessageClass = UUTCTFGameMessage::StaticClass();
}

void AUTCTFFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTCTFFlag, CaptureEffectLoc);
}

void AUTCTFFlag::PlayCaptureEffect()
{
	if (Role == ROLE_Authority)
	{
		CaptureEffectLoc = GetActorLocation();
		ForceNetUpdate();
	}
	if (GetNetMode() != NM_DedicatedServer)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(this, CaptureEffect, CaptureEffectLoc - FVector(0.0f, 0.0f, Collision->GetUnscaledCapsuleHalfHeight()), GetActorRotation());
		if (PSC != NULL)
		{
			AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
			if (GS != NULL && GS->Teams.IsValidIndex(GetTeamNum()) && GS->Teams[GetTeamNum()] != NULL)
			{
				PSC->SetColorParameter(FName(TEXT("TeamColor")), GS->Teams[GetTeamNum()]->TeamColor);
			}
		}
	}
}

static FName SavedObjectState;

void AUTCTFFlag::PreNetReceive()
{
	Super::PreNetReceive();

	SavedObjectState = ObjectState;
}

void AUTCTFFlag::PostNetReceiveLocationAndRotation()
{
	if (ObjectState != SavedObjectState && ObjectState == CarriedObjectState::Home)
	{
		PlayReturnedEffects();
	}
	Super::PostNetReceiveLocationAndRotation();
}