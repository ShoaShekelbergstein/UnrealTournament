// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "GameFramework/PlayerState.h"
#include "UTLobbyGameMode.h"
#include "UTLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UTGameInstance.h"
#include "UTParty.h"
#include "UTPartyGameState.h"
#include "BlueprintContextLibrary.h"
#include "PartyContext.h"
#include "UTLobbyMatchInfo.h"

AUTLobbyPlayerState::AUTLobbyPlayerState(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AUTLobbyPlayerState::PreInitializeComponents()
{
}

void AUTLobbyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUTLobbyPlayerState, CurrentMatch);
	DOREPLIFETIME(AUTLobbyPlayerState, LastInvitedMatch);
	DOREPLIFETIME(AUTLobbyPlayerState, DesiredTeamNum);
}

void AUTLobbyPlayerState::MatchButtonPressed()
{
	if (CurrentMatch == NULL)
	{
		// TODO Check to make sure the rulesets are replicated

		bool bIsInParty = false;
		UPartyContext* PartyContext = Cast<UPartyContext>(UBlueprintContextLibrary::GetContext(GetWorld(), UPartyContext::StaticClass()));
		if (PartyContext)
		{
			bIsInParty = PartyContext->GetPartySize() > 1;
		}

		ServerCreateMatch(bIsInParty);
	}
	else
	{
		ServerDestroyOrLeaveMatch();
	}
}

bool AUTLobbyPlayerState::ServerCreateMatch_Validate(bool bIsInParty) { return true; }
void AUTLobbyPlayerState::ServerCreateMatch_Implementation(bool bIsInParty)
{
	if (CurrentMatch == NULL)
	{
		AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
		if (GameState)
		{
			if (GameState->AvailableGameRulesets.Num() >0)
			{
				GameState->AddNewMatch(this, nullptr, bIsInParty);
			}
			else
			{
				ClientMatchError(NSLOCTEXT("LobbyMessage","MatchRuleError","The server does not have any rulesets defined.  Please notify the server admin."));
			}
		}
	}
}

bool AUTLobbyPlayerState::ServerDestroyOrLeaveMatch_Validate() { return true; }
void AUTLobbyPlayerState::ServerDestroyOrLeaveMatch_Implementation()
{
	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	if (GameState)
	{
		GameState->RemoveFromAMatch(this);
	}
}

bool AUTLobbyPlayerState::ServerJoinMatch_Validate(AUTLobbyMatchInfo* MatchToJoin, bool bAsSpectator) { return true; }
void AUTLobbyPlayerState::ServerJoinMatch_Implementation(AUTLobbyMatchInfo* MatchToJoin, bool bAsSpectator)
{
	// CurrentMatch may have been set by a previous call if client is lagged and sends multiple requests
	if (CurrentMatch != MatchToJoin)
	{
		AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
		if (GameState != NULL)
		{
			if (CurrentMatch != NULL)
			{
				GameState->RemoveFromAMatch(this);
			}
			GameState->JoinMatch(MatchToJoin, this, bAsSpectator);
		}
	}
}

void AUTLobbyPlayerState::AddedToMatch(AUTLobbyMatchInfo* Match)
{
	CurrentMatch = Match;
	bIsInMatch = true;
}

void AUTLobbyPlayerState::RemovedFromMatch(AUTLobbyMatchInfo* Match)
{
	CurrentMatch = NULL;
	bIsInMatch = false;
}

void AUTLobbyPlayerState::ClientMatchError_Implementation(const FText &MatchErrorMessage, int32 OptionalInt)
{
	AUTBasePlayerController* BasePC = Cast<AUTBasePlayerController>(GetOwner());
	if (BasePC)
	{
#if !UE_SERVER
		BasePC->ShowMessage(NSLOCTEXT("LobbyMessage","MatchMessage","Match Message"), FText::Format(MatchErrorMessage, FText::AsNumber(OptionalInt)), UTDIALOG_BUTTON_OK, NULL);	
#endif
	}
}

void AUTLobbyPlayerState::ClientConnectToInstance_Implementation(const FString& GameInstanceGUIDString, int32 InDesiredTeam, bool bAsSpectator)
{
	AUTBasePlayerController* BPC = Cast<AUTBasePlayerController>(GetOwner());
	if (BPC)
	{
		BPC->ConnectToServerViaGUID(GameInstanceGUIDString, InDesiredTeam, bAsSpectator);
	}
}

void AUTLobbyPlayerState::OnRep_CurrentMatch()
{
	bIsInMatch = CurrentMatch != NULL;
	AUTLobbyPC* PC = Cast<AUTLobbyPC>(GetOwner());
	if (PC)
	{
		PC->MatchChanged(CurrentMatch);
	}

#if !UE_SERVER
	if (CurrentMatchChangedDelegate.IsBound())
	{
		CurrentMatchChangedDelegate.Execute(this);
	}

#endif
}

bool AUTLobbyPlayerState::Server_ReadyToBeginDataPush_Validate() { return true; }
void AUTLobbyPlayerState::Server_ReadyToBeginDataPush_Implementation()
{
	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	if (GameState)
	{
		//UE_LOG(UT,Log,TEXT("[DataPush] - Telling Client to start %i"),GameState->AllowedGameData.Num());
		Client_BeginDataPush(GameState->AllowedGameData.Num());
	}	
}

void AUTLobbyPlayerState::Client_BeginDataPush_Implementation(int32 ExpectedSendCount)
{
	TotalBlockCount = ExpectedSendCount;

	//UE_LOG(UT,Log,TEXT("[DataPush] - Begin with an expected send count of %i"), TotalBlockCount);

	// Check to see if the client is ready for a data push.
	CheckDataPushReady();
}

void AUTLobbyPlayerState::CheckDataPushReady()
{
	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	if (GameState)
	{
		// We are ready.  Setup the 
		//UE_LOG(UT,Log,TEXT("[DataPush] - Requesting Block 0"));
		CurrentBlock = 0;
		Server_SendDataBlock(CurrentBlock);
	}
	else
	{
		//UE_LOG(UT,Log,TEXT("[DataPush] - GameState not yet replicated.  Delaying for 1/2 second"));

		// We don't have the GameState replicated yet.  Try again in 1/2 a second
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTLobbyPlayerState::CheckDataPushReady, 0.5f, false);
	}
}

bool AUTLobbyPlayerState::Server_SendDataBlock_Validate(int32 Block) { return true; }
void AUTLobbyPlayerState::Server_SendDataBlock_Implementation(int32 Block)
{
	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	//UE_LOG(UT,Log,TEXT("[DataPush] - Sending Block %i (%s)"), Block, GameState ? TEXT("Good To Send") : TEXT("Can't Send"));
	if (GameState && (Block >= 0) && (Block < GameState->AllowedGameData.Num()))
	{
		Client_ReceiveBlock(Block, GameState->AllowedGameData[Block]);
	}
}

void AUTLobbyPlayerState::Client_ReceiveBlock_Implementation(int32 Block, FAllowedData Data)
{

	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	//UE_LOG(UT,Log,TEXT("[DataPush] - Received Block %i (%s)"), Block, GameState ? TEXT("Good To Save") : TEXT("Can't Save"));
	if (GameState)
	{
		GameState->ClientAssignGameData(Data);
		Block++;
		if (Block < TotalBlockCount)
		{
			//UE_LOG(UT,Log,TEXT("[DataPush] - Requesting Block %i"), Block);
			Server_SendDataBlock(Block);
		}
		else
		{
			//UE_LOG(UT,Log,TEXT("[DataPush] - Completed %i vs %i"), Block, TotalBlockCount);
			GameState->bGameDataReplicationCompleted = true;
		}
	}	
}

void AUTLobbyPlayerState::InviteToMatch(AUTLobbyMatchInfo* Match)
{
	if (Match) 
	{
		LastInvitedMatch = Match;
	}
}

void AUTLobbyPlayerState::UninviteFromMatch(AUTLobbyMatchInfo* Match)
{
	if (Match) 
	{
		if (LastInvitedMatch == Match)
		{
			LastInvitedMatch = NULL;
		}
	}
}


void AUTLobbyPlayerState::NotifyBeginnerAutoLock_Implementation()
{
	AUTBasePlayerController* PC = Cast<AUTBasePlayerController>(GetOwner());
	if (PC)
	{
		UUTLocalPlayer* UTLocalPlayer = Cast<UUTLocalPlayer>(PC->Player);
		if (UTLocalPlayer && !UTLocalPlayer->bAutoRankLockWarningShown)
		{
			UTLocalPlayer->bAutoRankLockWarningShown = true;
#if !UE_SERVER
			UTLocalPlayer->ShowMessage(NSLOCTEXT("UTLobbyPlayerState","AutoLockTitle","AUTO RANK LOCKED ENABLED"),NSLOCTEXT("UTLobbyPlayerState","AutoLockMessage","Because you are a beginner, the match you are starting will be locked so that only other beginners of similar skill can join.  If you want to allow players of other skill levels to play, uncheck the \"Limit Rank\" check box in your matches settings."),UTDIALOG_BUTTON_OK);
#endif
		}
	}

}

void AUTLobbyPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AUTLobbyGameState* GameState = GetWorld()->GetGameState<AUTLobbyGameState>();
	if (GameState)
	{
		UUTGameInstance* UTGameInstance = Cast<UUTGameInstance>(GetGameInstance());
		if (UTGameInstance)
		{
			UUTParty* Party = UTGameInstance->GetParties();
			if (Party)
			{
				UUTPartyGameState* PartyGameState = Cast<UUTPartyGameState>(Party->GetUTPersistentParty());
				if (PartyGameState && !PartyGameState->IsLocalPartyLeader())
				{
					TSharedPtr<const FUniqueNetId> PartyLeaderId = PartyGameState->GetPartyLeader();
					for (int32 i = 0; i < GameState->AvailableMatches.Num(); i++)
					{
						if (GameState->AvailableMatches[i])
						{
							for (int32 PlayerIdx = 0; PlayerIdx < GameState->AvailableMatches[i]->Players.Num(); PlayerIdx++)
							{
								if (GameState->AvailableMatches[i]->Players[PlayerIdx] != nullptr && PartyLeaderId.IsValid())
								{
									if (GameState->AvailableMatches[i]->Players[PlayerIdx]->StatsID == PartyLeaderId->ToString())
									{
										if (CurrentMatch != GameState->AvailableMatches[i] && JoiningLeaderMatch != GameState->AvailableMatches[i])
										{
											JoiningLeaderMatch = GameState->AvailableMatches[i];
											ServerJoinMatch(GameState->AvailableMatches[i], false);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

}
