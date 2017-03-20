// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLineUpZone.h"
#include "UTCharacter.h"


AUTLineUpZone::AUTLineUpZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSnapToFloor = false;
	SnapFloorOffset = 95.f;

	SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	RootComponent = SceneRoot;
	RootComponent->Mobility = EComponentMobility::Movable;

	ZoneType = LineUpTypes::Invalid;

	bIsTeamSpawnList = true;

	// Create a CameraComponent	
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("LineUpCamera"));
	if (Camera)
	{
		Camera->SetupAttachment(RootComponent);

		FTransform CameraTransform;
		CameraTransform.SetTranslation(FVector(-200.f, 0.f, 30.f));
		Camera->SetFieldOfView(60.f);

		FRotator DefaultCameraRotation;
		DefaultCameraRotation.Roll = 0.f;
		DefaultCameraRotation.Pitch = 0.f;
		DefaultCameraRotation.Yaw = 0.f;

		FTransform DefaultCameraTransform;
		DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 47.5f));
		DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

		Camera->SetRelativeTransform(DefaultCameraTransform);
	}

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	if (SpriteComponent)
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteObject(TEXT("/Game/RestrictedAssets/EditorAssets/Icons/lineup_marker.lineup_marker"));
		SpriteComponent->Sprite = SpriteObject.Get();
			
		SpriteComponent->SpriteInfo.Category = FName(TEXT("Notes"));
		SpriteComponent->SpriteInfo.DisplayName = FText(NSLOCTEXT("SpriteCategory", "Notes", "Notes"));
	
		SpriteComponent->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->Mobility = EComponentMobility::Movable;
	}
#endif //WITH_EDITORONLY_DATA
}

void AUTLineUpZone::Destroyed()
{
	DeleteAllMeshVisualizations();
}

#if WITH_EDITOR

void AUTLineUpZone::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		InitializeMeshVisualizations();
	}
}

void AUTLineUpZone::PostEditMove(bool bFinished)
{
	if (bFinished)
	{
		if (bSnapToFloor)
		{
			SnapToFloor();
		}
		else
		{
			UpdateMeshVisualizations();
		}
	}

	Super::PostEditMove(bFinished);
}

void AUTLineUpZone::SetIsTemporarilyHiddenInEditor(bool bIsHidden)
{
	Super::SetIsTemporarilyHiddenInEditor(bIsHidden);

	//Delete all the mesh visualizations if we are hiding the line up in editor
	if (bIsHidden)
	{
		DeleteAllMeshVisualizations();
	}
	else
	{
		InitializeMeshVisualizations();
	}
}

void AUTLineUpZone::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("RedAndWinningTeamSpawnLocations")))
	{
		UpdateMeshVisualizations();
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("BlueAndLosingTeamSpawnLocations")))
	{
		UpdateMeshVisualizations();
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("FFATeamSpawnLocations")))
	{
		UpdateMeshVisualizations();
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("ZoneType")))
	{
		if (bIsTeamSpawnList)
		{
			if (ZoneType == LineUpTypes::Intro)
			{
				DefaultCreateForTeamIntro();
			}
			else if (ZoneType == LineUpTypes::Intermission)
			{
				DefaultCreateForTeamIntermission();
			}
			else if (ZoneType == LineUpTypes::PostMatch)
			{
				DefaultCreateForTeamEndMatch();
			}
		}
		else
		{
			if (ZoneType == LineUpTypes::Intro)
			{
				DefaultCreateForFFAIntro();
			}
			else if (ZoneType == LineUpTypes::Intermission)
			{
				DefaultCreateForFFAIntermission();
			}
			else if (ZoneType == LineUpTypes::PostMatch)
			{
				DefaultCreateForFFAEndMatch();
			}
		}
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("bSnapToFloor")))
	{
		if (bSnapToFloor)
		{
			SnapToFloor();
		}
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("bIsTeamSpawnList")))
	{
		if (bIsTeamSpawnList)
		{
			if (ZoneType == LineUpTypes::Intro)
			{
				DefaultCreateForTeamIntro();
			}
			else if (ZoneType == LineUpTypes::Intermission)
			{
				DefaultCreateForTeamIntermission();
			}
			else if (ZoneType == LineUpTypes::PostMatch)
			{
				DefaultCreateForTeamEndMatch();
			}
		}
		else
		{
			if (ZoneType == LineUpTypes::Intro)
			{
				DefaultCreateForFFAIntro();
			}
			else if (ZoneType == LineUpTypes::Intermission)
			{
				DefaultCreateForFFAIntermission();
			}
			else if (ZoneType == LineUpTypes::PostMatch)
			{
				DefaultCreateForFFAEndMatch();
			}
		}
	}

	if (PropertyChangedEvent.Property != NULL && PropertyChangedEvent.Property->GetFName() == FName(TEXT("SceneRoot")))
	{
		if (RootComponent->IsVisible())
		{
			InitializeMeshVisualizations();
		}
		else
		{
			DeleteAllMeshVisualizations();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AUTLineUpZone::PreEditUndo()
{
	//Sometimes an edit undo will leave mesh visualizations stranded due to a change
	//Go ahead and delete them all so we don't leave any behind in the level
	DeleteAllMeshVisualizations();
	Super::PreEditUndo();
}

void AUTLineUpZone::PostEditUndo()
{
	//We deleted all mesh visualiztions in preeditundo, so now recreate them after the undo.
	InitializeMeshVisualizations();
	Super::PostEditUndo();
}
#endif

void AUTLineUpZone::UpdateMeshVisualizations()
{
#if WITH_EDITORONLY_DATA
	//If visualization counts don't match up wipe them and start over
	if ((RedAndWinningTeamSpawnLocations.Num() + BlueAndLosingTeamSpawnLocations.Num() + FFATeamSpawnLocations.Num()) != MeshVisualizations.Num())
	{
		DeleteAllMeshVisualizations();
		InitializeMeshVisualizations();
		return;
	}

	int MeshIndex = 0;
	for (int RedTeamIndex = 0; ((RedTeamIndex < RedAndWinningTeamSpawnLocations.Num()) && (MeshVisualizations.Num() > MeshIndex)) ; ++RedTeamIndex)
	{
		if (MeshVisualizations[MeshIndex])
		{
			MeshVisualizations[MeshIndex]->GetRootComponent()->SetRelativeTransform(RedAndWinningTeamSpawnLocations[RedTeamIndex]);
			++MeshIndex;
		}
	}

	for (int BlueTeamIndex = 0; ((BlueTeamIndex < BlueAndLosingTeamSpawnLocations.Num()) && (MeshVisualizations.Num() > MeshIndex)); ++BlueTeamIndex)
	{
		if (MeshVisualizations[MeshIndex])
		{
			MeshVisualizations[MeshIndex]->GetRootComponent()->SetRelativeTransform(BlueAndLosingTeamSpawnLocations[BlueTeamIndex]);
			++MeshIndex;
		}
	}

	for (int FFAIndex = 0; ((FFAIndex < FFATeamSpawnLocations.Num()) && (MeshVisualizations.Num() > MeshIndex)); ++FFAIndex)
	{
		if (MeshVisualizations[MeshIndex])
		{
			MeshVisualizations[MeshIndex]->GetRootComponent()->SetRelativeTransform(FFATeamSpawnLocations[FFAIndex]);
			++MeshIndex;
		}
	}
#endif
}

void AUTLineUpZone::DeleteAllMeshVisualizations()
{
#if WITH_EDITORONLY_DATA
	for (int index = 0; index < MeshVisualizations.Num(); ++index)
	{
		if (MeshVisualizations[index])
		{
			//MeshVisualizations[index]->DestroyComponent();
			MeshVisualizations[index]->DetachRootComponentFromParent(true);
			MeshVisualizations[index]->Instigator = nullptr;
			MeshVisualizations[index]->SetOwner(nullptr);
			MeshVisualizations[index]->Destroy(true, true);
		}
	}

	MeshVisualizations.Empty();
#endif
}

void AUTLineUpZone::InitializeMeshVisualizations()
{
#if WITH_EDITORONLY_DATA
	DeleteAllMeshVisualizations();

	if ((EditorVisualizationCharacter == nullptr) || (GetWorld() == nullptr) || (!RootComponent->IsVisible()) || (IsTemporarilyHiddenInEditor()))
	{
		return;
	}

	for (int RedIndex = 0; RedIndex < RedAndWinningTeamSpawnLocations.Num(); ++RedIndex)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		AUTLineUpZoneVisualizationCharacter* SpawnedActor = GetWorld()->SpawnActor<AUTLineUpZoneVisualizationCharacter>(EditorVisualizationCharacter, RedAndWinningTeamSpawnLocations[RedIndex], Params);
		if (SpawnedActor)
		{
			MeshVisualizations.Add(SpawnedActor);
			SpawnedActor->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			SpawnedActor->TeamNum = 0;
			SpawnedActor->OnChangeTeamNum();
		}
	}

	for (int BlueIndex = 0; BlueIndex < BlueAndLosingTeamSpawnLocations.Num(); ++BlueIndex)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		AUTLineUpZoneVisualizationCharacter* SpawnedActor = GetWorld()->SpawnActor<AUTLineUpZoneVisualizationCharacter>(EditorVisualizationCharacter, BlueAndLosingTeamSpawnLocations[BlueIndex], Params);
		if (SpawnedActor)
		{
			MeshVisualizations.Add(SpawnedActor);
			SpawnedActor->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			SpawnedActor->TeamNum = 1;
			SpawnedActor->OnChangeTeamNum();
		}
	}

	for (int FFAIndex = 0; FFAIndex < FFATeamSpawnLocations.Num(); ++FFAIndex)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		AUTLineUpZoneVisualizationCharacter* SpawnedActor = GetWorld()->SpawnActor<AUTLineUpZoneVisualizationCharacter>(EditorVisualizationCharacter, FFATeamSpawnLocations[FFAIndex], Params);
		if (SpawnedActor)
		{
			MeshVisualizations.Add(SpawnedActor);
			SpawnedActor->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
			SpawnedActor->TeamNum = 255;
			SpawnedActor->OnChangeTeamNum();
		}
	}
#endif
}

void AUTLineUpZone::SnapToFloor()
{
	static const FName NAME_FreeCam = FName(TEXT("FreeCam"));

	if (bSnapToFloor)
	{
		//Move base LineUpZone actor to snapped floor position
		{
			FTransform TestLocation = ActorToWorld();

			FVector Start(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z + 500.f);
			FVector End(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z - 10000.f);

			FHitResult Hit;
			GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, COLLISION_TRACE_WEAPON, FCollisionShape::MakeBox(FVector(12.f)), FCollisionQueryParams(NAME_FreeCam, false, this));
			if (Hit.bBlockingHit)
			{
				FVector NewLocation = Hit.Location;
				NewLocation.Z += SnapFloorOffset;
				SetActorLocation(NewLocation);
			}
		}

		//Move all Red/Winning team spawns
		for (FTransform& Location : RedAndWinningTeamSpawnLocations)
		{
			FTransform TestLocation = Location * ActorToWorld();

			FVector Start(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z + 500.0f);
			FVector End(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z - 10000.0f);

			FHitResult Hit;
			GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, COLLISION_TRACE_WEAPON, FCollisionShape::MakeBox(FVector(12.f)), FCollisionQueryParams(NAME_FreeCam, false, this));
			if (Hit.bBlockingHit)
			{
				FVector NewLocation = Location.GetLocation();
				NewLocation.Z = (Hit.Location - GetActorLocation()).Z + SnapFloorOffset;
				Location.SetLocation(NewLocation);
			}
		}

		//Move all Blue/Losing team spawns
		for (FTransform& Location : BlueAndLosingTeamSpawnLocations)
		{
			FTransform TestLocation = Location * ActorToWorld();

			FVector Start(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z + 500.0f);
			FVector End(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z - 10000.0f);

			FHitResult Hit;
			GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, COLLISION_TRACE_WEAPON, FCollisionShape::MakeBox(FVector(12.f)), FCollisionQueryParams(NAME_FreeCam, false, this));
			if (Hit.bBlockingHit)
			{
				FVector NewLocation = Location.GetLocation();
				NewLocation.Z = (Hit.Location - GetActorLocation()).Z + SnapFloorOffset;
				Location.SetLocation(NewLocation);
			}
		}

		//Move all FFA spawns
		for (FTransform& Location : FFATeamSpawnLocations)
		{
			FTransform TestLocation = Location * ActorToWorld();

			FVector Start(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z + 500.0f);
			FVector End(TestLocation.GetTranslation().X, TestLocation.GetTranslation().Y, TestLocation.GetTranslation().Z - 10000.0f);

			FHitResult Hit;
			GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, COLLISION_TRACE_WEAPON, FCollisionShape::MakeBox(FVector(12.f)), FCollisionQueryParams(NAME_FreeCam, false, this));
			if (Hit.bBlockingHit)
			{
				FVector NewLocation = Location.GetLocation();
				NewLocation.Z = (Hit.Location - GetActorLocation()).Z + SnapFloorOffset;
				Location.SetLocation(NewLocation);
			}
		}

		UpdateMeshVisualizations();
	}
}

void AUTLineUpZone::UpdateSpawnLocationsWithVisualizationMove()
{
#if WITH_EDITORONLY_DATA
	
	int RedIndex = 0;
	int BlueIndex = 0;
	int FFAIndex = 0;

	for (int MeshIndex = 0; MeshIndex < MeshVisualizations.Num(); ++MeshIndex)
	{
		AUTLineUpZoneVisualizationCharacter* Mesh = Cast<AUTLineUpZoneVisualizationCharacter>(MeshVisualizations[MeshIndex]);
		if (Mesh)
		{
			if ((Mesh->TeamNum == 0) && (RedAndWinningTeamSpawnLocations.Num() > RedIndex))
			{
				RedAndWinningTeamSpawnLocations[RedIndex] = MeshVisualizations[MeshIndex]->GetRootComponent()->GetRelativeTransform();
				++RedIndex;
			}
			else if ((Mesh->TeamNum == 1) && (BlueAndLosingTeamSpawnLocations.Num() > BlueIndex))
			{
				BlueAndLosingTeamSpawnLocations[BlueIndex] = MeshVisualizations[MeshIndex]->GetRootComponent()->GetRelativeTransform();
				++BlueIndex;
			}
			else if (FFATeamSpawnLocations.Num() > FFAIndex)
			{
				FFATeamSpawnLocations[FFAIndex] = MeshVisualizations[MeshIndex]->GetRootComponent()->GetRelativeTransform();
				++FFAIndex;
			}
		}
	}

	if (bSnapToFloor)
	{
		SnapToFloor();
	}
#endif
}

void AUTLineUpZone::DefaultCreateForTeamIntro()
{
	TArray<FVector> RedStartLocations;
	TArray<FVector> BlueStartLocations;

	RedStartLocations.SetNum(5);
	BlueStartLocations.SetNum(5);

	RedStartLocations[0] = FVector(-150.f, -275.f, 0.0f);
	RedStartLocations[1] = FVector(-50.f, -250.f, 0.0f);
	RedStartLocations[2] = FVector(50.f, -200.f, 0.0f);
	RedStartLocations[3] = FVector(150.f, -150.f, 0.0f);
	RedStartLocations[4] = FVector(250.f, -100.f, 0.0f);

	BlueStartLocations[0] = FVector(-150.f, 275.f, 0.0f);
	BlueStartLocations[1] = FVector(-50.f, 250.f, 0.0f);
	BlueStartLocations[2] = FVector(50.f, 200.f, 0.0f);
	BlueStartLocations[3] = FVector(150.f, 150.f, 0.0f);
	BlueStartLocations[4] = FVector(250.f, 100.f, 0.0f);

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	RedAndWinningTeamSpawnLocations.SetNum(5);
	BlueAndLosingTeamSpawnLocations.SetNum(5);

	FRotator BlueWarpRotation;
	BlueWarpRotation.Pitch = 0.f;
	BlueWarpRotation.Roll = 0.f;
	BlueWarpRotation.Yaw = -135.f;

	FRotator RedWarpRotation;
	RedWarpRotation.Pitch = 0.f;
	RedWarpRotation.Roll = 0.f;
	RedWarpRotation.Yaw = 135.f;

	for (int RedIndex = 0; RedIndex < 5; ++RedIndex)
	{
		RedAndWinningTeamSpawnLocations[RedIndex].SetTranslation(RedStartLocations[RedIndex]);
		RedAndWinningTeamSpawnLocations[RedIndex].SetRotation(RedWarpRotation.Quaternion());
	}

	for (int BlueIndex = 0; BlueIndex < 5; ++BlueIndex)
	{
		BlueAndLosingTeamSpawnLocations[BlueIndex].SetTranslation(BlueStartLocations[BlueIndex]);
		BlueAndLosingTeamSpawnLocations[BlueIndex].SetRotation(BlueWarpRotation.Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;
	
	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForFFAIntro()
{
	TArray<FVector> FFAStartLocations;

	FFAStartLocations.SetNum(8);

	FFAStartLocations[0] = FVector(50.0f,50.0f, 0.0f);
	FFAStartLocations[1] = FVector(50.0,-50.0f, 0.0f);
	FFAStartLocations[2] = FVector(25.0f, 150.0f, 0.0f);
	FFAStartLocations[3] = FVector(25.0f,-150.0f, 0.0f);
	FFAStartLocations[4] = FVector(0.0f, 250.0f, 0.0f);
	FFAStartLocations[5] = FVector(0.0f,-250.0f, 0.0f);
	FFAStartLocations[6] = FVector(-25.0f,350.0f, 0.0f);
	FFAStartLocations[7] = FVector(-25.0f,-350.0f, 0.0f);


	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	FFATeamSpawnLocations.SetNum(FFAStartLocations.Num());

	FRotator Rotation;
	Rotation.Pitch = 0.f;
	Rotation.Roll = 0.f;
	Rotation.Yaw = -180.f;

	for (int FFAIndex = 0; FFAIndex < FFAStartLocations.Num(); ++FFAIndex)
	{
		FFATeamSpawnLocations[FFAIndex].SetTranslation(FFAStartLocations[FFAIndex]);
		FFATeamSpawnLocations[FFAIndex].SetRotation(Rotation.Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForTeamIntermission()
{
	TArray<FVector> WinningTeamStartLocations;
	TArray<FRotator> WinningTeamRotations;

	WinningTeamStartLocations.SetNum(5);
	WinningTeamRotations.SetNum(5);

	WinningTeamStartLocations[0] = FVector(-50.f, 0.f, 0.0f);
	WinningTeamRotations[0].Pitch = 0.f;
	WinningTeamRotations[0].Roll = 0.f;
	WinningTeamRotations[0].Yaw = -179.99f;

	WinningTeamStartLocations[1] = FVector(-75.f, 150.f, 0.0f);
	WinningTeamRotations[1].Pitch = 0.f;
	WinningTeamRotations[1].Roll = 0.f;
	WinningTeamRotations[1].Yaw = -168.75f;

	WinningTeamStartLocations[2] = FVector(-75.f, -150.f, 0.0f);
	WinningTeamRotations[2].Pitch = 0.f;
	WinningTeamRotations[2].Roll = 0.f;
	WinningTeamRotations[2].Yaw = 168.75f;

	WinningTeamStartLocations[3] = FVector(-150.f, 250.f, 0.0f);
	WinningTeamRotations[3].Pitch = 0.f;
	WinningTeamRotations[3].Roll = 0.f;
	WinningTeamRotations[3].Yaw = -157.49f;

	WinningTeamStartLocations[4] = FVector(-150.f, -250.f, 0.0f);
	WinningTeamRotations[4].Pitch = 0.f;
	WinningTeamRotations[4].Roll = 0.f;
	WinningTeamRotations[4].Yaw = 157.49f;

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	RedAndWinningTeamSpawnLocations.SetNum(5);

	for (int WinIndex = 0; WinIndex < 5; ++WinIndex)
	{
		RedAndWinningTeamSpawnLocations[WinIndex].SetTranslation(WinningTeamStartLocations[WinIndex]);
		RedAndWinningTeamSpawnLocations[WinIndex].SetRotation(WinningTeamRotations[WinIndex].Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForFFAIntermission()
{
	TArray<FVector> FFAStartLocations;
	TArray<FRotator> FFARotations;

	FFAStartLocations.SetNum(5);
	FFARotations.SetNum(5);

	FFAStartLocations[0] = FVector(-50.f, 0.f, 0.0f);
	FFARotations[0].Pitch = 0.f;
	FFARotations[0].Roll = 0.f;
	FFARotations[0].Yaw = -179.99f;

	FFAStartLocations[1] = FVector(-75.f, 150.f, 0.0f);
	FFARotations[1].Pitch = 0.f;
	FFARotations[1].Roll = 0.f;
	FFARotations[1].Yaw = -168.75f;

	FFAStartLocations[2] = FVector(-75.f, -150.f, 0.0f);
	FFARotations[2].Pitch = 0.f;
	FFARotations[2].Roll = 0.f;
	FFARotations[2].Yaw = 168.75f;

	FFAStartLocations[3] = FVector(-150.f, 250.f, 0.0f);
	FFARotations[3].Pitch = 0.f;
	FFARotations[3].Roll = 0.f;
	FFARotations[3].Yaw = -179.99f;

	FFAStartLocations[4] = FVector(-150.f, -250.f, 0.0f);
	FFARotations[4].Pitch = 0.f;
	FFARotations[4].Roll = 0.f;
	FFARotations[4].Yaw = -179.99f;

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	FFATeamSpawnLocations.SetNum(5);

	for (int WinIndex = 0; WinIndex < 5; ++WinIndex)
	{
		FFATeamSpawnLocations[WinIndex].SetTranslation(FFAStartLocations[WinIndex]);
		FFATeamSpawnLocations[WinIndex].SetRotation(FFARotations[WinIndex].Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForTeamEndMatch()
{
	TArray<FVector> WinningStartLocations;
	TArray<FRotator> WinningRotations;

	WinningStartLocations.SetNum(5);
	WinningRotations.SetNum(5);

	WinningStartLocations[0] = FVector(-150.f, 0.f, 0.0f);
	WinningRotations[0].Pitch = 0.f;
	WinningRotations[0].Roll = 0.f;
	WinningRotations[0].Yaw = -179.99f;

	WinningStartLocations[1] = FVector(-100.f, 150.f, 0.0f);
	WinningRotations[1].Pitch = 0.f;
	WinningRotations[1].Roll = 0.f;
	WinningRotations[1].Yaw = 179.99f;

	WinningStartLocations[2] = FVector(-100.f, -150.f, 0.0f);
	WinningRotations[2].Pitch = 0.f;
	WinningRotations[2].Roll = 0.f;
	WinningRotations[2].Yaw = -179.99f;

	WinningStartLocations[3] = FVector(-50.f, 300.f, 0.0f);
	WinningRotations[3].Pitch = 0.f;
	WinningRotations[3].Roll = 0.f;
	WinningRotations[3].Yaw = -179.99f;

	WinningStartLocations[4] = FVector(-50.f, -300.f, 0.0f);
	WinningRotations[4].Pitch = 0.f;
	WinningRotations[4].Roll = 0.f;
	WinningRotations[4].Yaw = -179.99f;

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	RedAndWinningTeamSpawnLocations.SetNum(5);

	for (int WinIndex = 0; WinIndex < 5; ++WinIndex)
	{
		RedAndWinningTeamSpawnLocations[WinIndex].SetTranslation(WinningStartLocations[WinIndex]);
		RedAndWinningTeamSpawnLocations[WinIndex].SetRotation(WinningRotations[WinIndex].Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForFFAEndMatch()
{
	TArray<FVector> FFAStartLocations;
	TArray<FRotator> FFARotations;

	FFAStartLocations.SetNum(5);
	FFARotations.SetNum(5);

	FFAStartLocations[0] = FVector(-150.f, 0.f, 0.0f);
	FFARotations[0].Pitch = 0.f;
	FFARotations[0].Roll = 0.f;
	FFARotations[0].Yaw = -180.f;

	FFAStartLocations[1] = FVector(-100.f, 150.f, 0.0f);
	FFARotations[1].Pitch = 0.f;
	FFARotations[1].Roll = 0.f;
	FFARotations[1].Yaw = -180.f;

	FFAStartLocations[2] = FVector(-100.f, -150.f, 0.0f);
	FFARotations[2].Pitch = 0.f;
	FFARotations[2].Roll = 0.f;
	FFARotations[2].Yaw = -180.f;

	FFAStartLocations[3] = FVector(-50.f, 300.f, 0.0f);
	FFARotations[3].Pitch = 0.f;
	FFARotations[3].Roll = 0.f;
	FFARotations[3].Yaw = -180.f;

	FFAStartLocations[4] = FVector(-50.f, -300.f, 0.0f);
	FFARotations[4].Pitch = 0.f;
	FFARotations[4].Roll = 0.f;
	FFARotations[4].Yaw = -180.f;

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	FFATeamSpawnLocations.SetNum(5);

	for (int WinIndex = 0; WinIndex < 5; ++WinIndex)
	{
		FFATeamSpawnLocations[WinIndex].SetTranslation(FFAStartLocations[WinIndex]);
		FFATeamSpawnLocations[WinIndex].SetRotation(FFARotations[WinIndex].Quaternion());
	}

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = 0.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(-750.0f, 0.f, 72.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}

void AUTLineUpZone::DefaultCreateForOnly1Character()
{
	TArray<FVector> FFAStartLocations;
	TArray<FRotator> FFARotations;

	FFAStartLocations.SetNum(1);
	FFARotations.SetNum(1);

	FFAStartLocations[0] = FVector(0.0f, 0.f, 0.0f);
	FFARotations[0].Pitch = 0.f;
	FFARotations[0].Roll = 0.f;
	FFARotations[0].Yaw = 0;

	RedAndWinningTeamSpawnLocations.Empty();
	BlueAndLosingTeamSpawnLocations.Empty();
	FFATeamSpawnLocations.Empty();

	FFATeamSpawnLocations.SetNum(1);

	FFATeamSpawnLocations[0].SetTranslation(FFAStartLocations[0]);
	FFATeamSpawnLocations[0].SetRotation(FFARotations[0].Quaternion());

	SnapToFloor();
	DeleteAllMeshVisualizations();
	InitializeMeshVisualizations();

	FTransform CameraTransform;
	CameraTransform.SetTranslation(FVector(200.f, 0.f, 30.f));
	Camera->SetFieldOfView(60.f);
	Camera->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FRotator DefaultCameraRotation;
	DefaultCameraRotation.Roll = 0.f;
	DefaultCameraRotation.Pitch = 0.f;
	DefaultCameraRotation.Yaw = -180.f;

	FTransform DefaultCameraTransform;
	DefaultCameraTransform.SetTranslation(FVector(750.f, 0.f, 47.5f));
	DefaultCameraTransform.SetRotation(DefaultCameraRotation.Quaternion());

	Camera->SetRelativeTransform(DefaultCameraTransform);
}