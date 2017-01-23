// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "SUTPlayerInfoDialog.h"
#include "../SUWindowsStyle.h"
#include "../Widgets/SUTTabWidget.h"
#include "UTCanvasRenderTarget2D.h"
#include "EngineModule.h"
#include "SlateMaterialBrush.h"
#include "UTPlayerCameraManager.h"
#include "UTCharacterContent.h"
#include "UTWeap_ShockRifle.h"
#include "UTWeaponAttachment.h"
#include "Engine/UserInterfaceSettings.h"
#include "UTHUDWidget_ReplayTimeSlider.h"
#include "UTPlayerInput.h"
#include "StatNames.h"

#if !UE_SERVER

#include "SScaleBox.h"
#include "Widgets/SDragImage.h"

void SUTPlayerInfoDialog::Construct(const FArguments& InArgs)
{
	FVector2D ViewportSize;
	InArgs._PlayerOwner->ViewportClient->GetViewportSize(ViewportSize);
	bAllowLogout = InArgs._bAllowLogout;

	TargetPlayerState = InArgs._TargetPlayerState;
	
	FText NewDialogTitle = FText::Format(NSLOCTEXT("SUTMenuBase", "PlayerInfoTitleFormat", "Player Info - {0}"), FText::FromString(InArgs._TargetPlayerState->PlayerName));
	SUTDialogBase::Construct(SUTDialogBase::FArguments()
							.PlayerOwner(InArgs._PlayerOwner)
							.DialogTitle(NewDialogTitle)
							.DialogSize(InArgs._DialogSize)
							.DialogPosition(InArgs._DialogPosition)
							.DialogAnchorPoint(InArgs._DialogAnchorPoint)
							.ContentPadding(FVector2D(0,0))
							.IsScrollable(false)
							.ButtonMask(UTDIALOG_BUTTON_OK)
							.OnDialogResult(InArgs._OnDialogResult)
							.bShadow(false)
						);

	if (TargetPlayerState.IsValid()) 
	{
		TargetUniqueId = TargetPlayerState->UniqueId.GetUniqueNetId();

		if (TargetPlayerState->Role == ROLE_Authority && PlayerOwner.IsValid() && PlayerOwner->GetProfileSettings())
		{
			TargetPlayerState->ServerSetCharacter(PlayerOwner->GetProfileSettings()->CharacterPath);
			TargetPlayerState->ServerReceiveHatClass(PlayerOwner->GetProfileSettings()->HatPath);
			TargetPlayerState->ServerReceiveHatVariant(PlayerOwner->GetProfileSettings()->HatVariant);
			TargetPlayerState->ServerReceiveEyewearClass(PlayerOwner->GetProfileSettings()->EyewearPath);
			TargetPlayerState->ServerReceiveEyewearVariant(PlayerOwner->GetProfileSettings()->EyewearVariant);
		}
	}

	PlayerPreviewMesh = nullptr;
	PreviewWeapon = nullptr;
	bSpinPlayer = true;
	ZoomOffset = 90.f;

	PlayerPreviewAnimBlueprint = LoadObject<UClass>(nullptr, TEXT("/Game/RestrictedAssets/UI/ABP_PlayerPreview.ABP_PlayerPreview_C"));
	PlayerPreviewAnimFemaleBlueprint = LoadObject<UClass>(nullptr, TEXT("/Game/RestrictedAssets/UI/ABP_Female_PlayerPreview.ABP_Female_PlayerPreview_C"));

	// allocate a preview scene for rendering
	PlayerPreviewWorld = UWorld::CreateWorld(EWorldType::GamePreview, true);
	PlayerPreviewWorld->bShouldSimulatePhysics = true;
	GEngine->CreateNewWorldContext(EWorldType::GamePreview).SetCurrentWorld(PlayerPreviewWorld);
	PlayerPreviewWorld->InitializeActorsForPlay(FURL(), true);
	ViewState.Allocate();
	{
		UClass* EnvironmentClass = LoadObject<UClass>(nullptr, TEXT("/Game/RestrictedAssets/UI/PlayerPreviewEnvironment.PlayerPreviewEnvironment_C"));
		PreviewEnvironment = PlayerPreviewWorld->SpawnActor<AActor>(EnvironmentClass, FVector(500.f, 50.f, 0.f), FRotator(0, 0, 0));
	}
	
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(NULL, TEXT("/Game/RestrictedAssets/UI/PlayerPreviewProxy.PlayerPreviewProxy"));
	if (BaseMat != NULL)
	{
		PlayerPreviewTexture = Cast<UUTCanvasRenderTarget2D>(UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GetPlayerOwner().Get(), UUTCanvasRenderTarget2D::StaticClass(), ViewportSize.X, ViewportSize.Y));
		PlayerPreviewTexture->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		PlayerPreviewTexture->OnNonUObjectRenderTargetUpdate.BindSP(this, &SUTPlayerInfoDialog::UpdatePlayerRender);
		PlayerPreviewMID = UMaterialInstanceDynamic::Create(BaseMat, PlayerPreviewWorld);
		PlayerPreviewMID->SetTextureParameterValue(FName(TEXT("TheTexture")), PlayerPreviewTexture);
		PlayerPreviewBrush = new FSlateMaterialBrush(*PlayerPreviewMID, ViewportSize);
	}
	else
	{
		PlayerPreviewTexture = NULL;
		PlayerPreviewMID = NULL;
		PlayerPreviewBrush = new FSlateMaterialBrush(*UMaterial::GetDefaultMaterial(MD_Surface), ViewportSize);
	}

	PlayerPreviewTexture->TargetGamma = GEngine->GetDisplayGamma();
	PlayerPreviewTexture->InitCustomFormat(ViewportSize.X, ViewportSize.Y, PF_B8G8R8A8, false);
	PlayerPreviewTexture->UpdateResourceImmediate();

	FVector2D ResolutionScale(ViewportSize.X / 1280.0f, ViewportSize.Y / 720.0f);

	if (DialogContent.IsValid())
	{
		const float MessageTextPaddingX = 10.0f;
		TSharedPtr<STextBlock> MessageTextBlock;
		DialogContent->AddSlot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(850)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFill)
						[
							SNew(SDragImage)
							.Image(PlayerPreviewBrush)
							.OnDrag(this, &SUTPlayerInfoDialog::DragPlayerPreview)
							.OnZoom(this, &SUTPlayerInfoDialog::ZoomPlayerPreview)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(InfoPanel, SOverlay)
						]
					]
				]
			]
		];
	}

	OnUpdatePlayerState();
	TabWidget->SelectTab(0);

	// Turn on Screen Space Reflection max quality
	auto SSRQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality"));
	OldSSRQuality = SSRQualityCVar->GetInt();
	SSRQualityCVar->Set(4, ECVF_SetByCode);
}

void SUTPlayerInfoDialog::AddButtonsToLeftOfButtonBar(uint32& ButtonCount)
{
	if (ButtonBar.IsValid() && bAllowLogout)
	{
		BuildButton(ButtonBar, NSLOCTEXT("SUTPlayerInfoDialog","LogoutButton","LOG OUT"),0xFFFF, ButtonCount);
	}
}


SUTPlayerInfoDialog::~SUTPlayerInfoDialog()
{
	// Reset Screen Space Reflection max quality, wish there was a cleaner way to reset the flags
	auto SSRQualityCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality"));
	EConsoleVariableFlags Flags = SSRQualityCVar->GetFlags();
	Flags = (EConsoleVariableFlags)(((uint32)Flags & ~ECVF_SetByMask) | ECVF_SetByScalability);
	SSRQualityCVar->Set(OldSSRQuality, ECVF_SetByCode);
	SSRQualityCVar->SetFlags(Flags);

	if (!GExitPurge)
	{
		if (PlayerPreviewTexture != NULL)
		{
			PlayerPreviewTexture->OnNonUObjectRenderTargetUpdate.Unbind();
			PlayerPreviewTexture = NULL;
		}
		FlushRenderingCommands();
		if (PlayerPreviewBrush != NULL)
		{
			// FIXME: Slate will corrupt memory if this is deleted. Must be referencing it somewhere that doesn't get cleaned up...
			//		for now, we'll take the minor memory leak (the texture still gets GC'ed so it's not too bad)
			//delete PlayerPreviewBrush;
			PlayerPreviewBrush->SetResourceObject(NULL);
			PlayerPreviewBrush = NULL;
		}
		if (PlayerPreviewWorld != NULL)
		{
			PlayerPreviewWorld->DestroyWorld(true);
			GEngine->DestroyWorldContext(PlayerPreviewWorld);
			PlayerPreviewWorld = NULL;
			GetPlayerOwner()->GetWorld()->ForceGarbageCollection(true);
		}
	}
	ViewState.Destroy();
}

void SUTPlayerInfoDialog::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PlayerPreviewTexture);
	Collector.AddReferencedObject(PlayerPreviewMID);
	Collector.AddReferencedObject(PlayerPreviewWorld);
	Collector.AddReferencedObject(PlayerPreviewAnimBlueprint);
	Collector.AddReferencedObject(PlayerPreviewAnimFemaleBlueprint);
}

FReply SUTPlayerInfoDialog::OnButtonClick(uint16 ButtonID)
{
	if (ButtonID == UTDIALOG_BUTTON_OK) 
	{
		GetPlayerOwner()->CloseDialog(SharedThis(this));	
	}
	else if (ButtonID == 0xFFFF)
	{
		Logout();
	}
	return FReply::Handled();
}

void SUTPlayerInfoDialog::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SUTDialogBase::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (PlayerPreviewWorld != nullptr)
	{
		PlayerPreviewWorld->Tick(LEVELTICK_All, InDeltaTime);
	}
	
	// Force the preview mesh to put the highest mips into memory
	if (PlayerPreviewMesh != nullptr)
	{
		PlayerPreviewMesh->PrestreamTextures(1, true);
		if (PlayerPreviewMesh->Hat)
		{
			PlayerPreviewMesh->Hat->PrestreamTextures(1, true);
		}
	}
	if (PreviewWeapon)
	{
		PreviewWeapon->PrestreamTextures(1, true);
	}

	if ( PlayerPreviewTexture != nullptr )
	{
		PlayerPreviewTexture->FastUpdateResource();
	}

	BuildFriendPanel();
}

void SUTPlayerInfoDialog::RecreatePlayerPreview()
{
	FRotator ActorRotation = FRotator(0.0f, 180.0f, 0.0f);

	if (!TargetPlayerState.IsValid()) return;

	if (PlayerPreviewMesh != nullptr)
	{
		ActorRotation = PlayerPreviewMesh->GetActorRotation();
		PlayerPreviewMesh->Destroy();
	}

	if ( PreviewWeapon != nullptr )
	{
		PreviewWeapon->Destroy();
	}

	AGameStateBase* GameState = GetPlayerOwner()->GetWorld()->GetGameState();
	const AUTBaseGameMode* DefaultGameMode = GameState->GetDefaultGameMode<AUTBaseGameMode>();
	if (DefaultGameMode)
	{
		TSubclassOf<class APawn> DefaultPawnClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *DefaultGameMode->PlayerPawnObject.ToStringReference().ToString(), NULL, LOAD_NoWarn));

		//For gamemodes without a default pawn class (menu gamemodes), spawn our default one
		if (DefaultPawnClass == nullptr)
		{
			DefaultPawnClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), NULL, *AUTGameMode::StaticClass()->GetDefaultObject<AUTGameMode>()->PlayerPawnObject.ToStringReference().ToString(), NULL, LOAD_NoWarn));
		}

		PlayerPreviewMesh = PlayerPreviewWorld->SpawnActor<AUTCharacter>(DefaultPawnClass, FVector(300.0f, 0.f, 4.f), ActorRotation);
		if (PlayerPreviewMesh && PlayerPreviewMesh->GetMesh())
		{
			PlayerPreviewMesh->ApplyCharacterData(TargetPlayerState->GetSelectedCharacter());
			PlayerPreviewMesh->SetHatClass(TargetPlayerState->HatClass);
			PlayerPreviewMesh->SetHatVariant(TargetPlayerState->HatVariant);
			PlayerPreviewMesh->SetEyewearClass(TargetPlayerState->EyewearClass);
			PlayerPreviewMesh->SetEyewearVariant(TargetPlayerState->EyewearVariant);
						
			if (TargetPlayerState->Team != NULL)
			{
				float SkinSelect = TargetPlayerState->Team->TeamIndex;
				
				for (UMaterialInstanceDynamic* MI : PlayerPreviewMesh->GetBodyMIs())
				{
					MI->SetScalarParameterValue(TEXT("TeamSelect"), SkinSelect);
				}
			}

			PlayerPreviewMesh->GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
			if (TargetPlayerState.IsValid() && TargetPlayerState->IsFemale())
			{
				if (PlayerPreviewAnimFemaleBlueprint)
				{
					PlayerPreviewMesh->GetMesh()->SetAnimInstanceClass(PlayerPreviewAnimFemaleBlueprint);
				}
			}
			else
			{
				if (PlayerPreviewAnimBlueprint)
				{
					PlayerPreviewMesh->GetMesh()->SetAnimInstanceClass(PlayerPreviewAnimBlueprint);
				}
			}

			UClass* PreviewAttachmentType = TargetPlayerState.IsValid() && TargetPlayerState->FavoriteWeapon ? TargetPlayerState->FavoriteWeapon->GetDefaultObject<AUTWeapon>()->AttachmentType : NULL;
			if (!PreviewAttachmentType)
			{
				PreviewAttachmentType = LoadClass<AUTWeaponAttachment>(NULL, TEXT("/Game/RestrictedAssets/Weapons/ShockRifle/ShockAttachment.ShockAttachment_C"), NULL, LOAD_None, NULL);
			}
			if (PreviewAttachmentType != NULL)
			{
				PreviewWeapon = PlayerPreviewWorld->SpawnActor<AUTWeaponAttachment>(PreviewAttachmentType, FVector(0, 0, 0), FRotator(0, 0, 0));
				PreviewWeapon->Instigator = PlayerPreviewMesh;
			}

			// Tick the world to make sure the animation is up to date.
			if ( PlayerPreviewWorld != nullptr )
			{
				PlayerPreviewWorld->Tick(LEVELTICK_All, 0.0);
			}

			if ( PreviewWeapon )
			{
				PreviewWeapon->BeginPlay();
				PreviewWeapon->AttachToOwner();
			}
		}
		else
		{
			UE_LOG(UT,Log,TEXT("Could not spawn the player's mesh (DefaultPawnClass = %s"), *DefaultGameMode->DefaultPawnClass->GetFullName());
		}
	}
}


void SUTPlayerInfoDialog::UpdatePlayerRender(UCanvas* C, int32 Width, int32 Height)
{
	FEngineShowFlags ShowFlags(ESFIM_Game);
	ShowFlags.SetMotionBlur(false);
	ShowFlags.SetGrain(false);
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(PlayerPreviewTexture->GameThread_GetRenderTargetResource(), PlayerPreviewWorld->Scene, ShowFlags).SetRealtimeUpdate(true));

	FVector CameraPosition(ZoomOffset, 0, -60);

	const float FOV = 45;
	const float AspectRatio = Width / (float)Height;

	FSceneViewInitOptions PlayerPreviewInitOptions;
	PlayerPreviewInitOptions.SetViewRectangle(FIntRect(0, 0, C->SizeX, C->SizeY));
	PlayerPreviewInitOptions.ViewOrigin = -CameraPosition;
	PlayerPreviewInitOptions.ViewRotationMatrix = FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0), FPlane(0, 0, 0, 1));
	PlayerPreviewInitOptions.ProjectionMatrix = 
		FReversedZPerspectiveMatrix(
			FMath::Max(0.001f, FOV) * (float)PI / 360.0f,
			AspectRatio,
			1.0f,
			GNearClippingPlane );
	PlayerPreviewInitOptions.ViewFamily = &ViewFamily;
	PlayerPreviewInitOptions.SceneViewStateInterface = ViewState.GetReference();
	PlayerPreviewInitOptions.BackgroundColor = FLinearColor::Black;
	PlayerPreviewInitOptions.WorldToMetersScale = GetPlayerOwner()->GetWorld()->GetWorldSettings()->WorldToMeters;
	PlayerPreviewInitOptions.CursorPos = FIntPoint(-1, -1);
	
	ViewFamily.bUseSeparateRenderTarget = true;

	FSceneView* View = new FSceneView(PlayerPreviewInitOptions); // note: renderer gets ownership
	View->ViewLocation = FVector::ZeroVector;
	View->ViewRotation = FRotator::ZeroRotator;
	FPostProcessSettings PPSettings = GetDefault<AUTPlayerCameraManager>()->DefaultPPSettings;

	ViewFamily.Views.Add(View);

	View->StartFinalPostprocessSettings(CameraPosition);
	View->EndFinalPostprocessSettings(PlayerPreviewInitOptions);
	View->ViewRect = View->UnscaledViewRect;

	// workaround for hacky renderer code that uses GFrameNumber to decide whether to resize render targets
	--GFrameNumber;
	GetRendererModule().BeginRenderingViewFamily(C->Canvas, &ViewFamily);
}

void SUTPlayerInfoDialog::DragPlayerPreview(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (PlayerPreviewMesh != nullptr)
	{
		bSpinPlayer = false;
		PlayerPreviewMesh->SetActorRotation(PlayerPreviewMesh->GetActorRotation() + FRotator(0, 0.2f * -MouseEvent.GetCursorDelta().X, 0.0f));
	}
}

void SUTPlayerInfoDialog::ZoomPlayerPreview(float WheelDelta)
{
	ZoomOffset = FMath::Clamp(ZoomOffset + (-WheelDelta * 5.0f), -100.0f, 400.0f);
}

TSharedRef<class SWidget> SUTPlayerInfoDialog::BuildCustomButtonBar()
{
	TSharedPtr<SHorizontalBox> CustomBox;
	SAssignNew(CustomBox, SHorizontalBox);
	CustomBox->AddSlot()
	.AutoWidth()
	[
		SAssignNew(FriendPanel, SHorizontalBox)
	];

	if (!PlayerOwner->PlayerController->PlayerState->bOnlySpectator)
	{
		CustomBox->AddSlot()
		.Padding(10.0f,0.0f,10.0f,0.0f)
		[
			SAssignNew(KickButton, SButton)
			.HAlign(HAlign_Center)
			.ButtonStyle(SUWindowsStyle::Get(), "UT.BottomMenu.Button")
			.ContentPadding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
			.Text(NSLOCTEXT("SUTPlayerInfoDialog","KickVote","Vote to Kick"))
			.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
			.Visibility(this, &SUTPlayerInfoDialog::VoteKickVis)
			.OnClicked(this, &SUTPlayerInfoDialog::KickVote)
		];
	}

	return CustomBox.ToSharedRef();
}

EVisibility SUTPlayerInfoDialog::VoteKickVis() const
{
	if (PlayerOwner.IsValid() && PlayerOwner->PlayerController)
	{
		AUTGameState* UTGameState = PlayerOwner->GetWorld()->GetGameState<AUTGameState>();
		AUTPlayerState* OwnerPlayerState = Cast<AUTPlayerState>(PlayerOwner->PlayerController->PlayerState);
		if ( GetPlayerOwner()->GetWorld()->GetNetMode() == NM_Client && 
				OwnerPlayerState != nullptr && 
				!OwnerPlayerState->bOnlySpectator && 
				TargetPlayerState.IsValid() && 
				!TargetPlayerState->bIsABot && 
				OwnerPlayerState != TargetPlayerState &&
				!UTGameState->bDisableVoteKick &&
				(!UTGameState->bOnlyTeamCanVoteKick || UTGameState->OnSameTeam(OwnerPlayerState, TargetPlayerState.Get()))
			)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}


FReply SUTPlayerInfoDialog::OnSendFriendRequest()
{
	if (FriendStatus != FFriendsStatus::FriendRequestPending && FriendStatus != FFriendsStatus::IsBot)
	{
		GetPlayerOwner()->RequestFriendship(TargetUniqueId);

		FriendPanel->ClearChildren();
		FriendPanel->AddSlot()
			.Padding(10.0, 0.0, 0.0, 0.0)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("SUTPlayerInfoDialog", "FriendRequestPending", "You have sent a friend request..."))
				.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
			];

		FriendStatus = FFriendsStatus::FriendRequestPending;
	}
	return FReply::Handled();
}

FText SUTPlayerInfoDialog::GetFunnyText()
{
	return NSLOCTEXT("SUTPlayerInfoDialog", "FunnyDefault", "Viewing self.");
}

void SUTPlayerInfoDialog::BuildFriendPanel()
{
	if (TargetPlayerState == NULL) return;

	FName NewFriendStatus;
	if (GetPlayerOwner()->PlayerController->PlayerState == TargetPlayerState)
	{
		NewFriendStatus = FFriendsStatus::IsYou;
	}
	else if (TargetPlayerState->bIsABot)
	{
		NewFriendStatus = FFriendsStatus::IsBot;
	}
	else
	{
		NewFriendStatus = GetPlayerOwner()->IsAFriend(TargetPlayerState->UniqueId) ? FFriendsStatus::Friend : FFriendsStatus::NotAFriend;
	}

	bool bRequiresRefresh = false;
	if (FriendStatus == FFriendsStatus::FriendRequestPending)
	{
		if (NewFriendStatus == FFriendsStatus::Friend)
		{
			FriendStatus = NewFriendStatus;
			bRequiresRefresh = true;
		}
	}
	else
	{
		bRequiresRefresh = FriendStatus != NewFriendStatus;
		FriendStatus = NewFriendStatus;
	}

	if (bRequiresRefresh)
	{
		FriendPanel->ClearChildren();
		if (FriendStatus == FFriendsStatus::IsYou)
		{
			FText FunnyText = GetFunnyText();
			FriendPanel->AddSlot()
			.Padding(10.0, 0.0, 0.0, 0.0)
				[
					SNew(STextBlock)
					.Text(FunnyText)
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
				];
		}
		else if (FriendStatus == FFriendsStatus::IsBot)
		{
			FriendPanel->AddSlot()
				.Padding(10.0, 0.0, 0.0, 0.0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("SUTPlayerInfoDialog", "IsABot", "AI (C) Liandri Corp."))
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
				];
		}
		else if (FriendStatus == FFriendsStatus::Friend)
		{
			FriendPanel->AddSlot()
				.Padding(10.0, 0.0, 0.0, 0.0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("SUTPlayerInfoDialog", "IsAFriend", "Is your friend"))
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
				];
		}
		else if (FriendStatus == FFriendsStatus::FriendRequestPending)
		{
		}
		else
		{
			FriendPanel->AddSlot()
				.Padding(10.0, 0.0, 0.0, 0.0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ButtonStyle(SUWindowsStyle::Get(), "UT.BottomMenu.Button")
					.ContentPadding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
					.Text(NSLOCTEXT("SUTPlayerInfoDialog", "SendFriendRequest", "Send Friend Request"))
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
					.OnClicked(this, &SUTPlayerInfoDialog::OnSendFriendRequest)
				];
		}
	}

}

FReply SUTPlayerInfoDialog::KickVote()
{
	if (TargetPlayerState.IsValid()) 
	{
		AUTPlayerState* MyPlayerState =  Cast<AUTPlayerState>(GetPlayerOwner()->PlayerController->PlayerState);
		if (TargetPlayerState != MyPlayerState)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(GetPlayerOwner()->PlayerController);
			if (PC)
			{
				PC->ServerRegisterBanVote(TargetPlayerState.Get());
				KickButton->SetEnabled(false);
			}
		}
	}

	return FReply::Handled();
}

FReply SUTPlayerInfoDialog::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	//Close with escape
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		GetPlayerOwner()->CloseDialog(SharedThis(this));
		return FReply::Handled();
	}

	//If the key matches the TogglePlayerInfo spectator bind then close the dialog
	FName KeyName = InKeyEvent.GetKey().GetFName();

	UUTPlayerInput* UTInput = Cast<UUTPlayerInput>(GetPlayerOwner()->PlayerController->PlayerInput);
	if (UTInput != nullptr)
	{
		for (auto& Bind : UTInput->SpectatorBinds)
		{
			if (Bind.Command == TEXT("TogglePlayerInfo") && KeyName == Bind.KeyName)
			{
				GetPlayerOwner()->CloseDialog(SharedThis(this));
				return FReply::Handled();
			}
		}
	}

	if (InKeyEvent.GetKey() == EKeys::Left)
	{
		PreviousPlayer();
	}
	else if (InKeyEvent.GetKey() == EKeys::Right)
	{
		NextPlayer();
	}

	return FReply::Unhandled();
}

void SUTPlayerInfoDialog::OnTabButtonSelectionChanged(const FText& NewText)
{
	static const FText Score = NSLOCTEXT("AUTGameMode", "Score", "Score");
	static const FText Weapons = NSLOCTEXT("AUTGameMode", "Weapons", "Weapons");
	static const FText Rewards = NSLOCTEXT("AUTGameMode", "Rewards", "Rewards");
	static const FText Movement = NSLOCTEXT("AUTGameMode", "Movement", "Movement");

	AUTPlayerController* UTPC = Cast<AUTPlayerController>(GetPlayerOwner()->PlayerController);
	if (UTPC != nullptr)
	{
		if (NewText.EqualTo(Score))
		{
			UTPC->ServerSetViewedScorePS(TargetPlayerState.Get(), 0);
		}
		else if (NewText.EqualTo(Weapons))
		{
			UTPC->ServerSetViewedScorePS(TargetPlayerState.Get(), 1);
		}
		else if (NewText.EqualTo(Rewards))
		{
			UTPC->ServerSetViewedScorePS(TargetPlayerState.Get(), 2);
		}
		else if (NewText.EqualTo(Movement))
		{
			UTPC->ServerSetViewedScorePS(TargetPlayerState.Get(), 3);
		}
		else
		{
			UTPC->ServerSetViewedScorePS(nullptr, 0);
		}
	}

	CurrentTab = NewText;
}

FReply SUTPlayerInfoDialog::NextPlayer()
{
	if (TargetPlayerState.IsValid())
	{
		TargetPlayerState = GetNextPlayerState(1);
		if (TargetPlayerState.IsValid())
		{
			OnUpdatePlayerState();
		}
		else
		{
			GetPlayerOwner()->CloseDialog(SharedThis(this));
		}
	}
	
	return FReply::Handled();
}
FReply SUTPlayerInfoDialog::PreviousPlayer()
{
	if (TargetPlayerState.IsValid())
	{
		TargetPlayerState = GetNextPlayerState(-1);
		if (TargetPlayerState.IsValid())
		{
			OnUpdatePlayerState();
		}
		else
		{
			GetPlayerOwner()->CloseDialog(SharedThis(this));
		}
	}
	return FReply::Handled();
}

AUTPlayerState* SUTPlayerInfoDialog::GetNextPlayerState(int32 dir)
{
	int32 CurrentIndex = -1;	
	if (TargetPlayerState.IsValid())
	{
		UWorld* World = TargetPlayerState->GetWorld();
		AGameState* GameState = World->GetGameState<AGameState>();

		// Find index of current viewtarget's PlayerState
		for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
		{
			if (TargetPlayerState.Get() == GameState->PlayerArray[i])
			{
				CurrentIndex = i;
				break;
			}
		}

		// Find next valid viewtarget in appropriate direction
		int32 NewIndex;
		for (NewIndex = CurrentIndex + dir; (NewIndex >= 0) && (NewIndex < GameState->PlayerArray.Num()); NewIndex = NewIndex + dir)
		{
			AUTPlayerState* const PlayerState = Cast<AUTPlayerState>(GameState->PlayerArray[NewIndex]);
			if ((PlayerState != NULL) && (!PlayerState->bOnlySpectator))
			{
				return PlayerState;
			}
		}

		// wrap around
		CurrentIndex = (NewIndex < 0) ? GameState->PlayerArray.Num() : -1;
		for (NewIndex = CurrentIndex + dir; (NewIndex >= 0) && (NewIndex < GameState->PlayerArray.Num()); NewIndex = NewIndex + dir)
		{
			AUTPlayerState* const PlayerState = Cast<AUTPlayerState>(GameState->PlayerArray[NewIndex]);
			if ((PlayerState != NULL) && (!PlayerState->bOnlySpectator))
			{
				return PlayerState;
			}
		}
	}
	return NULL;
}

void SUTPlayerInfoDialog::OnUpdatePlayerState()
{
	if (TargetPlayerState.IsValid())
	{
		UpdatePlayerStateInReplays();

		StatList.Empty();
		InfoPanel->ClearChildren();

		InfoPanel->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(TabWidget, SUTTabWidget)
			.OnTabButtonSelectionChanged(this, &SUTPlayerInfoDialog::OnTabButtonSelectionChanged)
		];
	
		TargetPlayerState->BuildPlayerInfo(TabWidget, StatList);

		//Draw the game specific stats
		AGameStateBase* GameState = GetPlayerOwner()->GetWorld()->GetGameState();
		if (GameState && !TargetPlayerState->bOnlySpectator)
		{
			AUTBaseGameMode* UTDefaultGameMode = const_cast<AUTBaseGameMode*>(GameState->GetDefaultGameMode<AUTBaseGameMode>());
			if (UTDefaultGameMode)
			{
				UTDefaultGameMode->BuildPlayerInfo(TargetPlayerState.Get(), TabWidget, StatList);
			}
		}

		FriendStatus = NAME_None;
		RecreatePlayerPreview();

		TabWidget->OnButtonClicked(CurrentTab);

		//We are in a game where we can only ever be a spectator. 
		//This means we are not in the player list, and thus need to use our Account Display Name as our Player Name isn't accurate.
		if (TargetPlayerState->bOnlySpectator)
		{
			DialogTitle->SetText(GetPlayerOwner()->GetAccountDisplayName());
		}
		else
		{
			DialogTitle->SetText(FText::FromString(TargetPlayerState->PlayerName));
		}
	}
}

void SUTPlayerInfoDialog::UpdatePlayerStateInReplays()
{
	if (TargetPlayerState.IsValid() && TargetPlayerState->IsOwnedByReplayController())
	{
		FMMREntry DuelMMR;
		FMMREntry CTFMMR;
		FMMREntry TDMMMR;
		FMMREntry DMMMR;
		FMMREntry ShowdownMMR;
		FMMREntry FlagRunMMR;

		PlayerOwner->GetMMREntry(NAME_SkillRating.ToString(), DuelMMR);
		PlayerOwner->GetMMREntry(NAME_CTFSkillRating.ToString(), CTFMMR);
		PlayerOwner->GetMMREntry(NAME_TDMSkillRating.ToString(), TDMMMR);
		PlayerOwner->GetMMREntry(NAME_DMSkillRating.ToString(), DMMMR);
		PlayerOwner->GetMMREntry(NAME_ShowdownSkillRating.ToString(), ShowdownMMR);
		PlayerOwner->GetMMREntry(NAME_FlagRunSkillRating.ToString(), FlagRunMMR);

		UpdatePlayerStateRankingStatsFromLocalPlayer(
			DuelMMR.MMR,
			CTFMMR.MMR,
			TDMMMR.MMR,
			DMMMR.MMR,
			ShowdownMMR.MMR,
			FlagRunMMR.MMR,
			PlayerOwner->GetTotalChallengeStars(), 
			FMath::Min(255, DuelMMR.MatchesPlayed),
			FMath::Min(255, CTFMMR.MatchesPlayed),
			FMath::Min(255, TDMMMR.MatchesPlayed),
			FMath::Min(255, DMMMR.MatchesPlayed),
			FMath::Min(255, ShowdownMMR.MatchesPlayed),
			FMath::Min(255, FlagRunMMR.MatchesPlayed));

		UpdatePlayerCharacterPreviewInReplays();
	}
}

void SUTPlayerInfoDialog::UpdatePlayerStateRankingStatsFromLocalPlayer(int32 NewDuelRank, int32 NewCTFRank, int32 NewTDMRank, int32 NewDMRank, int32 NewShowdownRank, int32 NewFlagRunRank, int32 TotalStars, uint8 DuelMatchesPlayed, uint8 CTFMatchesPlayed, uint8 TDMMatchesPlayed, uint8 DMMatchesPlayed, uint8 ShowdownMatchesPlayed, uint8 FlagRunMatchesPlayed)
{
	if (TargetPlayerState.IsValid())
	{
		TargetPlayerState->DuelRank = NewDuelRank;
		TargetPlayerState->CTFRank = NewCTFRank;
		TargetPlayerState->TDMRank = NewTDMRank;
		TargetPlayerState->DMRank = NewDMRank;
		TargetPlayerState->FlagRunRank = NewFlagRunRank;
		TargetPlayerState->ShowdownRank = NewShowdownRank;
		TargetPlayerState->TotalChallengeStars = TotalStars;
		TargetPlayerState->DuelMatchesPlayed = DuelMatchesPlayed;
		TargetPlayerState->CTFMatchesPlayed = CTFMatchesPlayed;
		TargetPlayerState->TDMMatchesPlayed = TDMMatchesPlayed;
		TargetPlayerState->DMMatchesPlayed = DMMatchesPlayed;
		TargetPlayerState->FlagRunMatchesPlayed = FlagRunMatchesPlayed;
		TargetPlayerState->ShowdownMatchesPlayed = ShowdownMatchesPlayed;
	}
}

void SUTPlayerInfoDialog::UpdatePlayerCharacterPreviewInReplays()
{
	if (TargetPlayerState.IsValid() && PlayerOwner.IsValid() && PlayerOwner->GetProfileSettings())
	{
		TargetPlayerState->SetCharacter(PlayerOwner->GetProfileSettings()->CharacterPath);
	}
}

TSharedRef<class SWidget> SUTPlayerInfoDialog::BuildTitleBar(FText InDialogTitle)
{
	if (TargetPlayerState.IsValid())
	{
		UWorld* World = TargetPlayerState->GetWorld();
		if (World && World->GetGameState() && (World->GetGameState()->PlayerArray.Num() > 1))
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(20.0f, 0.0f, 0.0f, 0.0f)
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Left)
					.ButtonStyle(SUWindowsStyle::Get(), "UT.BottomMenu.Button")
					.ContentPadding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
					.Text(NSLOCTEXT("SUTPlayerInfoDialog", "PreviousPlayer", "<- Previous"))
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
					.OnClicked(this, &SUTPlayerInfoDialog::PreviousPlayer)
				]
			+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.FillWidth(1.0f)
				[
					SUTDialogBase::BuildTitleBar(InDialogTitle)
				]
			+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 20.0f, 0.0f)
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Right)
					.ButtonStyle(SUWindowsStyle::Get(), "UT.BottomMenu.Button")
					.ContentPadding(FMargin(5.0f, 5.0f, 5.0f, 5.0f))
					.Text(NSLOCTEXT("SUTPlayerInfoDialog", "NextPlayer", "Next ->"))
					.TextStyle(SUWindowsStyle::Get(), "UT.TopMenu.Button.SmallTextStyle")
					.OnClicked(this, &SUTPlayerInfoDialog::NextPlayer)
				];
		}
	}
	return SUTDialogBase::BuildTitleBar(InDialogTitle);
}

FReply SUTPlayerInfoDialog::Logout()
{
	PlayerOwner->ShowMessage(NSLOCTEXT("SUTPlayerInfoDialog", "SignOuttConfirmationTitle", "Sign Out?"), NSLOCTEXT("SUTPlayerInfoDialog", "SignOuttConfirmationMessage", "You are about to sign out of this account.  Doing so will return you to the main menu.  Are you sure?"), UTDIALOG_BUTTON_YES + UTDIALOG_BUTTON_NO, FDialogResultDelegate::CreateSP(this, &SUTPlayerInfoDialog::SignOutConfirmationResult));
	return FReply::Handled();
}

void SUTPlayerInfoDialog::SignOutConfirmationResult(TSharedPtr<SCompoundWidget> Widget, uint16 ButtonID)
{
	if (ButtonID == UTDIALOG_BUTTON_YES)
	{
		PlayerOwner->CloseDialog(SharedThis(this));
		PlayerOwner->Logout();
	}
}


#endif