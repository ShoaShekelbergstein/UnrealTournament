// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Slate/SlateGameResources.h"
#include "../Base/SUTPanelBase.h"
#include "../SUWindowsStyle.h"
#include "SUTInGameHomePanel.h"
#include "SUTLobbyMatchSetupPanel.h"
#include "SUTPlayerListPanel.h"
#include "SUTTextChatPanel.h"
#include "SUTMatchPanel.h"
#include "UTLobbyMatchInfo.h"

#if !UE_SERVER

class SUTStartMatchWindow;

class UNREALTOURNAMENT_API SUTLobbyInfoPanel : public SUTPanelBase 
{
public:
	virtual void ConstructPanel(FVector2D ViewportSize);

protected:
	TSharedPtr<SOverlay> MainOverlay;
	TSharedPtr<SVerticalBox> LeftPanel;
	TSharedPtr<SVerticalBox> RightPanel;

	// TODO: Add a SP for the Chat panel when it's written.
	TSharedPtr<SUTMatchPanel> MatchBrowser;
	TSharedPtr<SUTLobbyMatchSetupPanel> MatchSetup;
	TSharedPtr<SUTPlayerListPanel> PlayerListPanel;
	TSharedPtr<SUTTextChatPanel> TextChatPanel;


protected:

	// Will be true if we are showing the match dock.  It will be set to false when the owner enters a match 
	bool bShowingMatchBrowser;

	// Holds the last match state if the player owner has a current match.  It's used to detect state changes and rebuild the menu as needed.
	FName LastMatchState;

	/**
	 *	Builds the chat window and player list
	 **/
	virtual void BuildChatAndPlayerList();

	void ShowMatchSetupPanel();

	/**
	 *	Builds the Match Browser
	 **/
	void ShowMatchBrowser();

	virtual void ChatDestionationChanged(FName NewDestination);
	virtual void RulesetChanged();
	virtual void PlayerClicked(FUniqueNetIdRepl PlayerId);

	virtual void OnShowPanel(TSharedPtr<SUTMenuBase> inParentWindow);
	virtual void OnHidePanel();

	TSharedPtr<SUTGameSetupDialog> SetupDialog;
	TSharedPtr<SUTStartMatchWindow> StartMatchWindow;

public:
	void ApplySetup(TWeakObjectPtr<AUTLobbyMatchInfo> MatchInfo);
	void StartMatch();
	void OnGameChangeDialogResult(TSharedPtr<SCompoundWidget> Dialog, uint16 ButtonPressed);
	void CancelInstance();

};

#endif