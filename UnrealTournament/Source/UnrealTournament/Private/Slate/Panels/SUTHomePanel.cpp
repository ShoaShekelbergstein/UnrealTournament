// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLocalPlayer.h"
#include "SlateBasics.h"
#include "../SUTUtils.h"
#include "../SUTStyle.h"
#include "../Widgets/SUTScaleBox.h"
#include "../Widgets/SUTBorder.h"
#include "Slate/SlateGameResources.h"
#include "SUTHomePanel.h"
#include "../Menus/SUTMainMenu.h"
#include "../Widgets/SUTButton.h"
#include "NetworkVersion.h"
#include "UTGameInstance.h"
#include "BlueprintContextLibrary.h"
#include "MatchmakingContext.h"
#include "SUTServerBrowserPanel.h"


#if !UE_SERVER

void SUTHomePanel::ConstructPanel(FVector2D ViewportSize)
{
	this->ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SAssignNew(AnimWidget, SUTBorder)
			.OnAnimEnd(this, &SUTHomePanel::AnimEnd)
			[
				BuildHomePanel()
			]
		]
	];

	AnnouncmentTimer = 3.0;
}

void SUTHomePanel::OnShowPanel(TSharedPtr<SUTMenuBase> inParentWindow)
{
	SUTPanelBase::OnShowPanel(inParentWindow);

	CheckForLanServers();
	PlayerOwner->GetWorld()->GetTimerManager().SetTimer(LanTimerHandle, FTimerDelegate::CreateSP(this, &SUTHomePanel::CheckForLanServers), 30.0f, true);

	if (AnimWidget.IsValid())
	{
		AnimWidget->Animate(FVector2D(100.0f, 0.0f), FVector2D(0.0f, 0.0f), 0.0f, 1.0f, 0.3f);
	}
}



void SUTHomePanel::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (AnnouncmentTimer > 0)
	{
		AnnouncmentTimer -= InDeltaTime;
		if (AnnouncmentTimer <= 0.0)
		{
			BuildAnnouncement();
		}
	}

	if (AnnouncmentFadeTimer > 0)
	{
		AnnouncmentFadeTimer -= InDeltaTime;
	}

	if (NewChallengeBox.IsValid())
	{
		if (NewChallengeImage.IsValid())
		{
			float Scale = 1.0f + (0.1 * FMath::Sin(PlayerOwner->GetWorld()->GetTimeSeconds() * 10.0f));
			NewChallengeImage->SetRenderTransform(FSlateRenderTransform(Scale));
		}
	}
}

void SUTHomePanel::CheckForLanServers()
{
	TSharedPtr<SUTServerBrowserPanel> Browser = PlayerOwner->GetServerBrowser();
	if (Browser.IsValid())
	{
		TArray<TSharedPtr<FServerData>> LanServers;
		if (Browser->GetLanServerList(&LanServers) != LanMatches.Num())
		{
			LanBox->ClearChildren();		
			LanMatches.Empty();

			for (int32 i = 0; i < LanServers.Num(); i++)
			{
				FText ServerName = LanServers[i]->GetBrowserName();
				FText ServerInfo = FText::Format(NSLOCTEXT("SUTHomePanel","LanServerFormat","Game: {0}  Map: {1}   # Players: {2}   # Friends: {3}"),
												 LanServers[i]->GetBrowserGameMode(),
												 LanServers[i]->GetBrowserMapName(),
												 LanServers[i]->GetBrowserNumPlayers(),
												 LanServers[i]->GetBrowserNumFriends());
				LanMatches.Add(LanServers[i]);
				LanBox->AddSlot().AutoHeight()
				[

					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().Padding(0.0f,10.0f,0.0f,0.0f)
					[
						SNew(SBorder)
						.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.SuperDark"))
						.ColorAndOpacity(this, &SUTHomePanel::GetFadeColor)
						.BorderBackgroundColor(this, &SUTHomePanel::GetFadeBKColor)
						.HAlign(HAlign_Right)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("SUTHomePanel","LanServerTitle","...Found a LAN Server"))
							.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Tiny.Bold")
							.ColorAndOpacity(FLinearColor::Yellow)
						]
					]
					+SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
						.ColorAndOpacity(this, &SUTHomePanel::GetFadeColor)
						.BorderBackgroundColor(this, &SUTHomePanel::GetFadeBKColor)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SBox).WidthOverride(940)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot().AutoHeight()
										[
											SNew(SUTImage)
											.WidthOverride(90)
											.HeightOverride(64)
											.Image(SUTStyle::Get().GetBrush("UT.Icon.Lan.Big"))
										]
									]
									+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(SBox).WidthOverride(115).HeightOverride(86)
										[
											SNew(SUTButton)
											.ButtonStyle(SUTStyle::Get(),"UT.ClearButton")
											[
												SNew(SVerticalBox)
												+SVerticalBox::Slot()
												.Padding(0.0,4.0,0.0,0.0)
												.AutoHeight()
												[
													SNew(SUTButton)
													.ButtonStyle(SUTStyle::Get(),"UT.SimpleButton.Medium")
													.Text(NSLOCTEXT("SUTMatchPanel","JoinText","JOIN"))
													.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small")
													.CaptionHAlign(HAlign_Center)
													.OnClicked(this, &SUTHomePanel::OnJoinLanClicked, LanServers[i])
													//.IsEnabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(InItem.Get(), &FTrackedMatch::CanJoin, PlayerOwner)))
												]
												+SVerticalBox::Slot()
												.Padding(0.0,10.0,0.0,0.0)
												.AutoHeight()
												[
													SNew(SUTButton)
													.ButtonStyle(SUTStyle::Get(),"UT.SimpleButton.Medium")
													.Text(NSLOCTEXT("SUTMatchPanel","SpectateText","SPECTATE"))
													.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small")
													.CaptionHAlign(HAlign_Center)
													.OnClicked(this, &SUTHomePanel::OnSpectateLanClicked, LanServers[i])
													//.IsEnabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(InItem.Get(), &FTrackedMatch::CanSpectate, PlayerOwner)))
												]
											]
										]
									]
									+SHorizontalBox::Slot().FillWidth(1.0)
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot().AutoHeight()
										[
											SNew(STextBlock)
											.Text(ServerName)
											.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Large.Bold")
											.ColorAndOpacity(FLinearColor(1.0f, 0.412f, 0.027f, 1.0f))
										]
										+SVerticalBox::Slot().AutoHeight()
										[									
											SNew(STextBlock)
											.Text(ServerInfo)
											.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Tiny")
										]
									]

								]
							]
						]
					]
				];
			
			}
		}
	}
}

FLinearColor SUTHomePanel::GetFadeColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = FMath::Clamp<float>(1.0 - (AnnouncmentFadeTimer / 0.8f),0.0f, 1.0f);
	return Color;
}

FSlateColor SUTHomePanel::GetFadeBKColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = FMath::Clamp<float>(1.0 - (AnnouncmentFadeTimer / 0.8f),0.0f, 1.0f);
	return Color;
}


void SUTHomePanel::BuildAnnouncement()
{
	TSharedPtr<SVerticalBox> SlotBox;
	int32 Day = 0;
	int32 Month = 0;
	int32 Year = 0;

	FDateTime Now = FDateTime::UtcNow();
	AnnouncmentFadeTimer = 0.8;

	if (PlayerOwner->MCPAnnouncements.Announcements.Num() > 0)
	{
		for (int32 i=0; i < PlayerOwner->MCPAnnouncements.Announcements.Num(); i++)
		{

			FDateTime Start = PlayerOwner->MCPAnnouncements.Announcements[i].StartDate;
			FDateTime End = PlayerOwner->MCPAnnouncements.Announcements[i].EndDate;

			if ( Now >= Start && Now <= End)
			{
				AnnouncementBox->AddSlot().AutoHeight().Padding(0.0f,10.0f,0.0f,0.0f)
				[
					SNew(SBorder)
					.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.SuperDark"))
					.ColorAndOpacity(this, &SUTHomePanel::GetFadeColor)
					.BorderBackgroundColor(this, &SUTHomePanel::GetFadeBKColor)
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(FText::FromString(PlayerOwner->MCPAnnouncements.Announcements[i].Title))
						.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Tiny.Bold")
						.ColorAndOpacity(FLinearColor::Yellow)
					]
				];

				AnnouncementBox->AddSlot().AutoHeight().Padding(0.0f,0.0f,0.0f,0.0f)
				[
					SNew(SBorder)
					.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
					.ColorAndOpacity(this, &SUTHomePanel::GetFadeColor)
					.BorderBackgroundColor(this, &SUTHomePanel::GetFadeBKColor)
					[
						SAssignNew(SlotBox,SVerticalBox)
						+SVerticalBox::Slot()
						.Padding(5.0,5.0,5.0,5.0)
						.AutoHeight()
						[
							SNew(SBox).HeightOverride(PlayerOwner->MCPAnnouncements.Announcements[i].MinHeight)
							[
								SNew(SUTWebBrowserPanel, PlayerOwner)
								.InitialURL(PlayerOwner->MCPAnnouncements.Announcements[i].AnnouncementURL)
								.ShowControls(false)
							]	
						]
					]
				];
			}
		}
	}
}


TSharedRef<SWidget> SUTHomePanel::BuildHomePanel()
{

	FString BuildVersion = FApp::GetBuildVersion();
	int32 Pos = -1;
	if (BuildVersion.FindLastChar('-', Pos)) BuildVersion = BuildVersion.Right(BuildVersion.Len() - (Pos + 1));

	TSharedPtr<SOverlay> Final;
	TSharedPtr<SHorizontalBox> QuickPlayBox;

	SAssignNew(Final, SOverlay)

		// Announcement box
		+SOverlay::Slot()
		.Padding(920.0,0.0,0.0,32.0)
		.VAlign(VAlign_Bottom)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(940)
					[
						SAssignNew(AnnouncementBox, SVerticalBox)
					]
				]
			]
			+SVerticalBox::Slot().AutoHeight()
			.AutoHeight()
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(940)
					[
						SAssignNew(LanBox, SVerticalBox)
					]
				]
			]
		]

		+SOverlay::Slot()
		.Padding(64.0,50.0,6.0,32.0)
		[
			SNew(SVerticalBox)

			// Main Button

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(800).HeightOverride(143)
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot().AutoWidth()
							[
								SNew(SUTImage)
								.Image(SUTStyle::Get().GetBrush("UT.Logo.Loading")).WidthOverride(400).HeightOverride(143)	
							]
							+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0.0f,0.0f,0.0f,10.0f)
							[
								SNew(SBox).WidthOverride(400).HAlign(HAlign_Right)
								[
									SNew(STextBlock)
									.Text(FText::Format(NSLOCTEXT("Common","VersionFormat","Build {0}"), FText::FromString(BuildVersion)))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small.Bold")
									.ColorAndOpacity(FLinearColor(1.0f, 0.412f, 0.027f, 1.0f))
								]
							]
						]
					]
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(800).HeightOverride(220)
					[
						BuildMainButtonContent()
					]
				]
			]


			// PLAY

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0,30.0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox).WidthOverride(800).HeightOverride(270)
					[
						SAssignNew(QuickPlayBox, SHorizontalBox)
					]
				]
			]

			+ SVerticalBox::Slot()
			.Padding(0.0,30.0)
			.AutoHeight()
			[
				BuildRankedPlaylist()
			]
		]
		+SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FillHeight(1.0f).HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Top)
				[
					SAssignNew(PartyBox, SUTPartyWidget, PlayerOwner->PlayerController)
				]
			]
		]
		+SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FillHeight(1.0f).HAlign(HAlign_Right).Padding(0.0f,142.0f,0.0f,0.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Top)
				[
					SAssignNew(PartyInviteBox, SUTPartyInviteWidget, PlayerOwner->PlayerController)
				]
			]
		];


	BuildQuickplayButton(QuickPlayBox, TEXT("UT.HomePanel.CTFBadge"), NSLOCTEXT("SUTHomePanel","QP_FlagRun","FLAG RUN"), EMenuCommand::MC_QuickPlayFlagrun);
	BuildQuickplayButton(QuickPlayBox, TEXT("UT.HomePanel.DMBadge"), NSLOCTEXT("SUTHomePanel","QP_DM","DEATHMATCH"), EMenuCommand::MC_QuickPlayDM, 25.0f);
	BuildQuickplayButton(QuickPlayBox, TEXT("UT.HomePanel.TeamShowdownBadge"), NSLOCTEXT("SUTHomePanel","QP_Showdown","SHOWDOWN"), EMenuCommand::MC_QuickPlayShowdown);

	return Final.ToSharedRef();
}

TSharedRef<SWidget> SUTHomePanel::BuildMainButtonContent()
{
	if (PlayerOwner->HasProgressionKeys({ FName(TEXT("PROGRESSION_KEY_NoLongerANoob")) }))
	{
		return SNew(SUTButton)
			.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
			.OnClicked(this, &SUTHomePanel::OfflineAction_Click)
			.ToolTipText( TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(PlayerOwner.Get(), &UUTLocalPlayer::GetMenuCommandTooltipText, EMenuCommand::MC_Challenges) ))
			.bSpringButton(true)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SUTImage)
					.Image(SUTStyle::Get().GetBrush("UT.HomePanel.Challenges"))
					.WidthOverride(250)
					.HeightOverride(270)
				]
				+SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().Padding(0.0f,130.0f,0.0f,0.0f)
					[
						SNew(SBorder)
						.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Shaded"))
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.Padding(10.0f,5.0f)
							.AutoWidth()
							[
								SAssignNew(NewChallengeBox, SVerticalBox)
								.Visibility(this, &SUTHomePanel::ShowNewChallengeImage)
								+SVerticalBox::Slot()
								.AutoHeight()
								[
									SAssignNew(NewChallengeImage, SUTImage)
									.Image(SUTStyle::Get().GetBrush("UT.HomePanel.ChallengesNewIcon"))
									.WidthOverride(72.0f).HeightOverride(72.0f)
									.RenderTransformPivot(FVector2D(0.5f, 0.5f))
								]
							]
							+SHorizontalBox::Slot()
							.Padding(20.0f,5.0f,0.0,0.0)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("SINGLE PLAYER CHALLENGES")))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Large")
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Play against bots and earn rewards in a variety of challenge matches.")))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small.Bold")
									.ColorAndOpacity(FLinearColor(1.0f, 0.412f, 0.027f, 1.0f))
								]
							]
						]

					]
				]
			];
	}
	else
	{
		return SNew(SUTButton)
			.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
			.OnClicked(this, &SUTHomePanel::BasicTraining_Click)
			.ToolTip(SUTUtils::CreateTooltip(NSLOCTEXT("SUTHomePanel","BasicTrainingTT","Complete several training sessions to lean all of the skills needed to compete in the tournament.")))
			.bSpringButton(true)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SUTImage)
					.Image(SUTStyle::Get().GetBrush("UT.HomePanel.BasicTraining"))
					.WidthOverride(250)
					.HeightOverride(270)
				]
				+SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().Padding(0.0f,130.0f,0.0f,0.0f)
					[
						SNew(SBorder)
						.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Shaded"))
						[
							SNew(SOverlay)
							+SOverlay::Slot()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.Padding(10.0f,5.0f)
								.AutoWidth()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SBox).WidthOverride(72).HeightOverride(72)
										[
											SNew(SImage)
											.Image(SUTStyle::Get().GetBrush("UT.HomePanel.TutorialLogo"))
										]
									]
								]
							]
							+SOverlay::Slot()
							.Padding(100.0f,5.0f,0.0,0.0)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("BASIC TRAINING")))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Large")
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Train to become the ultimate Unreal Tournament warrior!")))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small.Bold")
									.ColorAndOpacity(FLinearColor(1.0f, 0.412f, 0.027f, 1.0f))
								]
							]
						]

					]
				]
			];
	}
}

void SUTHomePanel::BuildQuickplayButton(TSharedPtr<SHorizontalBox> QuickPlayBox, FName BackgroundTexture, FText Caption, FName QuickMatchType, float Padding)
{
	QuickPlayBox->AddSlot()
		.Padding(Padding,0.0,Padding,0.0)
		.AutoWidth()
		[
			SNew(SBox).WidthOverride(250)
			[
				SNew(SUTButton)
				.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
				.bSpringButton(true)
				.OnClicked(this, &SUTHomePanel::QuickPlayClick, QuickMatchType)
				.ToolTipText( TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(PlayerOwner.Get(), &UUTLocalPlayer::GetMenuCommandTooltipText, QuickMatchType) ) )
				.ContentPadding(FMargin(2.0f, 2.0f, 2.0f, 2.0f))
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					[
						SNew(SUTImage)
						.Image(SUTStyle::Get().GetBrush(BackgroundTexture))
						.WidthOverride(250)
						.HeightOverride(270)
					]
					+SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Shaded"))
							.Padding(FMargin(0.0f,0.0f,0.0f,0.0f))
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("QUICK PLAY")))
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Medium")
								]
								+SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Center)
								[
									SNew(STextBlock)
									.Text(Caption)
									.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small.Bold")
									.ColorAndOpacity(FLinearColor(1.0f, 0.412f, 0.027f, 1.0f))
								]
							]
						]
					]
				]
			]
		];
}

FReply SUTHomePanel::QuickPlayClick(FName QuickMatchType)
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid())
	{
		if (QuickMatchType == EMenuCommand::MC_QuickPlayDM) MainMenu->QuickPlay(EEpicDefaultRuleTags::Deathmatch);
		else if (QuickMatchType == EMenuCommand::MC_QuickPlayFlagrun) MainMenu->QuickPlay(EEpicDefaultRuleTags::FlagRun);
		else if (QuickMatchType == EMenuCommand::MC_QuickPlayShowdown) MainMenu->QuickPlay(EEpicDefaultRuleTags::TEAMSHOWDOWN);
	}

	return FReply::Handled();
}

FReply SUTHomePanel::BasicTraining_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid()) MainMenu->OpenTutorialMenu();
	return FReply::Handled();
}


FReply SUTHomePanel::OfflineAction_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid())
	{
		MainMenu->ShowGamePanel();
	}
	return FReply::Handled();
}

FReply SUTHomePanel::FindAMatch_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid())
	{
		if (PlayerOwner->IsMenuOptionLocked(EMenuCommand::MC_FindAMatch))
		{
			PlayerOwner->LaunchTutorial(ETutorialTags::TUTTAG_Progress);
		}
		else
		{
			MainMenu->OnShowServerBrowserPanel();
		}
	}
	return FReply::Handled();
}

FReply SUTHomePanel::FragCenter_Click()
{

	PlayerOwner->UpdateFragCenter();

	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid()) MainMenu->ShowFragCenter();
	return FReply::Handled();
}

FReply SUTHomePanel::RecentMatches_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid()) MainMenu->RecentReplays();
	return FReply::Handled();
}

FReply SUTHomePanel::WatchLive_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid()) MainMenu->ShowLiveGameReplays();
	return FReply::Handled();
}

FReply SUTHomePanel::TrainingVideos_Click()
{
	TSharedPtr<SUTMainMenu> MainMenu = StaticCastSharedPtr<SUTMainMenu>(GetParentWindow());
	if (MainMenu.IsValid()) MainMenu->ShowCommunity();
	return FReply::Handled();
}

EVisibility SUTHomePanel::ShowNewChallengeImage() const
{
	if (PlayerOwner.IsValid() && PlayerOwner->MCPPulledData.bValid)
	{
		if (PlayerOwner->ChallengeRevisionNumber< PlayerOwner->MCPPulledData.ChallengeRevisionNumber)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FSlateColor SUTHomePanel::GetFragCenterWatchNowColorAndOpacity() const
{
	if (PlayerOwner->IsFragCenterNew())
	{
		float Alpha = 0.5 + (0.5 * FMath::Abs<float>(FMath::Sin(PlayerOwner->GetWorld()->GetTimeSeconds() * 3)));
		return FSlateColor(FLinearColor(1.0,1.0,1.0, Alpha));
	}
	return FSlateColor(FLinearColor(1.0,1.0,1.0,0.0));
}

void SUTHomePanel::OnHidePanel()
{

	PlayerOwner->GetWorld()->GetTimerManager().ClearTimer(LanTimerHandle);


	bClosing = true;
	if (AnimWidget.IsValid())
	{
		AnimWidget->Animate(FVector2D(0.0f, 0.0f), FVector2D(-100.0f, 0.0f), 1.0f, 0.0f, 0.3f);
	}
	else
	{
		SUTPanelBase::OnHidePanel();
	}
}


void SUTHomePanel::AnimEnd()
{
	if (bClosing)
	{
		bClosing = false;
		TSharedPtr<SWidget> Panel = this->AsShared();
		ParentWindow->PanelHidden(Panel);
		ParentWindow.Reset();
	}
}

TSharedRef<SWidget> SUTHomePanel::BuildRankedPlaylist()
{
	UUTGameInstance* UTGameInstance = CastChecked<UUTGameInstance>(PlayerOwner->GetGameInstance());
	if (UTGameInstance && UTGameInstance->GetPlaylistManager() && PlayerOwner->IsLoggedIn())
	{
		int32 NumPlaylists = UTGameInstance->GetPlaylistManager()->GetNumPlaylists();
		if (NumPlaylists > 0)
		{
			TSharedPtr<SHorizontalBox> FinalBox;
			TSharedPtr<SHorizontalBox> RankedBox;

			SAssignNew(FinalBox, SHorizontalBox);

			FinalBox->AddSlot()
			.AutoWidth()
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(SUTImage)
				.Image(SUTStyle::Get().GetBrush("UT.HomePanel.NewFragCenter.Transparent"))
				.WidthOverride(196).HeightOverride(196)
			];

			FinalBox->AddSlot().AutoWidth().VAlign(VAlign_Bottom)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("- Ranked Matches -")))
					.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Medium.Bold")

				]
				+SVerticalBox::Slot().AutoHeight().Padding(0.0f,5.0f,0.0f,0.0f)
				[
					SAssignNew(RankedBox, SHorizontalBox)
				]
			];
			
			int32 ButtonCount = 0;
			for (int32 i = 0; i < NumPlaylists; i++)
			{
				FString PlaylistName;
				int32 MaxTeamCount, MaxTeamSize, MaxPartySize, PlaylistId;

				if (UTGameInstance->GetPlaylistManager()->GetPlaylistId(i, PlaylistId) &&
					PlayerOwner->IsRankedMatchmakingEnabled(PlaylistId) &&
					UTGameInstance->GetPlaylistManager()->GetPlaylistName(PlaylistId, PlaylistName) &&
					UTGameInstance->GetPlaylistManager()->GetMaxTeamInfoForPlaylist(PlaylistId, MaxTeamCount, MaxTeamSize, MaxPartySize))
				{
					FString PlaylistPlayerCount = FString::Printf(TEXT("%dv%d"), MaxTeamSize, MaxTeamSize);
					FName SlateBadgeName = UTGameInstance->GetPlaylistManager()->GetPlaylistSlateBadge(PlaylistId);
					if (SlateBadgeName == NAME_None) SlateBadgeName = FName(TEXT("UT.HomePanel.DMBadge"));

					ButtonCount++;
					RankedBox->AddSlot()
					.AutoWidth()

					.Padding(FMargin(2.0f, 0.0f))
					[
						SNew(SUTButton)
						.ButtonStyle(SUTStyle::Get(), "UT.HomePanel.Button")
						.bSpringButton(true)
						.OnClicked(FOnClicked::CreateSP(this, &SUTHomePanel::OnStartRankedPlaylist, PlaylistId))
						.ToolTip(SUTUtils::CreateTooltip(NSLOCTEXT("SUTHomePanel","Ranked","Play a ranked match and earn XP.")))
						[

							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBox)
								.WidthOverride(128)
								.HeightOverride(128)
								[
									SNew(SImage)
									.Image(SUTStyle::Get().GetBrush(SlateBadgeName))
								]
							]
							+ SOverlay::Slot()
							[
								SNew(SBox)
								.WidthOverride(128)
								.HeightOverride(128)
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Top)
									.FillHeight(1.0f)
									[
										SNew(STextBlock)
										.Text(FText::FromString(PlaylistName))
										.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small.Bold")
									]
									+SVerticalBox::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Center)
									.AutoHeight()
									[
										SNew(SBorder)
										.BorderImage(SUTStyle::Get().GetBrush("UT.HeaderBackground.Shaded"))
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot()
											.AutoHeight()
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											.Padding(0.0f, 0.0f, 0.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(FText::FromString(PlaylistPlayerCount))
												.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Medium.Bold")
											]
										]
									]
								]
							]
						]
					];					
				}
			}

			if (ButtonCount > 0)
			{
				return FinalBox.ToSharedRef();
			}

			FinalBox->ClearChildren();
		}
	}
	return SNullWidget::NullWidget;
}

FReply SUTHomePanel::OnStartRankedPlaylist(int32 PlaylistId)
{
	if (PlayerOwner.IsValid())
	{
		if (!PlayerOwner->IsRankedMatchmakingEnabled(PlaylistId))
		{
			PlayerOwner->ShowToast(NSLOCTEXT("SUTPartyWidget", "RankedPlayDisabled", "This playlist is not currently active"));
			return FReply::Handled();
		}

		UMatchmakingContext* MatchmakingContext = Cast<UMatchmakingContext>(UBlueprintContextLibrary::GetContext(PlayerOwner->GetWorld(), UMatchmakingContext::StaticClass()));
		if (MatchmakingContext)
		{
			MatchmakingContext->StartMatchmaking(PlaylistId);
		}
	}

	return FReply::Handled();
}


FReply SUTHomePanel::OnJoinLanClicked(TSharedPtr<FServerData> Server)
{
	TSharedPtr<SUTServerBrowserPanel> Browser = PlayerOwner->GetServerBrowser(false);
	if (Browser.IsValid() && Server.IsValid())
	{
		Browser->ConnectTo(*Server, false);	
	}

	return FReply::Handled();
}

FReply SUTHomePanel::OnSpectateLanClicked(TSharedPtr<FServerData> Server)
{
	TSharedPtr<SUTServerBrowserPanel> Browser = PlayerOwner->GetServerBrowser(false);
	if (Browser.IsValid() && Server.IsValid())
	{
		Browser->ConnectTo(*Server, true);	
	}

	return FReply::Handled();

}


#endif