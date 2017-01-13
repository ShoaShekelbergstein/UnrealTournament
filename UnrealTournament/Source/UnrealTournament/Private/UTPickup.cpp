// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTPickup.h"
#include "UnrealNetwork.h"
#include "UTRecastNavMesh.h"
#include "UTPickupMessage.h"
#include "UTWorldSettings.h"

FName NAME_Progress(TEXT("Progress"));
FName NAME_RespawnTime(TEXT("RespawnTime"));

void AUTPickup::PostEditImport()
{
	Super::PostEditImport();

	if (!IsPendingKill())
	{
		Collision->OnComponentBeginOverlap.Clear();
		Collision->OnComponentBeginOverlap.AddDynamic(this, &AUTPickup::OnOverlapBegin);
	}
}

AUTPickup::AUTPickup(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCanBeDamaged = false;

	Collision = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule"));
	Collision->SetCollisionProfileName(FName(TEXT("Pickup")));
	Collision->InitCapsuleSize(64.0f, 75.0f);
	Collision->bShouldUpdatePhysicsVolume = false;
	Collision->Mobility = EComponentMobility::Static;
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AUTPickup::OnOverlapBegin);
	RootComponent = Collision;

	TimerEffect = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("TimerEffect"));
	if (TimerEffect != NULL)
	{
		TimerEffect->SetHiddenInGame(true);
		TimerEffect->SetupAttachment(RootComponent);
		TimerEffect->LDMaxDrawDistance = 1024.0f;
		TimerEffect->RelativeLocation.Z = 40.0f;
		TimerEffect->Mobility = EComponentMobility::Static;
		TimerEffect->SetCastShadow(false);
	}
	BaseEffect = ObjectInitializer.CreateOptionalDefaultSubobject<UParticleSystemComponent>(this, TEXT("BaseEffect"));
	if (BaseEffect != NULL)
	{
		BaseEffect->SetupAttachment(RootComponent);
		BaseEffect->LDMaxDrawDistance = 2048.0f;
		BaseEffect->RelativeLocation.Z = -58.0f;
		BaseEffect->Mobility = EComponentMobility::Static;
	}
	TakenEffectTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	RespawnEffectTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	State.bActive = true;
	RespawnTime = 30.0f;
	SetReplicates(true);
	bAlwaysRelevant = true;
	NetUpdateFrequency = 1.0f;
	PrimaryActorTick.bCanEverTick = true;
	PickupMessageString = NSLOCTEXT("PickupMessage", "ItemPickedUp", "Item snagged.");
	bHasTacComView = false;
	TeamSide = 255;
	bOverride_TeamSide = false;
	IconColor = FLinearColor::White;
	bSpawnOncePerRound = false;
}

void AUTPickup::SetTacCom(bool bTacComEnabled)
{
	if (bHasTacComView && TimerEffect != NULL)
	{
		TimerEffect->LDMaxDrawDistance = bTacComEnabled ? 50000.f : 1024.f;
		TimerEffect->CachedMaxDrawDistance = TimerEffect->LDMaxDrawDistance;
		TimerEffect->MarkRenderStateDirty();
	}
}

void AUTPickup::BeginPlay()
{
	Super::BeginPlay();

	if (BaseEffect != NULL && BaseTemplateAvailable != NULL)
	{
		BaseEffect->SetTemplate(BaseTemplateAvailable);
	}

	AUTRecastNavMesh* NavData = GetUTNavData(GetWorld());
	if (NavData != NULL)
	{
		NavData->AddToNavigation(this);
	}
}

void AUTPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearAllTimersForObject(this);
	AUTRecastNavMesh* NavData = GetUTNavData(GetWorld());
	if (NavData != NULL)
	{
		NavData->RemoveFromNavigation(this);
	}
}

FCanvasIcon AUTPickup::GetMinimapIcon() const
{
	return (MinimapIcon.Texture != NULL) ? MinimapIcon : HUDIcon;
}

void AUTPickup::Reset_Implementation()
{
	bHasSpawnedThisRound = false;
	GetWorld()->GetTimerManager().ClearTimer(WakeUpTimerHandle);
	if (bDelayedSpawn)
	{
		State.bRepTakenEffects = false;
		StartSleeping();
	}
	else if (!State.bActive)
	{
		WakeUp();
	}
	bReplicateReset = !bReplicateReset;
}

void AUTPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	APawn* P = Cast<APawn>(OtherActor);
	if (P != NULL && !P->bTearOff && !GetWorld()->LineTraceTestByChannel(P->GetActorLocation(), GetActorLocation(), ECC_Pawn, FCollisionQueryParams(), WorldResponseParams))
	{
		ProcessTouch(P);
	}
}

bool AUTPickup::FlashOnMinimap_Implementation()
{
	return false;
}

bool AUTPickup::AllowPickupBy_Implementation(APawn* Other, bool bDefaultAllowPickup)
{
	AUTCharacter* UTC = Cast<AUTCharacter>(Other);
	bDefaultAllowPickup = bDefaultAllowPickup && (UTC == NULL || UTC->bCanPickupItems);
	bool bAllowPickup = bDefaultAllowPickup;
	AUTGameMode* UTGameMode = GetWorld()->GetAuthGameMode<AUTGameMode>();
	return (UTGameMode == NULL || !UTGameMode->OverridePickupQuery(Other, NULL, this, bAllowPickup)) ? bDefaultAllowPickup : bAllowPickup;
}

void AUTPickup::ProcessTouch_Implementation(APawn* TouchedBy)
{
	if (Role == ROLE_Authority && State.bActive && TouchedBy->Controller != NULL && AllowPickupBy(TouchedBy, true))
	{
		GiveTo(TouchedBy);
		AUTGameMode* UTGameMode = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (UTGameMode != NULL)
		{
			AUTPlayerState* PickedUpBy = Cast<AUTPlayerState>(TouchedBy->PlayerState);
			UTGameMode->ScorePickup(this, PickedUpBy, LastPickedUpBy);
			LastPickedUpBy = PickedUpBy;

			if (UTGameMode->NumBots > 0)
			{
				float Radius = 0.0f;
				if (TakenSound != NULL)
				{
					Radius = TakenSound->GetMaxAudibleDistance();
					const FAttenuationSettings* Settings = TakenSound->GetAttenuationSettingsToApply();
					if (Settings != NULL)
					{
						Radius = FMath::Max<float>(Radius, Settings->GetMaxDimension());
					}
				}
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					if (It->IsValid())
					{
						AUTBot* B = Cast<AUTBot>(It->Get());
						if (B != NULL)
						{
							B->NotifyPickup(TouchedBy, this, Radius);
						}
					}
				}
			}
		}

		PlayTakenEffects(true);
		StartSleeping();
	}
}

void AUTPickup::GiveTo_Implementation(APawn* Target)
{
	AUTPlayerController* UTPC = (Target != nullptr) ? Cast<AUTPlayerController>(Target->GetController()) : nullptr;
	if (UTPC)
	{
		UTPC->ClientReceiveLocalizedMessage(UUTPickupMessage::StaticClass(), 0, NULL, NULL, GetClass());
		AUTGameMode* GameMode = GetWorld()->GetAuthGameMode<AUTGameMode>();
		if (GameMode && GameMode->bBasicTrainingGame && !GameMode->bDamageHurtsHealth && (GetNetMode() == NM_Standalone))
		{
			for (int32 Index = 0; Index < TutorialAnnouncements.Num(); Index++)
			{
				UTPC->PlayTutorialAnnouncement(Index, this);
			}
		}
	}
}

void AUTPickup::SetPickupHidden(bool bNowHidden)
{
	if (TakenHideTags.Num() == 0 || RootComponent == NULL)
	{
		SetActorHiddenInGame(bNowHidden);
	}
	else
	{
		TArray<USceneComponent*> Components;
		RootComponent->GetChildrenComponents(true, Components);
		for (int32 i = 0; i < Components.Num(); i++)
		{
			for (int32 j = 0; j < TakenHideTags.Num(); j++)
			{
				if (Components[i]->ComponentHasTag(TakenHideTags[j]))
				{
					Components[i]->SetVisibility(!bNowHidden);
				}
			}
		}
	}
}

void AUTPickup::StartSleeping_Implementation()
{
	SetPickupHidden(true);
	SetActorEnableCollision(false);
	if (RespawnTime > 0.0f)
	{
		if (bSpawnOncePerRound && bHasSpawnedThisRound)
		{
			TimerEffect->SetFloatParameter(NAME_Progress, 0.0f);
			TimerEffect->SetFloatParameter(NAME_RespawnTime, RespawnTime);
			TimerEffect->SetHiddenInGame(true);
		}
		else
		{
			if (!bFixedRespawnInterval || !GetWorld()->GetTimerManager().IsTimerActive(WakeUpTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(WakeUpTimerHandle, this, &AUTPickup::WakeUpTimer, RespawnTime, false);
			}
			if (TimerEffect != NULL && TimerEffect->Template != NULL)
			{
				// FIXME: workaround for particle bug; screen facing particles don't handle negative scale correctly
				FVector FixedScale = TimerEffect->GetComponentScale();
				FixedScale = FVector(FMath::Abs<float>(FixedScale.X), FMath::Abs<float>(FixedScale.Y), FMath::Abs<float>(FixedScale.Z));
				TimerEffect->SetWorldScale3D(FixedScale);

				TimerEffect->SetFloatParameter(NAME_Progress, 0.0f);
				TimerEffect->SetFloatParameter(NAME_RespawnTime, RespawnTime);
				TimerEffect->SetHiddenInGame(false);
				PrimaryActorTick.SetTickFunctionEnable(true);
			}
		}
	}

	// this needs to be done redundantly because not all paths for all pickups call both StartSleeping() and PlayTakenEffects()
	if (BaseEffect != NULL && BaseTemplateTaken != NULL)
	{
		BaseEffect->SetTemplate(BaseTemplateTaken);
	}

	if (Role == ROLE_Authority)
	{
		State.bActive = false;
		State.ChangeCounter++;
		ForceNetUpdate();
	}
}
void AUTPickup::PlayTakenEffects(bool bReplicate)
{
	if (bReplicate && Role == ROLE_Authority)
	{
		State.bRepTakenEffects = true;
		ForceNetUpdate();
	}
	if (GetNetMode() != NM_DedicatedServer)
	{
		AUTWorldSettings* WS = Cast<AUTWorldSettings>(GetWorld()->GetWorldSettings());
		if (WS == NULL || WS->EffectIsRelevant(this, GetActorLocation(), true, false, 10000.0f, 1000.0f, false))
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(TakenParticles, RootComponent, NAME_None, TakenEffectTransform.GetLocation(), TakenEffectTransform.GetRotation().Rotator());
			if (PSC != NULL)
			{
				PSC->SetRelativeScale3D(TakenEffectTransform.GetScale3D());
			}
		}
		if (BaseEffect != NULL && BaseTemplateTaken != NULL)
		{
			BaseEffect->SetTemplate(BaseTemplateTaken);
		}
		UUTGameplayStatics::UTPlaySound(GetWorld(), TakenSound, this, SRT_None, false, FVector::ZeroVector, NULL, NULL, false);
	}
}
void AUTPickup::WakeUp_Implementation()
{
	bHasSpawnedThisRound = true;
	SetPickupHidden(false);
	GetWorld()->GetTimerManager().ClearTimer(WakeUpTimerHandle);
	if (bFixedRespawnInterval && !bSpawnOncePerRound)
	{
		// start timer for next time
		GetWorld()->GetTimerManager().SetTimer(WakeUpTimerHandle, this, &AUTPickup::WakeUpTimer, RespawnTime, false);
		if (bFixedRespawnInterval && Role == ROLE_Authority)
		{
			bReplicateReset = !bReplicateReset;
		}
	}

	PrimaryActorTick.SetTickFunctionEnable(GetClass()->GetDefaultObject<AUTPickup>()->PrimaryActorTick.bStartWithTickEnabled);
	if (TimerEffect != NULL)
	{
		TimerEffect->SetHiddenInGame(true);
	}

	if (Role == ROLE_Authority)
	{
		State.bActive = true;
		State.bRepTakenEffects = false;
		State.ChangeCounter++;
		ForceNetUpdate();
		LastRespawnTime = GetWorld()->TimeSeconds;
	}

	PlayRespawnEffects();

	// last so if a player is already touching we're fully ready to act on it
	SetActorEnableCollision(true);
}
void AUTPickup::WakeUpTimer()
{
	if (Role == ROLE_Authority)
	{
		if (!bFixedRespawnInterval || !State.bActive)
		{
			WakeUp();
		}
	}
	else if (!bFixedRespawnInterval)
	{
		// it's possible we're out of sync, so set up a state that indicates the pickup should respawn any time now, but isn't yet available
		if (TimerEffect != NULL)
		{
			TimerEffect->SetFloatParameter(NAME_Progress, 0.99f);
		}
	}
}
void AUTPickup::PlayRespawnEffects()
{
	// TODO: EffectIsRelevant() ?
	if (GetNetMode() != NM_DedicatedServer)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(RespawnParticles, RootComponent, NAME_None, RespawnEffectTransform.GetLocation(), RespawnEffectTransform.GetRotation().Rotator());
		if (PSC != NULL)
		{
			PSC->SetRelativeScale3D(RespawnEffectTransform.GetScale3D());
		}
		if (BaseEffect != NULL && BaseTemplateAvailable != NULL)
		{
			BaseEffect->SetTemplate(BaseTemplateAvailable);
		}
		UUTGameplayStatics::UTPlaySound(GetWorld(), RespawnSound, this, SRT_None);
	}
}

float AUTPickup::GetRespawnTimeOffset(APawn* Asker) const
{
	if (State.bActive)
	{
		return LastRespawnTime - GetWorld()->TimeSeconds;
	}
	else
	{
		float OutRespawnTime = GetWorldTimerManager().GetTimerRemaining(WakeUpTimerHandle);
		return (OutRespawnTime <= 0.0f) ? FLT_MAX : OutRespawnTime;
	}
}

void AUTPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (TimerEffect && (RespawnTime > 0.0f) && !State.bActive && World->GetTimerManager().IsTimerActive(WakeUpTimerHandle))
	{
		TimerEffect->SetFloatParameter(NAME_Progress, 1.0f - World->GetTimerManager().GetTimerRemaining(WakeUpTimerHandle) / RespawnTime);
	}
}

float AUTPickup::BotDesireability_Implementation(APawn* Asker, AController* RequestOwner, float PathDistance)
{
	return BaseDesireability;
}
float AUTPickup::DetourWeight_Implementation(APawn* Asker, float PathDistance)
{
	return 0.0f;
}
bool AUTPickup::IsSuperDesireable_Implementation(AController* RequestOwner, float CalculatedDesire)
{
	return FMath::Max<float>(BaseDesireability, CalculatedDesire) >= 1.0f;
}

static FPickupReplicatedState PreRepState;

void AUTPickup::PreNetReceive()
{
	PreRepState = State;
	Super::PreNetReceive();
}
void AUTPickup::PostNetReceive()
{
	Super::PostNetReceive();

	// make sure not to re-invoke WakeUp()/StartSleeping() if only bRepTakenEffects has changed
	// since that will reset timers on the client incorrectly
	if (PreRepState.bActive != State.bActive || PreRepState.ChangeCounter != State.ChangeCounter)
	{
		if (State.bActive)
		{
			WakeUp();
		}
		else
		{
			StartSleeping();
		}
	}
	if (!State.bActive && State.bRepTakenEffects)
	{
		PlayTakenEffects(true);
	}
}

void AUTPickup::OnRep_RespawnTimeRemaining()
{
	if (!State.bActive && (RespawnTimeRemaining != GetWorld()->GetTimerManager().GetTimerRemaining(WakeUpTimerHandle)))
	{
		GetWorld()->GetTimerManager().SetTimer(WakeUpTimerHandle, this, &AUTPickup::WakeUpTimer, RespawnTimeRemaining, false);
	}
}

void AUTPickup::OnRep_Reset()
{
	// this is only important on non-initial cases as RespawnTimeRemaining handles the others
	if (bFixedRespawnInterval && CreationTime < GetWorld()->TimeSeconds)
	{
		GetWorld()->GetTimerManager().SetTimer(WakeUpTimerHandle, this, &AUTPickup::WakeUpTimer, RespawnTime, false);
	}
}

void AUTPickup::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	RespawnTimeRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(WakeUpTimerHandle);
}

void AUTPickup::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUTPickup, RespawnTime);
	DOREPLIFETIME(AUTPickup, bFixedRespawnInterval);
	// warning: we rely on this ordering
	DOREPLIFETIME(AUTPickup, bReplicateReset);
	DOREPLIFETIME(AUTPickup, bSpawnOncePerRound);
	DOREPLIFETIME(AUTPickup, bHasSpawnedThisRound);
	DOREPLIFETIME(AUTPickup, State);
	DOREPLIFETIME_CONDITION(AUTPickup, RespawnTimeRemaining, COND_InitialOnly);
}

void AUTPickup::PrecacheTutorialAnnouncements(UUTAnnouncer* Announcer) const
{
	for (int32 i = 0; i < TutorialAnnouncements.Num(); i++)
	{
		Announcer->PrecacheAnnouncement(TutorialAnnouncements[i]);
	}
}

FName AUTPickup::GetTutorialAnnouncement(int32 Switch) const
{
	return (Switch < TutorialAnnouncements.Num()) ? TutorialAnnouncements[Switch] : NAME_None;
}
