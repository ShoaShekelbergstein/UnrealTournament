// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLocalPlayer.h"
#include "SUTQuickMatchWindow.h"
#include "../SUTStyle.h"
#include "../Widgets/SUTButton.h"
#include "UTOnlineGameSearchBase.h"
#include "UTOnlineGameSettingsBase.h"
#include "OnlineSubsystemTypes.h"
#include "UTGameEngine.h"
#include "UTServerBeaconClient.h"
#include "Engine/UserInterfaceSettings.h"
#include "UnrealNetwork.h"
#include "UTDMGameMode.h"
#include "UTCTFBaseGame.h"
#include "UTTeamShowdownGame.h"
#include "BlueprintContextLibrary.h"
#include "PartyContext.h"
#include "UTGameInstance.h"
#include "UTParty.h"
#include "UTPartyGameState.h"
#include "UTMcpUtils.h"
#include "UTAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Analytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"

#if !UE_SERVER

SUTQuickMatchWindow::~SUTQuickMatchWindow()
{
	DefaultGameModeObject.Reset();
	
	UUTGameInstance* UTGameInstance = Cast<UUTGameInstance>(PlayerOwner->GetGameInstance());
	UUTParty* Party = UTGameInstance->GetParties();
	if (Party)
	{
		UUTPartyGameState* PersistentParty = Cast<UUTPartyGameState>(Party->GetPersistentParty());
		if (PersistentParty)
		{
			if (bCancelQuickmatch)
			{
				PersistentParty->SetPartyCancelQuickMatch();
			}
			else
			{
				PersistentParty->SetPartyJoiningQuickMatch();
			}
		}
	}
}

void SUTQuickMatchWindow::Construct(const FArguments& InArgs, TWeakObjectPtr<UUTLocalPlayer> InPlayerOwner)
{
	PlayerOwner = InPlayerOwner;
	QuickMatchType = InArgs._QuickMatchType;
	bWaitingForMatch = false;
	checkSlow(PlayerOwner != NULL);
	StartTime = PlayerOwner->GetWorld()->GetRealTimeSeconds();

	QMStats_NumHubsConsidered = 0;
	QMStats_NumInstancesConsidered = 0;
	QMStats_NumRejectedForRank = 0;
	QMStats_NumRejectedForPartySize = 0;
	QMStats_NumRejectedForGameType = 0;
	QMStats_NumRejectedForJoinable = 0;
	QMStats_NumPingFailures = 0;
	QMStats_NumInstancesSpooled = 0;
	QMStats_NumAttemptedJoins = 0;
	QMStats_RankCheck = 0;
	QMStats_FinalResult = TEXT("Invalid");

	DefaultGameModeObject = nullptr;
	UClass* GameModeClass = AUTDMGameMode::StaticClass();

	if (QuickMatchType.Equals(EEpicDefaultRuleTags::CTF, ESearchCase::IgnoreCase))
	{
		GameModeClass = AUTCTFBaseGame::StaticClass();
	}
	else if (QuickMatchType.Equals(EEpicDefaultRuleTags::TEAMSHOWDOWN, ESearchCase::IgnoreCase))
	{
		GameModeClass = AUTTeamShowdownGame::StaticClass();
	}

	if (GameModeClass)
	{
		DefaultGameModeObject = GameModeClass->GetDefaultObject<AUTBaseGameMode>();
	}

	SUTWindowBase::Construct
	(
		SUTWindowBase::FArguments()
			.Size(FVector2D(800, 220))
			.bSizeIsRelative(false)
			.Position(FVector2D(0.5f, 0.5f))
			.AnchorPoint(FVector2D(0.5f, 0.5f))
			.bShadow(true)

		, PlayerOwner

	);
	
	UUTGameInstance* UTGameInstance = Cast<UUTGameInstance>(PlayerOwner->GetGameInstance());
	UUTParty* Party = UTGameInstance->GetParties();
	if (Party)
	{
		UUTPartyGameState* PersistentParty = Cast<UUTPartyGameState>(Party->GetPersistentParty());
		if (PersistentParty)
		{
			PersistentParty->SetPartyQuickMatching();
		}
	}
}

void SUTQuickMatchWindow::BuildWindow()
{
	// this is the background image
	Content->AddSlot()
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SImage)
			.Image(SUTStyle::Get().GetBrush("UT.HeaderBackground.Dark"))
		]
	];

	// This will define a vertical box that holds the various components of the dialog box.
	Content->AddSlot()
	[
		SNew(SVerticalBox)

		// The title bar
		+ SVerticalBox::Slot()						
		.Padding(0.0f, 5.0f, 0.0f, 5.0f)
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(46)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("QuickMatch","SearchingForGame","FINDING A GAME TO JOIN"))
				.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Large")
			]
		]
		+ SVerticalBox::Slot()
		.Padding(12.0f, 25.0f, 12.0f, 25.0f)
		.AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SUTQuickMatchWindow::GetStatusText)
				.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Medium")
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SUTQuickMatchWindow::GetMinorStatusText)
				.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Tiny")
			]
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0.0f,10.0f,0.0f,0.0f)
			[
				SNew(SUTButton)
				.ButtonStyle(SUTStyle::Get(), "UT.SimpleButton.Dark")
				.OnClicked(this, &SUTQuickMatchWindow::OnCancelClick)
				.ContentPadding(FMargin(25.0, 0.0, 25.0, 5.0))
				.Text(NSLOCTEXT("QuickMatch", "CancelText", "ESC to Cancel"))
				.TextStyle(SUTStyle::Get(), "UT.Font.NormalText.Small")
			]
		]

	];

	UUTGameInstance* UTGameInstance = Cast<UUTGameInstance>(PlayerOwner->GetGameInstance());
	AUTPlayerState* PlayerState = Cast<AUTPlayerState>(GetPlayerOwner()->PlayerController->PlayerState);
	MatchTargetRank = (PlayerState != nullptr && DefaultGameModeObject != nullptr) ? PlayerState->GetRankCheck(DefaultGameModeObject.Get()) : NUMBER_RANK_LEVELS * 4;
	TArray<FUniqueNetIdRepl> AccountIds;

	bool bStartQuickMatch = true;

	UUTMcpUtils* McpUtils = nullptr;
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr OnlineIdentityInterface = OnlineSubsystem->GetIdentityInterface();
		if (OnlineIdentityInterface.IsValid())
		{
			McpUtils = UUTMcpUtils::Get(PlayerOwner->GetWorld(), OnlineIdentityInterface->GetUniquePlayerId(PlayerOwner->GetControllerId()));
			if (McpUtils == nullptr)
			{
				UE_LOG(LogOnline, Warning, TEXT("Unable to load McpUtils. Will not be able to read ELO from MCP"));
			}
		}
	}

	if (McpUtils && UTGameInstance)
	{
		UUTParty* Parties = UTGameInstance->GetParties();
		if (Parties)
		{
			UPartyGameState* Party = Parties->GetPersistentParty();
			if (Party)
			{
				TArray<UPartyMemberState*> PartyMembers;
				Party->GetAllPartyMembers(PartyMembers);

				for (int32 i = 0; i < PartyMembers.Num(); i++)
				{
					AccountIds.Add(PartyMembers[i]->UniqueId);
				}

				bStartQuickMatch = false;
				TSharedPtr<SUTQuickMatchWindow> Myself = SharedThis(this);
				McpUtils->GetTeamHighestMmr(GetNameSafe(DefaultGameModeObject->GetClass()), AccountIds, [Myself](const FOnlineError& TeamMmrResult, const FHighestMmr& Response)
				{
					if (Myself.IsValid() && Myself->GetPlayerOwner().IsValid() && Myself->GetPlayerOwner()->bIsQuickmatchDialogOpen())
					{
						if (!TeamMmrResult.bSucceeded)
						{
							UE_LOG(UT,Log,TEXT("Best MMR: %i"),Response.Mmr);
							Myself->MatchTargetRank = Response.Mmr;
						}
						Myself->BeginQuickmatch();
					}
				});
			}
		}
	}

	QMStats_RankCheck = MatchTargetRank;

	if (bStartQuickMatch)
	{
		BeginQuickmatch();
	}

	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EKeyboardFocusCause::Keyboard);
}


FText SUTQuickMatchWindow::GetStatusText() const
{
	int32 DeltaSeconds = int32(PlayerOwner->GetWorld()->GetRealTimeSeconds() - StartTime);

	if (bWaitingForMatch)
	{
		return FText::Format(NSLOCTEXT("QuickMatch","StatusFormatStrWait","Starting up Match... ({0})"), FText::AsNumber(DeltaSeconds));
	}

	return FText::Format(NSLOCTEXT("QuickMatch","StatusFormatStr","Searching for a game... ({0})"), FText::AsNumber(DeltaSeconds));
}

FText SUTQuickMatchWindow::GetMinorStatusText() const
{
	return MinorStatusText;
}


void SUTQuickMatchWindow::BeginQuickmatch()
{
	bCancelQuickmatch = false;
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	IOnlineIdentityPtr OnlineIdentityInterface;
	IOnlineSessionPtr OnlineSessionInterface;
	if (OnlineSubsystem) OnlineIdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (OnlineSubsystem) OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

	if (OnlineSessionInterface.IsValid())
	{
		// First step, cancel out any existing MCP searches...

		FOnCancelFindSessionsCompleteDelegate Delegate;
		Delegate.BindSP(this, &SUTQuickMatchWindow::OnInitialFindCancel);
		OnCancelFindSessionCompleteHandle = OnlineSessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(Delegate);
		OnlineSessionInterface->CancelFindSessions();
	}
}

void SUTQuickMatchWindow::OnInitialFindCancel(bool bWasSuccessful)
{
	// We don't really care if this succeeded since a failure just means there were
	// no sessions.

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr OnlineSessionInterface;
	if (OnlineSubsystem) OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	if (OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionCompleteHandle);
		OnCancelFindSessionCompleteHandle.Reset();
	}

	QMStats_FinalResult = TEXT("OSS_Cancelled");

	if (bCancelQuickmatch)
	{
		PlayerOwner->CloseQuickMatch();
	}
	else
	{
		FindHUBToJoin();
	}
}

void SUTQuickMatchWindow::FindHUBToJoin()
{
	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr OnlineSessionInterface;
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}

	if (OnlineSessionInterface.IsValid())
	{
		// Setup our Find complete Delegate
		FOnFindSessionsCompleteDelegate Delegate;
		Delegate.BindSP(this, &SUTQuickMatchWindow::OnFindSessionsComplete);
		OnFindSessionCompleteHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(Delegate);

		// Now look for official hubs

		SearchSettings = MakeShareable(new FUTOnlineGameSearchBase(false));
		SearchSettings->MaxSearchResults = 10000;
		FString GameVer = FString::Printf(TEXT("%i"), FNetworkVersion::GetLocalNetworkVersion());
		SearchSettings->QuerySettings.Set(SETTING_SERVERVERSION, GameVer, EOnlineComparisonOp::Equals);			// Must equal the game version
		SearchSettings->QuerySettings.Set(SETTING_GAMEINSTANCE, 1, EOnlineComparisonOp::NotEquals);				// Must not be a lobby server instance
		SearchSettings->QuerySettings.Set(SETTING_RANKED, 1, EOnlineComparisonOp::NotEquals);

		FString GameMode = TEXT("/Script/UnrealTournament.UTLobbyGameMode");
		SearchSettings->QuerySettings.Set(SETTING_GAMEMODE, GameMode, EOnlineComparisonOp::Equals);				// Must be a lobby server
		
		// TODO: Add the search setting for TrustLevel

		TSharedRef<FUTOnlineGameSearchBase> SearchSettingsRef = SearchSettings.ToSharedRef();

		OnlineSessionInterface->FindSessions(0, SearchSettingsRef);
		bSearchInProgress = true;

		MinorStatusText = NSLOCTEXT("QuickMatch","Status_GettingServerList","Retreiving Server List from MCP...");
	}
}


void SUTQuickMatchWindow::OnFindSessionsComplete(bool bWasSuccessful)
{
	Instances.Empty();

	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr OnlineSessionInterface;
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}

	// Clear this delegate
	if (OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteHandle);
		OnFindSessionCompleteHandle.Reset();
	}

	if (bWasSuccessful)
	{
		// Iterate through all of the hubs and first the best one...

		if (SearchSettings->SearchResults.Num() > 0)
		{

			QMStats_NumHubsConsidered = SearchSettings->SearchResults.Num();

			for (int32 ServerIndex = 0; ServerIndex < SearchSettings->SearchResults.Num(); ServerIndex++)
			{
				int32 ServerFlags = 0x0000;
				SearchSettings->SearchResults[ServerIndex].Session.SessionSettings.Get(SETTING_SERVERFLAGS, ServerFlags);

				int32 TrustLevel = 2;
				SearchSettings->SearchResults[ServerIndex].Session.SessionSettings.Get(SETTING_TRUSTLEVEL, TrustLevel);

				// Make sure the server we are connecting to isn't password protected.
				if ((ServerFlags & 0x01) != 0x01 && TrustLevel < 2)
				{
					int32 NoPlayers;
					SearchSettings->SearchResults[ServerIndex].Session.SessionSettings.Get(SETTING_PLAYERSONLINE, NoPlayers);
					TSharedRef<FServerSearchInfo> NewServer = FServerSearchInfo::Make(SearchSettings->SearchResults[ServerIndex], 0, NoPlayers);
					ServerList.Add(NewServer);
				}
			}

			if (ServerList.Num() > 0)
			{
				PingNextBatch();
				return;
			}
		}
	}

	// We get here, we just force the find best match call.  This will fail and error out but insures and clean up happens
	FindBestMatch();
}

void SUTQuickMatchWindow::NoAvailableMatches()
{
	PlayerOwner->CloseQuickMatch();
	PlayerOwner->MessageBox(NSLOCTEXT("QuickMatch", "NoServersTitle", "ONLINE FAILURE"), NSLOCTEXT("QuickMatch", "NoServerTitle", "No quickplay instances were found.  Try finding a game using the hub browser."));
}

void SUTQuickMatchWindow::PingNextBatch()
{
	while (ServerList.Num() > 0 && PingTrackers.Num() < 10)
	{
		int32 Cnt = ServerList.Num() + PingTrackers.Num();
		MinorStatusText = FText::Format( NSLOCTEXT("QuickMatch","Status_PingingFormat","Pinging Servers ({0})"), FText::AsNumber(Cnt) );
		PingServer(ServerList[0]);
		ServerList.RemoveAtSwap(0);
	}

	if (ServerList.Num() == 0 && PingTrackers.Num() == 0)
	{
		CollectInstances();
	}
}

void SUTQuickMatchWindow::PingServer(TSharedPtr<FServerSearchInfo> ServerToPing)
{
	// Build the beacon
	AUTServerBeaconClient* Beacon = PlayerOwner->GetWorld()->SpawnActor<AUTServerBeaconClient>(AUTServerBeaconClient::StaticClass());
	if (Beacon)
	{
		ServerToPing->Beacon = Beacon;

		FString BeaconIP;
		bool bResolvedBeaconIP = false;

		IOnlineSubsystem* OnlineSubsystem;
		IOnlineSessionPtr OnlineSessionInterface;
		OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem)
		{
			OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		}
		if (OnlineSessionInterface.IsValid())
		{
			bResolvedBeaconIP = OnlineSessionInterface->GetResolvedConnectString(ServerToPing->SearchResult, FName(TEXT("BeaconPort")), BeaconIP);
		}

		if (!BeaconIP.IsEmpty())
		{
			Beacon->OnServerRequestResults = FServerRequestResultsDelegate::CreateSP(this, &SUTQuickMatchWindow::OnServerBeaconResult);
			Beacon->OnServerRequestFailure = FServerRequestFailureDelegate::CreateSP(this, &SUTQuickMatchWindow::OnServerBeaconFailure);
			FURL BeaconURL(nullptr, *BeaconIP, TRAVEL_Absolute);
			Beacon->InitClient(BeaconURL);
			PingTrackers.Add(FServerSearchPingTracker(ServerToPing, Beacon, PlayerOwner->GetWorld()->GetRealTimeSeconds()));
		}
	}
}

void SUTQuickMatchWindow::OnServerBeaconFailure(AUTServerBeaconClient* Sender)
{
	for (int32 i = 0; i < PingTrackers.Num(); i++)
	{
		if (PingTrackers[i].Beacon == Sender)
		{
			// Server is not responding, so ignore it...
			PingTrackers[i].Beacon->DestroyBeacon();
			PingTrackers.RemoveAt(i, 1);

			QMStats_NumPingFailures++;

			break;
		}
	}
	PingNextBatch();
}

void SUTQuickMatchWindow::OnServerBeaconResult(AUTServerBeaconClient* Sender, FServerBeaconInfo ServerInfo)
{
	for (int32 i = 0; i < PingTrackers.Num(); i++)
	{
		if (PingTrackers[i].Beacon == Sender)
		{
			PingTrackers[i].Server->Ping = Sender->Ping;

			// Insert sort it in to the final list of servers by ping.
			bool bInserted = false;
			for (int32 Idx=0; Idx < FinalList.Num(); Idx++)
			{
				if ( (FinalList[Idx]->ServerTrustLevel > PingTrackers[i].Server->ServerTrustLevel) || (FinalList[Idx]->Ping > Sender->Ping) )
				{
					FinalList.Insert(PingTrackers[i].Server, Idx);
					bInserted = true;
					break;					
				}
			}

			PingTrackers[i].Server->bHasFriends = HasFriendsInInstances(Sender->Instances, PlayerOwner);
			if (!bInserted) FinalList.Add(PingTrackers[i].Server);
			PingTrackers.RemoveAt(i, 1);
			break;
		}
	}

	PingNextBatch();
}

bool SUTQuickMatchWindow::HasFriendsInInstances(const TArray<TSharedPtr<FServerInstanceData>>& InstancesToCheck, TWeakObjectPtr<UUTLocalPlayer> LocalPlayer)
{
	if (PlayerOwner.IsValid())
	{
		TArray<FUTFriend> FriendsList;
		PlayerOwner->GetFriendsList(FriendsList);
		{
			for (int32 i = 0; i < InstancesToCheck.Num(); i++)
			{
				int32 Count = CountFriendsInInstance(FriendsList, InstancesToCheck[i], PlayerOwner);
				if (Count >0)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int32 SUTQuickMatchWindow::CountFriendsInInstance(const TArray<FUTFriend>& FriendsList, TSharedPtr<FServerInstanceData> InstanceToCheck, TWeakObjectPtr<UUTLocalPlayer> LocalPlayer)
{
	int32 FinalCount = 0;
	if (PlayerOwner.IsValid() && InstanceToCheck.IsValid() )
	{
		for (int32 p = 0; p < InstanceToCheck->Players.Num(); p++)
		{
			for (int32 j = 0; j < FriendsList.Num(); j++)
			{
				if (InstanceToCheck->Players[p].PlayerId == FriendsList[j].UserId)
				{
					FinalCount++;
				}
			}
		}
	}

	return FinalCount;
}

void SUTQuickMatchWindow::CollectInstances()
{
	AUTPlayerState* PlayerState = Cast<AUTPlayerState>(GetPlayerOwner()->PlayerController->PlayerState);
	bool bIsBeginner = PlayerState && PlayerState->IsABeginner(DefaultGameModeObject.Get());  

	int32 PartySize = 1;
	UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(GetPlayerOwner()->GetWorld(), UPartyContext::StaticClass()));
	if (PartyContext)
	{
		PartySize = PartyContext->GetPartySize();
		if (PartySize > 1)
		{
			bIsBeginner = false;
		}
	}

	QMStats_NumInstancesConsidered = FinalList.Num();

	for (int32 i=0; i < FinalList.Num(); i++)
	{
		if (FinalList[i]->Beacon.IsValid())
		{
			for (int32 j = 0; j < FinalList[i]->Beacon->Instances.Num(); j++)
			{
				TSharedPtr<FServerInstanceData> Instance = FinalList[i]->Beacon->Instances[j];
				if (Instance.IsValid())
				{
					// REJECT INSTANCE -- Here is where we want to cull any instances that we don't want to consider.

					// When MatchState is none, the instance is still in the menu.  We do not want to quick match to these
					// for Quick-play.  So we reject them here.
					if (Instance->MatchData.MatchState != NAME_None)
					{
						if (Instance->MaxPlayers - Instance->NumPlayers() < PartySize)
						{
							QMStats_NumRejectedForPartySize++;
						}
						// Cull any instances that do not match this quickmatch type or is not joinable as a player.
						else if (Instance->RulesTag.Equals(QuickMatchType, ESearchCase::IgnoreCase) && Instance->bJoinableAsPlayer)
						{
							// Get the Target Rank based on the quickmatch type
							if ( AUTPlayerState::CheckRank(MatchTargetRank, FinalList[i]->Beacon->Instances[j]->RankCheck, true) == 0 )
							{
								// Look to see if I could 
								Instances.Add(FInstanceTracker(FinalList[i], FinalList[i]->Beacon->Instances[j]));					
							}
							else
							{
								QMStats_NumRejectedForRank++;
							}
						}
						else
						{
							if (!Instance->bJoinableAsPlayer)
							{
								QMStats_NumRejectedForJoinable++;
							}
							else if (!Instance->RulesTag.Equals(QuickMatchType, ESearchCase::IgnoreCase))
							{
								QMStats_NumRejectedForGameType++;
							}
						}
					}
				}
			}
		}
	}

	FindBestMatch();
}


void SUTQuickMatchWindow::FindBestMatch()
{
	// At this point, FinalList contains a list of hub servers grouped first by beginner (if available), then trust level and then each group is sorted by ping.
	// Instances contains a full list of active instances available to play.  

	if (FinalList.Num() > 0)
	{
		// Step 1... look to see if there is an instance that is ready to play but not started yet and is within 50ms of the best server.
		int32 DesiredIndex = INDEX_NONE;
		for (int32 i = 0; i < Instances.Num(); i++)
		{
			if (Instances[i].InstanceData.IsValid())
			{
				int32 Ping = Instances[i].GetPing();
				if (Ping - FinalList[0]->Ping <= 50)
				{
					bool bMatchHasBegun = Instances[i].InstanceData->MatchData.bMatchHasBegun;
					if (!bMatchHasBegun)
					{
						// This is a better choice if the current choice has begun, or if our ping is better.
						if (DesiredIndex == INDEX_NONE || Instances[DesiredIndex].GetPing() > Ping || Instances[DesiredIndex].InstanceData->MatchData.bMatchHasBegun != bMatchHasBegun)
						{
							DesiredIndex = i;
						}
					}
					else if ((DesiredIndex == INDEX_NONE) || !Instances[DesiredIndex].InstanceData->MatchData.bMatchHasBegun || (Instances[DesiredIndex].GetPing() > Ping))
					{
						DesiredIndex = i;
					}
				}
			}
		}

		if (DesiredIndex != INDEX_NONE)
		{
			AttemptQuickMatch(Instances[DesiredIndex].ServerData, Instances[DesiredIndex].InstanceData);
			Instances.RemoveAt(DesiredIndex,1);
			return;
		}
	}

	// So there were no instances within 50 ms of our best hub, so attempt to join a hub.

	TSharedPtr<FServerInstanceData> Empty;
	Empty.Reset();

	if (FinalList.Num() > 0)
	{
		AttemptQuickMatch(FinalList[0], Empty);
		FinalList.RemoveAt(0,1);
	}
	else
	{
		// Start over and keep trying until a match is found or the user cancels
		BeginQuickmatch();
	}

}

void SUTQuickMatchWindow::Cancel()
{
	if (bCancelQuickmatch) return;	// Quick out if we are already cancelling

	bCancelQuickmatch = true;

	// Shut down any ping trackers and clean up the lists.

	for (int32 i=0;i<PingTrackers.Num();i++)
	{
		if (PingTrackers[i].Beacon.IsValid())
		{
			PingTrackers[i].Beacon->DestroyBeacon();
		}
	}
	PingTrackers.Empty();
	FinalList.Empty();
	Instances.Empty();


	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr OnlineSessionInterface;
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	if (OnlineSessionInterface.IsValid())
	{
		// Look to see if we are currently in the search phase.  If we are, we have to cancel it first.
		if (OnFindSessionCompleteHandle.IsValid())
		{
			// We clear the find session delegate to insure it's not fired while we are trying to cancel
			OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteHandle);
			OnFindSessionCompleteHandle.Reset();
		}

		// Tell the MCP to cancel if we are not already in that process.
		if (!OnCancelFindSessionCompleteHandle.IsValid())
		{
			// Start the cancel process
			FOnCancelFindSessionsCompleteDelegate Delegate;
			Delegate.BindSP(this, &SUTQuickMatchWindow::OnSearchCancelled);
			OnCancelFindSessionCompleteHandle = OnlineSessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(Delegate);
			OnlineSessionInterface->CancelFindSessions();
		}
	}

}

void SUTQuickMatchWindow::OnSearchCancelled(bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr OnlineSessionInterface;
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	if (OnlineSessionInterface.IsValid())
	{
		OnlineSessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(OnCancelFindSessionCompleteHandle);
		OnCancelFindSessionCompleteHandle.Reset();
	}

	PlayerOwner->CloseQuickMatch();
}

bool SUTQuickMatchWindow::SupportsKeyboardFocus() const
{
	return true;
}


FReply SUTQuickMatchWindow::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		return OnCancelClick();
	}

	return FReply::Unhandled();
}


FReply SUTQuickMatchWindow::OnCancelClick()
{
	Cancel();
	return FReply::Handled();
}

void SUTQuickMatchWindow::TellSlateIWantKeyboardFocus()

{
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EKeyboardFocusCause::Keyboard);
}

void SUTQuickMatchWindow::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	TArray<TWeakObjectPtr<AUTServerBeaconClient>> KillList;
	if (PingTrackers.Num() > 0)
	{
		float CurrentTime = PlayerOwner->GetWorld()->GetRealTimeSeconds();
		for (int32 i = 0; i < PingTrackers.Num(); i++)
		{
			if (CurrentTime - PingTrackers[i].PingStartTime > 5)
			{
				// This tracker is far outside what we want to consider.  Kill it...
				KillList.Add(PingTrackers[i].Beacon);
			}
		
		}

		if (KillList.Num() > 0)
		{
			for (int32 i=0; i<KillList.Num(); i++)
			{
				OnServerBeaconFailure(KillList[i].Get());
			}
		}
	}

	if (bWaitingForResponseFromHub)
	{
		HubResponseWaitTime+= InDeltaTime;
		if (HubResponseWaitTime > 15)		// Server has timed out or bad connect
		{
			RequestQuickPlayResults(NULL, EQuickMatchResults::JoinTimeout, TEXT(""));
		}
	}

}

void SUTQuickMatchWindow::AttemptQuickMatch(TSharedPtr<FServerSearchInfo> DesiredServer, TSharedPtr<FServerInstanceData> DesiredInstance)
{

	QMStats_NumAttemptedJoins++;

	bWaitingForResponseFromHub = true;
	HubResponseWaitTime = 0.0;

	ConnectingServer = DesiredServer;
	ConnectingInstance = DesiredInstance;

	AUTPlayerState* PlayerState = Cast<AUTPlayerState>(GetPlayerOwner()->PlayerController->PlayerState);
	bool bIsBeginner = PlayerState && PlayerState->IsABeginner(DefaultGameModeObject.Get()); 

	int32 PlayerRankCheck = PlayerState->GetRankCheck(DefaultGameModeObject.Get());

	UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(GetPlayerOwner()->GetWorld(), UPartyContext::StaticClass()));
	if (PartyContext)
	{
		const int32 PartySize = PartyContext->GetPartySize();
		if (PartySize > 1)
		{
			bIsBeginner = false;
			if (PlayerRankCheck < ((NUMBER_RANK_LEVELS * 2) - 1))
			{
				PlayerRankCheck = ((NUMBER_RANK_LEVELS * 2) - 1);
			}
		}
	}

	if (DesiredInstance.IsValid())
	{
		ConnectingServer->Beacon->OnRequestJoinInstanceResult = FServerRequestJoinInstanceResult::CreateSP(this, &SUTQuickMatchWindow::RequestJoinInstanceResult);
		ConnectingServer->Beacon->ServerRequestInstanceJoin(DesiredInstance->InstanceId.ToString(), false, PlayerRankCheck);
	}
	else
	{
		ConnectingServer->Beacon->OnRequestQuickplay = FServerRequestQuickplayDelegate::CreateSP(this, & SUTQuickMatchWindow::RequestQuickPlayResults);
		ConnectingServer->Beacon->ServerRequestQuickplay(QuickMatchType, PlayerRankCheck, bIsBeginner);
	}
}


void SUTQuickMatchWindow::RequestJoinInstanceResult(EInstanceJoinResult::Type Result, const FString& Params)
{
	ConnectingServer.Reset();
	ConnectingInstance.Reset();
	bWaitingForResponseFromHub = false;

	if (Result == EInstanceJoinResult::JoinDirectly)
	{
		AUTBasePlayerController* UTPlayerController = Cast<AUTBasePlayerController>(PlayerOwner->PlayerController);
		if (UTPlayerController)
		{
			QMStats_FinalResult = TEXT("DirectJoin");
			UTPlayerController->ConnectToServerViaGUID(Params,-1, false);
			return;
		}
	}

	// We failed to join, so find the next test
	FindBestMatch();		
}

void SUTQuickMatchWindow::RequestQuickPlayResults(AUTServerBeaconClient* Beacon, const FName& CommandCode, const FString& InstanceGuid)
{
	bWaitingForResponseFromHub = false;

	if (CommandCode == EQuickMatchResults::WaitingForStart || CommandCode == EQuickMatchResults::WaitingForStartNew )
	{
		UE_LOG(UT,Log,TEXT("Quickmatch instance is spooling up."));
		QMStats_NumInstancesSpooled++;
		bWaitingForMatch = true;
		if ( CommandCode == EQuickMatchResults::WaitingForStartNew )
		{
			FString ServerName;
			ConnectingServer->SearchResult.Session.SessionSettings.Get(SETTING_SERVERNAME,ServerName);

			PlayerOwner->QuickMatchLimitTime = PlayerOwner->GetWorld()->GetTimeSeconds() + 60.0;
			MinorStatusText = FText::Format( NSLOCTEXT("QuickMatch","Status_WaitingforMatchMinor","Hub '{0}' is starting a match for you to join."), FText::FromString(ServerName));
		}
	}
	else if (CommandCode == EQuickMatchResults::Join)
	{
		UE_LOG(UT,Log,TEXT("Quickplay connecting to hub"));
		AUTBasePlayerController* PC = Cast<AUTBasePlayerController>(GetPlayerOwner()->PlayerController);
		if (PC)
		{
			QMStats_FinalResult = TEXT("HubFoundInstance");
			PC->ConnectToServerViaGUID(InstanceGuid, -1, false);
		}
		else
		{
			UE_LOG(UT, Warning,TEXT("Quickmatch could not cast to BasePlayerController"));
		}
	}

}

//Special markup for Analytics event so they show up properly in grafana. Should be eventually moved to UTAnalytics.
/*  
* @EventName Quickmatch
*
* @Trigger Fires every time a quick match match happens or is canceled
*
* @Type Sent by the client
*
* @EventParam PlayerId
* @EventParam QuickmatchType

* @EventParam RankCheck

* @EventParam NumHubsConsidered
* @EventParam NumInstancesConsidered
* @EventParam NumRejectedForRank
* @EventParam NumRejectedForGameType
* @EventParam NumRejectedForJoinable
* @EventParam NumPingFailures
* @EventParam NumInstancesSpooled
* @EventParam NumAttemptedJoins
* @EventParam FinalResult

* @EventParam Duration
* @EventParam QuickMatch

* @Comments
*/
void SUTQuickMatchWindow::OnClosed()
{
	for (int32 i=0; i < FinalList.Num(); i++)
	{
		if (FinalList[i].IsValid() && FinalList[i]->Beacon.IsValid())
		{
			FinalList[i]->Beacon->DestroyBeacon();
		}
	}

	Instances.Empty();

	if (bCancelQuickmatch) QMStats_FinalResult = TEXT("UserCancel");

	if (FUTAnalytics::IsAvailable() && PlayerOwner.IsValid() && PlayerOwner->PlayerController && PlayerOwner->PlayerController->PlayerState)
	{
		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("PlayerId"), PlayerOwner->PlayerController->PlayerState->UniqueId.ToString()));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("QuickMatchType"), QuickMatchType));
		
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("RankCheck"), QMStats_RankCheck));
			
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumHubsConsidered"), QMStats_NumHubsConsidered));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumInstancesConsidered"), QMStats_NumInstancesConsidered));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumRejectedForRank"), QMStats_NumRejectedForRank));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumRejectedForPartySize"), QMStats_NumRejectedForPartySize));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumRejectedForGameType"), QMStats_NumRejectedForGameType));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumRejectedForJoinable"), QMStats_NumRejectedForJoinable));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumPingFailures"), QMStats_NumPingFailures));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumInstancesSpooled"), QMStats_NumInstancesSpooled));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("NumAttemptedJoins"), QMStats_NumAttemptedJoins));
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("FinalResult"), QMStats_FinalResult));

		float Duration = PlayerOwner->GetWorld()->GetRealTimeSeconds() - StartTime;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Duration"), Duration));

		FUTAnalytics::SetClientInitialParameters(Cast<AUTPlayerController>(PlayerOwner->PlayerController), ParamArray, false);

		FUTAnalytics::GetProvider().RecordEvent(TEXT("QuickMatch"), ParamArray);
	}

	SUTWindowBase::OnClosed();
}


#endif