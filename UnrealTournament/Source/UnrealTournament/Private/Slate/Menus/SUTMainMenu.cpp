// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLocalPlayer.h"
#include "SlateBasics.h"
#include "Slate/SlateGameResources.h"
#include "SUTMainMenu.h"
#include "../SUWindowsStyle.h"
#include "../Base/SUTDialogBase.h"
#include "../Dialogs/SUTSystemSettingsDialog.h"
#include "../Dialogs/SUTPlayerSettingsDialog.h"
#include "../Dialogs/SUTControlSettingsDialog.h"
#include "../Dialogs/SUTInputBoxDialog.h"
#include "../Dialogs/SUTMessageBoxDialog.h"
#include "../Dialogs/SUTGameSetupDialog.h"
#include "../Widgets/SUTScaleBox.h"
#include "../Dialogs/SUTDifficultyLevel.h"
#include "UTGameEngine.h"
#include "../Panels/SUTServerBrowserPanel.h"
#include "../Panels/SUTReplayBrowserPanel.h"
#include "../Panels/SUTStatsViewerPanel.h"
#include "../Panels/SUTCreditsPanel.h"
#include "../Panels/SUTChallengePanel.h"
#include "../Panels/SUTHomePanel.h"
#include "../Panels/SUTFragCenterPanel.h"
#include "UTEpicDefaultRulesets.h"
#include "UTReplicatedGameRuleset.h"
#include "UTAnalytics.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "../Panels/SUTUMGPanel.h"

#if !UE_SERVER

#include "UserWidget.h"
#include "AnalyticsEventAttribute.h"
#include "IAnalyticsProvider.h"

void SUTMainMenu::CreateDesktop()
{
	bNeedToShowGamePanel = false;
	SUTMenuBase::CreateDesktop();
}

SUTMainMenu::~SUTMainMenu()
{
	HomePanel.Reset();
}

TSharedRef<SWidget> SUTMainMenu::BuildBackground()
{
	return SNew(SOverlay)
	+SOverlay::Slot()
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SUTScaleBox)
				.bMaintainAspectRatio(false)
				[
					SNew(SImage)
					.Image(SUTStyle::Get().GetBrush("UT.HomePanel.Background"))
				]
			]
		]
	];
}

FReply SUTMainMenu::OnFragCenterClick()
{
	ShowFragCenter();
	return FReply::Handled();
}

void SUTMainMenu::DeactivatePanel(TSharedPtr<class SUTPanelBase> PanelToDeactivate)
{
	if (FragCenterPanel.IsValid()) FragCenterPanel.Reset();
	if (WebPanel.IsValid()) WebPanel.Reset();

	SUTMenuBase::DeactivatePanel(PanelToDeactivate);
}

void SUTMainMenu::ShowFragCenter()
{
	if (!FragCenterPanel.IsValid())
	{
		SAssignNew(FragCenterPanel, SUTFragCenterPanel, PlayerOwner)
			.ViewportSize(FVector2D(1920, 1020))
			.AllowScaling(true)
			.ShowControls(false);

		if (FragCenterPanel.IsValid())
		{
			FragCenterPanel->Browse(TEXT("http://www.unrealtournament.com/fragcenter"));
			ActivatePanel(FragCenterPanel);
		}
	}
}

void SUTMainMenu::SetInitialPanel()
{
	SAssignNew(HomePanel, SUTHomePanel, PlayerOwner);

	if (HomePanel.IsValid())
	{
		ActivatePanel(HomePanel);
	}
}

FReply SUTMainMenu::OnShowHomePanel()
{
	//if we are leaving the tutorial panel, we may have canceled on boarding
	if (FUTAnalytics::IsAvailable() && ActivePanel == TutorialPanel)
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(PlayerOwner->PlayerController);
		if (PC && !PC->SkipTutorialCheck())
		{
			UUTLocalPlayer* UTLocalPlayer = Cast<UUTLocalPlayer>(PC->Player);
			if (UTLocalPlayer)
			{
				int32 TutorialMask = UTLocalPlayer->GetTutorialMask();
				if (TutorialMask == 0 || (TutorialMask & TUTORIAL_SkillMoves) != TUTORIAL_SkillMoves) 
				{
					FUTAnalytics::FireEvent_UTCancelOnboarding(PC);
				}
			}
		}
	}

	return SUTMenuBase::OnShowHomePanel();
}

/****************************** [ Build Sub Menus ] *****************************************/

void SUTMainMenu::BuildLeftMenuBar()
{
	if (LeftMenuBar.IsValid())
	{
		LeftMenuBar->AddSlot()
		.Padding(50.0f, 0.0f, 0.0f, 0.0f)
		.AutoWidth()
		[
			AddPlayNow()
		];

		LeftMenuBar->AddSlot()
		.Padding(40.0f,0.0f,0.0f,0.0f)
		.AutoWidth()
		[
			BuildTutorialSubMenu()
		];
		
		LeftMenuBar->AddSlot()
		.Padding(40.0f,0.0f,0.0f,0.0f)
		.AutoWidth()
		[
			BuildWatchSubMenu()
		];
	}
}

TSharedRef<SWidget> SUTMainMenu::BuildWatchSubMenu()
{
	TSharedPtr<SUTComboButton> DropDownButton = NULL;
		SNew(SBox)
	.HeightOverride(56)
	[
		SAssignNew(DropDownButton, SUTComboButton)
		.HasDownArrow(false)
		.Text(NSLOCTEXT("SUTMenuBase", "MenuBar_REPLAYS", "WATCH"))
		.TextStyle(SUTStyle::Get(), "UT.Font.MenuBarText")
		.ContentPadding(FMargin(35.0,0.0,35.0,0.0))
		.ContentHAlign(HAlign_Left)
	];

//	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Watch_FragCenter", "Frag Center"), FOnClicked::CreateSP(this, &SUTMainMenu::OnFragCenterClick));
//	DropDownButton->AddSpacer();
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Replays_YourReplays", "Your Replays"), FOnClicked::CreateSP(this, &SUTMainMenu::OnYourReplaysClick));
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Replays_RecentReplays", "Recent Replays"), FOnClicked::CreateSP(this, &SUTMainMenu::OnRecentReplaysClick));
	DropDownButton->AddSpacer();
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Replays_LiveGames", "Live Games"), FOnClicked::CreateSP(this, &SUTMainMenu::OnLiveGameReplaysClick), true);

	return DropDownButton.ToSharedRef();
}

TSharedRef<SWidget> SUTMainMenu::BuildTutorialSubMenu()
{
	TSharedPtr<SUTComboButton> DropDownButton = NULL;
	SNew(SBox)
	.HeightOverride(56)
	[
		SAssignNew(DropDownButton, SUTComboButton)
		.HasDownArrow(false)
		.Text(NSLOCTEXT("SUTMenuBase", "MenuBar_TUTORIAL", "LEARN"))
		.TextStyle(SUTStyle::Get(), "UT.Font.MenuBarText")
		.ContentPadding(FMargin(35.0,0.0,35.0,0.0))
		.ContentHAlign(HAlign_Left)
	];

	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Tutorial_LeanHoToPlay", "Basic Training"), FOnClicked::CreateSP(this, &SUTMainMenu::OnBootCampClick));
	DropDownButton->AddSpacer();
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_Tutorial_Community", "Training Videos"), FOnClicked::CreateSP(this, &SUTMainMenu::OnCommunityClick), true);

	return DropDownButton.ToSharedRef();

}


TSharedRef<SWidget> SUTMainMenu::AddPlayNow()
{
	TSharedPtr<SUTComboButton> DropDownButton = NULL;

	SNew(SBox)
	.HeightOverride(56)
	[
		SAssignNew(DropDownButton, SUTComboButton)
		.HasDownArrow(false)
		.Text(NSLOCTEXT("SUTMenuBase", "MenuBar_QuickMatch", "PLAY"))
		.TextStyle(SUTStyle::Get(), "UT.Font.MenuBarText")
		.ContentPadding(FMargin(35.0,0.0,35.0,0.0))
		.ContentHAlign(HAlign_Left)
	];

	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_QuickMatch_PlayDM", "QuickPlay Deathmatch"), FOnClicked::CreateSP(this, &SUTMainMenu::OnPlayQuickMatch,	EEpicDefaultRuleTags::Deathmatch));
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_QuickMatch_PlayFlagRun", "QuickPlay Flag Run"), FOnClicked::CreateSP(this, &SUTMainMenu::OnPlayQuickMatch, EEpicDefaultRuleTags::FlagRun));
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_QuickMatch_PlayFlagRunVSAI", "QuickPlay Flag Run Coop vs AI"), FOnClicked::CreateSP(this, &SUTMainMenu::OnPlayQuickMatch, EEpicDefaultRuleTags::FlagRunVSAI));
	DropDownButton->AddSpacer();
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_ChallengesGame", "Single Player Challenges"), FOnClicked::CreateSP(this, &SUTMainMenu::OnShowGamePanel));
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_CreateGame", "Custom Single Player Match"), FOnClicked::CreateSP(this, &SUTMainMenu::OnShowCustomGamePanel));
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_StartLANGame", "Start LAN Match"), FOnClicked::CreateSP(this, &SUTMainMenu::OnShowCustomGamePanel));

	DropDownButton->AddSpacer();
	DropDownButton->AddSubMenuItem(NSLOCTEXT("SUTMenuBase", "MenuBar_QuickMatch_FindGame", "Find a Match..."), FOnClicked::CreateSP(this, &SUTMenuBase::OnShowServerBrowserPanel),true);
	
	return DropDownButton.ToSharedRef();
}

FReply SUTMainMenu::OnCloseClicked()
{

	for (int32 i=0; i<AvailableGameRulesets.Num();i++)
	{
		AvailableGameRulesets[i]->SlateBadge = NULL;
	}


	PlayerOwner->HideMenu();
	ConsoleCommand(TEXT("quit"));
	return FReply::Handled();
}



FReply SUTMainMenu::OnShowGamePanel()
{
	ShowGamePanel();
	return FReply::Handled();
}

FReply SUTMainMenu::OnShowCustomGamePanel()
{
	ShowCustomGamePanel();
	return FReply::Handled();
}


void SUTMainMenu::ShowGamePanel()
{
	bool bIsInParty = false;
	UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(PlayerOwner->GetWorld(), UPartyContext::StaticClass()));
	if (PartyContext)
	{
		bIsInParty = PartyContext->GetPartySize() > 1;
	}

	if (bIsInParty)
	{
		PlayerOwner->ShowToast(NSLOCTEXT("SUTMenuBase", "ShowGamePanelNotLeader", "You may not do challenges while in a party"));
		return;
	}

	if ( !ChallengePanel.IsValid() )
	{
		SAssignNew(ChallengePanel, SUTChallengePanel, PlayerOwner);
	}

	ActivatePanel(ChallengePanel);
}

void SUTMainMenu::ShowCustomGamePanel()
{
	bool bIsInParty = false;
	UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(PlayerOwner->GetWorld(), UPartyContext::StaticClass()));
	if (PartyContext)
	{
		bIsInParty = PartyContext->GetPartySize() > 1;
	}

	if (bIsInParty)
	{
		PlayerOwner->ShowToast(NSLOCTEXT("SUTMenuBase", "ShowCustomGamePanelNotLeader", "You may not do custom matches while in a party"));
		return;
	}

	if (TickCountDown <= 0)
	{
		PlayerOwner->ShowContentLoadingMessage();
		bNeedToShowGamePanel = true;
		TickCountDown = 3;
	}
}

void SUTMainMenu::OpenDelayedMenu()
{
	SUTMenuBase::OpenDelayedMenu();
	if (bNeedToShowGamePanel)
	{
		bNeedToShowGamePanel = false;
		AvailableGameRulesets.Empty();
		TArray<FString> AllowedGameRulesets;

		UUTEpicDefaultRulesets* DefaultRulesets = UUTEpicDefaultRulesets::StaticClass()->GetDefaultObject<UUTEpicDefaultRulesets>();
		if (DefaultRulesets && DefaultRulesets->AllowedRulesets.Num() > 0)
		{
			AllowedGameRulesets.Append(DefaultRulesets->AllowedRulesets);
		}

		// If someone has screwed up the ini completely, just load all of the Epic defaults
		if (AllowedGameRulesets.Num() <= 0)
		{
			UUTEpicDefaultRulesets::GetEpicRulesets(AllowedGameRulesets);
		}

		// Grab all of the available map assets.
		TArray<FAssetData> MapAssets;
		GetAllAssetData(UWorld::StaticClass(), MapAssets, false);

		UE_LOG(UT,Verbose,TEXT("Loading Settings for %i Rules"), AllowedGameRulesets.Num())
		for (int32 i=0; i < AllowedGameRulesets.Num(); i++)
		{
			UE_LOG(UT,Verbose,TEXT("Loading Rule %s"), *AllowedGameRulesets[i])
			if (!AllowedGameRulesets[i].IsEmpty())
			{
				FName RuleName = FName(*AllowedGameRulesets[i]);
				UUTGameRuleset* NewRuleset = NewObject<UUTGameRuleset>(GetTransientPackage(), RuleName, RF_Transient);
				if (NewRuleset)
				{
					NewRuleset->UniqueTag = AllowedGameRulesets[i];
					bool bExistsAlready = false;
					for (int32 j=0; j < AvailableGameRulesets.Num(); j++)
					{
						if ( AvailableGameRulesets[j]->UniqueTag.Equals(NewRuleset->UniqueTag, ESearchCase::IgnoreCase) || AvailableGameRulesets[j]->Title.ToLower() == NewRuleset->Title.ToLower() )
						{
							bExistsAlready = true;
							break;
						}
					}

					if ( !bExistsAlready )
					{
						// Before we create the replicated version of this rule.. if it's an epic rule.. insure they are using our defaults.
						UUTEpicDefaultRulesets::InsureEpicDefaults(NewRuleset);

						FActorSpawnParameters Params;
						Params.Owner = PlayerOwner->GetWorld()->GetGameState();
						AUTReplicatedGameRuleset* NewReplicatedRuleset = PlayerOwner->GetWorld()->SpawnActor<AUTReplicatedGameRuleset>(Params);
						if (NewReplicatedRuleset)
						{
							// Build out the map info
							NewReplicatedRuleset->SetRules(NewRuleset, MapAssets);

							// If this ruleset doesn't have any maps, then don't use it
							if (NewReplicatedRuleset->MapList.Num() > 0)
							{
								AvailableGameRulesets.Add(NewReplicatedRuleset);
							}
							else
							{
								UE_LOG(UT,Warning,TEXT("Detected a ruleset [%s] that has no maps"), *NewRuleset->UniqueTag);
								NewReplicatedRuleset->Destroy();
							}
						}
					}
					else
					{
						UE_LOG(UT,Verbose,TEXT("Rule %s already exists."), *AllowedGameRulesets[i]);
					}
				}
			}
		}
	
		for (int32 i=0; i < AvailableGameRulesets.Num(); i++)
		{
			AvailableGameRulesets[i]->BuildSlateBadge();
		}

		if (AvailableGameRulesets.Num() > 0)
		{
			
			SAssignNew(CreateGameDialog, SUTGameSetupDialog)
			.PlayerOwner(PlayerOwner)
			.GameRuleSets(AvailableGameRulesets)
			.DialogSize(FVector2D(1920,1080))
#if PLATFORM_WINDOWS
			.ButtonMask(UTDIALOG_BUTTON_PLAY | UTDIALOG_BUTTON_LAN | UTDIALOG_BUTTON_CANCEL);
#else
			.ButtonMask(UTDIALOG_BUTTON_PLAY | UTDIALOG_BUTTON_CANCEL);
#endif

			if ( CreateGameDialog.IsValid() )
			{
				PlayerOwner->OpenDialog(CreateGameDialog.ToSharedRef(), 100);
			}
	
		}
	}
	PlayerOwner->HideContentLoadingMessage();
}

FReply SUTMainMenu::OnPlayQuickMatch(FString QuickMatchType)
{
	QuickPlay(QuickMatchType);
	return FReply::Handled();
}


void SUTMainMenu::QuickPlay(const FString& QuickMatchType)
{
	if (!PlayerOwner->IsPartyLeader())
	{
		PlayerOwner->ShowToast(NSLOCTEXT("SUTMenuBase", "QuickPlayNotLeader", "Only the party leader may start Quick Play"));
		return;
	}

	if (!PlayerOwner->IsLoggedIn())
	{
		PlayerOwner->GetAuth();
		return;
	}

	if (QuickMatchType == EEpicDefaultRuleTags::FlagRunVSAI)
	{
		DifficultyLevelDialog = SNew(SUTDifficultyLevel).PlayerOwner(PlayerOwner).OnDialogResult(FDialogResultDelegate::CreateSP(this, &SUTMainMenu::DifficultyResult));
		if (DifficultyLevelDialog.IsValid())
		{
			PlayerOwner->OpenDialog(DifficultyLevelDialog.ToSharedRef());
			return;
		}
	}

	UE_LOG(UT,Log,TEXT("QuickMatch: %s"),*QuickMatchType);
	PlayerOwner->StartQuickMatch(QuickMatchType);
}

void SUTMainMenu::DifficultyResult(TSharedPtr<SCompoundWidget> Widget, uint16 ButtonID)
{
	if (ButtonID != UTDIALOG_BUTTON_CANCEL && DifficultyLevelDialog.IsValid())
	{
		FString QuickMatchType = TEXT("");
		switch (DifficultyLevelDialog->GetDifficulty())
		{
			case 1 : 
				QuickMatchType = EEpicDefaultRuleTags::FlagRunVSAINormal; 
				break;

			case 2: 
				QuickMatchType = EEpicDefaultRuleTags::FlagRunVSAIHard; 
				break;

			default: 
				QuickMatchType = EEpicDefaultRuleTags::FlagRunVSAIEasy; 
				break;

		}
		PlayerOwner->StartQuickMatch(QuickMatchType);
		DifficultyLevelDialog.Reset();
	}
}

FReply SUTMainMenu::OnBootCampClick()
{
	OpenTutorialMenu();
	return FReply::Handled();
}

void SUTMainMenu::OpenTutorialMenu()
{
	bool bIsInParty = false;
	UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(PlayerOwner->GetWorld(), UPartyContext::StaticClass()));
	if (PartyContext)
	{
		bIsInParty = PartyContext->GetPartySize() > 1;
	}

	if (bIsInParty)
	{
		PlayerOwner->ShowToast(NSLOCTEXT("SUTMenuBase", "TutorialNotLeader", "You may not enter tutorials while in a party"));
		return;
	}

	if (!TutorialPanel.IsValid())
	{
		SAssignNew(TutorialPanel,SUTUMGPanel,PlayerOwner).UMGClass(TEXT("/Game/RestrictedAssets/Tutorials/Blueprints/TutMainMenuWidget.TutMainMenuWidget_C"));
	}

	if (TutorialPanel.IsValid() && ActivePanel != TutorialPanel)
	{
		ActivatePanel(TutorialPanel);
	}
}


FReply SUTMainMenu::OnYourReplaysClick()
{
	if (!PlayerOwner->IsLoggedIn())
	{
		PlayerOwner->LoginOnline( TEXT( "" ), TEXT( "" ) );
		return FReply::Handled();
	}

	TSharedPtr<class SUTReplayBrowserPanel> ReplayBrowser = PlayerOwner->GetReplayBrowser();
	if (ReplayBrowser.IsValid())
	{
		ReplayBrowser->bLiveOnly = false;
		ReplayBrowser->bShowReplaysFromAllUsers = false;
		ReplayBrowser->MetaString = TEXT("");

		if (ReplayBrowser == ActivePanel)
		{
			ReplayBrowser->BuildReplayList(PlayerOwner->GetPreferredUniqueNetId()->ToString());
		}
		else
		{
			ActivatePanel(ReplayBrowser);
		}
	}

	return FReply::Handled();
}

FReply SUTMainMenu::OnRecentReplaysClick()
{
	RecentReplays();
	return FReply::Handled();
}

void SUTMainMenu::RecentReplays()
{
	if (!PlayerOwner->IsLoggedIn())
	{
		PlayerOwner->LoginOnline( TEXT( "" ), TEXT( "" ) );
		return;
	}

	TSharedPtr<class SUTReplayBrowserPanel> ReplayBrowser = PlayerOwner->GetReplayBrowser();
	if (ReplayBrowser.IsValid())
	{
		ReplayBrowser->bLiveOnly = false;
		ReplayBrowser->bShowReplaysFromAllUsers = true;
		ReplayBrowser->MetaString = TEXT("");

		if (ReplayBrowser == ActivePanel)
		{
			ReplayBrowser->BuildReplayList(TEXT(""));
		}
		else
		{
			ActivatePanel(ReplayBrowser);
		}
	}
}

FReply SUTMainMenu::OnLiveGameReplaysClick()
{
	ShowLiveGameReplays();

	return FReply::Handled();
}

void SUTMainMenu::ShowLiveGameReplays()
{
	if (!PlayerOwner->IsLoggedIn())
	{
		PlayerOwner->LoginOnline( TEXT( "" ), TEXT( "" ) );
		return;
	}

	TSharedPtr<class SUTReplayBrowserPanel> ReplayBrowser = PlayerOwner->GetReplayBrowser();
	if (ReplayBrowser.IsValid())
	{
		ReplayBrowser->bLiveOnly = true;
		ReplayBrowser->bShowReplaysFromAllUsers = true;
		ReplayBrowser->MetaString = TEXT("");

		if (ReplayBrowser == ActivePanel)
		{
			ReplayBrowser->BuildReplayList(TEXT(""));
		}
		else
		{
			ActivatePanel(ReplayBrowser);
		}
	}
}

FReply SUTMainMenu::OnCommunityClick()
{
	ShowCommunity();
	return FReply::Handled();
}

void SUTMainMenu::ShowCommunity()
{
	if ( !WebPanel.IsValid() )
	{
		TSharedPtr<SUTWebBrowserPanel> NewWebPanel;

		// Create the Web panel
		SAssignNew(NewWebPanel, SUTWebBrowserPanel, PlayerOwner);
		if (NewWebPanel.IsValid())
		{
			if (ActivePanel.IsValid() && ActivePanel != NewWebPanel)
			{
				ActivatePanel(NewWebPanel);
			}
			NewWebPanel->Browse(CommunityVideoURL);
			WebPanel = NewWebPanel;
		}
	}
}

bool SUTMainMenu::ShouldShowBrowserIcon()
{
	return (PlayerOwner.IsValid() && PlayerOwner->bShowBrowserIconOnMainMenu);
}

FReply SUTMainMenu::OnShowServerBrowserPanel()
{
	return SUTMenuBase::OnShowServerBrowserPanel();

}

void SUTMainMenu::OnMenuOpened(const FString& Parameters)
{
	SUTMenuBase::OnMenuOpened(Parameters);
	if (Parameters.Equals(TEXT("showchallenge"), ESearchCase::IgnoreCase))
	{
		ShowGamePanel();
	}
}

#endif