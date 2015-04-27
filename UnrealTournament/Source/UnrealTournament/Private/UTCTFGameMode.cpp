// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTTeamGameMode.h"
#include "UTHUD_CTF.h"
#include "UTCTFGameMode.h"
#include "UTCTFGameMessage.h"
#include "UTCTFRewardMessage.h"
#include "UTFirstBloodMessage.h"
#include "UTPickup.h"
#include "UTGameMessage.h"
#include "UTMutator.h"
#include "UTCTFSquadAI.h"
#include "UTWorldSettings.h"

namespace MatchState
{
	const FName MatchEnteringHalftime = FName(TEXT("MatchEnteringHalftime"));
	const FName MatchIsAtHalftime = FName(TEXT("MatchIsAtHalftime"));
	const FName MatchExitingHalftime = FName(TEXT("MatchExitingHalftime"));
	const FName MatchEnteringSuddenDeath = FName(TEXT("MatchEnteringSuddenDeath"));
	const FName MatchIsInSuddenDeath = FName(TEXT("MatchIsInSuddenDeath"));
}


AUTCTFGameMode::AUTCTFGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// By default, we do 2 team CTF
	NumTeams = 2;
	HalftimeDuration = 30.f;
	HUDClass = AUTHUD_CTF::StaticClass();
	GameStateClass = AUTCTFGameState::StaticClass();
	bAllowOvertime = true;
	OvertimeDuration = 5;
	bUseTeamStarts = true;
	bSuddenDeath = true;
	MercyScore = 5;
	GoalScore = 0;
	TimeLimit = 14;
	MapPrefix = TEXT("CTF");
	SquadType = AUTCTFSquadAI::StaticClass();
	CTFScoringClass = AUTCTFScoring::StaticClass();

	//Add the translocator here for now :(
	static ConstructorHelpers::FObjectFinder<UClass> WeapTranslocator(TEXT("BlueprintGeneratedClass'/Game/RestrictedAssets/Weapons/Translocator/BP_Translocator.BP_Translocator_C'"));
	DefaultInventory.Add(WeapTranslocator.Object);

	DisplayName = NSLOCTEXT("UTGameMode", "CTF", "Capture the Flag");
}

void AUTCTFGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bSuddenDeath = EvalBoolOptions(ParseOption(Options, TEXT("SuddenDeath")), bSuddenDeath);

	// HalftimeDuration is in seconds and used in seconds,
	HalftimeDuration = FMath::Max(0, GetIntOption(Options, TEXT("HalftimeDuration"), HalftimeDuration));

	// OvertimeDuration is in minutes
	OvertimeDuration = FMath::Max(1, GetIntOption(Options, TEXT("OvertimeDuration"), OvertimeDuration));
	OvertimeDuration *= 60;

	if (TimeLimit > 0)
	{
		TimeLimit = uint32(float(TimeLimit) * 0.5);
	}
}

void AUTCTFGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	CTFScoring = GetWorld()->SpawnActor<AUTCTFScoring>(CTFScoringClass, SpawnInfo);
	CTFScoring->CTFGameState = CTFGameState;
}

void AUTCTFGameMode::InitGameState()
{
	Super::InitGameState();
	// Store a cached reference to the GameState
	CTFGameState = Cast<AUTCTFGameState>(GameState);
	CTFGameState->SetMaxNumberOfTeams(NumTeams);
}

void AUTCTFGameMode::ScoreObject(AUTCarriedObject* GameObject, AUTCharacter* HolderPawn, AUTPlayerState* Holder, FName Reason)
{
	if (Holder != NULL && Holder->Team != NULL && !CTFGameState->HasMatchEnded() && !CTFGameState->IsMatchAtHalftime() && GetMatchState() != MatchState::MatchEnteringHalftime)
	{
		CTFScoring->ScoreObject(GameObject, HolderPawn, Holder, Reason, TimeLimit);

		if (BaseMutator != NULL)
		{
			BaseMutator->ScoreObject(GameObject, HolderPawn, Holder, Reason);
		}

		if ( Reason == FName("SentHome") )
		{
			// if all flags are returned, end advantage time right away
			if (CTFGameState->bPlayingAdvantage)
			{
				bool bAllHome = true;
				for (AUTCTFFlagBase* Base : CTFGameState->FlagBases)
				{
					if (Base != NULL && Base->GetCarriedObjectState() != CarriedObjectState::Home)
					{
						bAllHome = false;
						break;
					}
				}
				if (bAllHome)
				{
					EndOfHalf();
				}
			}
		}
		else if (Reason == FName("FlagCapture"))
		{
			// Give the team a capture.
			Holder->Team->Score++;
			Holder->Team->ForceNetUpdate();
			BroadcastScoreUpdate(Holder, Holder->Team);

			for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				(*Iterator)->ClientPlaySound(CTFGameState->FlagBases[Holder->Team->TeamIndex]->FlagScoreRewardSound, 2.f);
				AUTPlayerState* PS = Cast<AUTPlayerState>((*Iterator)->PlayerState);
				if (PS && PS->bNeedsAssistAnnouncement)
				{
					(*Iterator)->ClientReceiveLocalizedMessage(UUTCTFRewardMessage::StaticClass(), 2, PS, NULL, NULL);
					PS->bNeedsAssistAnnouncement = false;
				}
			}

			if (CTFGameState->IsMatchInOvertime())
			{
				EndGame(Holder, FName(TEXT("GoldenCap")));	
			}
			else
			{
				CheckScore(Holder);
				// any cap ends advantage time immediately
				// warning: CheckScore() could have ended the match already
				if (CTFGameState->bPlayingAdvantage && !CTFGameState->HasMatchEnded())
				{
					EndOfHalf();
				}
			}
		}
	}
}

bool AUTCTFGameMode::CheckScore(AUTPlayerState* Scorer)
{
	if (Scorer->Team != NULL)
	{
		if (GoalScore > 0 && Scorer->Team->Score >= GoalScore)
		{
			EndGame(Scorer, FName(TEXT("scorelimit")));
		}
		else if (MercyScore > 0)
		{
			int32 Spread = Scorer->Team->Score;
			for (AUTTeamInfo* OtherTeam : Teams)
			{
				if (OtherTeam != Scorer->Team)
				{
					Spread = FMath::Min<int32>(Spread, Scorer->Team->Score - OtherTeam->Score);
				}
			}
			if (Spread >= MercyScore)
			{
				EndGame(Scorer, FName(TEXT("MercyScore")));
			}
		}
	}

	return true;
}

void AUTCTFGameMode::CheckGameTime()
{
	if (HalftimeDuration <= 0 || TimeLimit == 0)
	{
		Super::CheckGameTime();
	}
	else if ( CTFGameState->IsMatchInProgress() )
	{
		// First look to see if we are in halftime. 
		if (CTFGameState->IsMatchAtHalftime())		
		{
			if (CTFGameState->RemainingTime <= 0)
			{
				SetMatchState(MatchState::MatchExitingHalftime);
			}
		}

		// If the match is in progress and we are not playing advantage, then enter the halftime/end of game logic depending on the half
		else if (CTFGameState->RemainingTime <= 0)
		{
			if (!CTFGameState->bPlayingAdvantage)
			{
				// If we are in Overtime - Keep battling until one team wins.  We might want to add half-time or bring sudden death back 
				if ( CTFGameState->IsMatchInOvertime() )
				{
					AUTTeamInfo* WinningTeam = CTFGameState->FindLeadingTeam();
					if ( WinningTeam != NULL )
					{
						// Match is over....
						AUTPlayerState* WinningPS = FindBestPlayerOnTeam(WinningTeam->GetTeamNum());
						EndGame(WinningPS, FName(TEXT("TimeLimit")));	
					}
				}
				// We are in normal time, so figure out what to do...
				else
				{
					// Look to see if we should be playing advantage
					if (!CTFGameState->bPlayingAdvantage)
					{
						int32 AdvantageTeam = TeamWithAdvantage();
						if (AdvantageTeam >= 0 && AdvantageTeam <= 1)
						{
							// A team has advantage.. so set the flags.
							CTFGameState->bPlayingAdvantage = true;
							CTFGameState->AdvantageTeamIndex = AdvantageTeam;
							CTFGameState->SetTimeLimit(60);

							// Broadcast the Advantage Message....
							BroadcastLocalized(this, UUTCTFGameMessage::StaticClass(), 6, NULL, NULL, CTFGameState->Teams[AdvantageTeam]);

							return;
						}
					}

					// If we still aren't playing advantage, handle halftime, etc.
					if (!CTFGameState->bPlayingAdvantage)
					{
						EndOfHalf();
					}
				}
			}
			// Advantage Time ran out
			else
			{
				EndOfHalf();
			}
		}
		// If we are playing advantage, we need to check to see if we should be playing advantage
		else if (CTFGameState->bPlayingAdvantage)
		{
			// Look to see if we should still be playing advantage
			if (!CheckAdvantage())
			{
				EndOfHalf();
			}
		}
	}
}

void AUTCTFGameMode::EndOfHalf()
{
	// Handle possible end of game at the end of the second half
	if (CTFGameState->bSecondHalf)
	{
		// Look to see if there is a winning team
		AUTTeamInfo* WinningTeam = CTFGameState->FindLeadingTeam();
		if (WinningTeam != NULL)
		{
			// Game Over... yeh!
			AUTPlayerState* WinningPS = FindBestPlayerOnTeam(WinningTeam->GetTeamNum());
			EndGame(WinningPS, FName(TEXT("TimeLimit")));
		}

		// Otherwise look to see if we should enter overtime
		else
		{
			if (bAllowOvertime)
			{
				SetMatchState(MatchState::MatchIsInOvertime);
			}
			else
			{
				// Match is over....
				EndGame(NULL, FName(TEXT("TimeLimit")));
			}
		}
	}

	// Time expired and noone has advantage so enter the second half.
	else
	{
		SetMatchState(MatchState::MatchEnteringHalftime);
	}
}

void AUTCTFGameMode::HandleMatchHasStarted()
{
	if (!CTFGameState->bSecondHalf)
	{
		Super::HandleMatchHasStarted();
	}
}

void AUTCTFGameMode::HandleHalftime()
{
}

void AUTCTFGameMode::PlacePlayersAroundFlagBase(int32 TeamNum)
{
	TArray<AController*> Members = Teams[TeamNum]->GetTeamMembers();
	const int MaxPlayers = FMath::Min(8, Members.Num());

	FVector FlagLoc = CTFGameState->FlagBases[TeamNum]->GetActorLocation();
	float AngleSlices = 360.0f / MaxPlayers;
	int32 PlacementCounter = 0;
	for (AController* C : Members)
	{
		AUTCharacter* UTChar = Cast<AUTCharacter>(C->GetPawn());
		if (UTChar && !UTChar->IsDead())
		{
			while (PlacementCounter < MaxPlayers)
			{
				FRotator AdjustmentAngle(0, AngleSlices * PlacementCounter, 0);

				PlacementCounter++;

				FVector PlacementLoc = FlagLoc + AdjustmentAngle.RotateVector(FVector(200, 0, 0));
				PlacementLoc.Z += UTChar->GetSimpleCollisionHalfHeight() * 1.1f;

				FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(UTChar->GetSimpleCollisionRadius(), UTChar->GetSimpleCollisionHalfHeight());
				static const FName NAME_FlagPlacement = FName(TEXT("FlagPlacement"));
				FCollisionQueryParams CapsuleParams(NAME_FlagPlacement, false, this);
				FCollisionResponseParams ResponseParam;
				TArray<FOverlapResult> Overlaps;
				bool bEncroached = GetWorld()->OverlapMulti(Overlaps, PlacementLoc, FQuat::Identity, ECC_Pawn, CapsuleShape, CapsuleParams, ResponseParam);
				if (!bEncroached)
				{
					UTChar->SetActorLocation(PlacementLoc);
					break;
				}
			}
		}

		if (PlacementCounter == 8)
		{
			break;
		}
	}
}

void AUTCTFGameMode::HandleEnteringHalftime()
{
	CTFGameState->ResetFlags();

	// Figure out who we should look at
	// Init targets
	TArray<AUTCharacter*> BestPlayers;
	int32 BestTeam = 0;
	int32 BestTeamScore = 0;
	for (int i=0;i<Teams.Num();i++)
	{
		BestPlayers.Add(NULL);
		PlacePlayersAroundFlagBase(i);

		if (i == BestTeam || Teams[i]->Score > BestTeamScore)
		{
			BestTeamScore = Teams[i]->Score;
			BestTeam = i;
		}
	}
	
	for( FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator )
	{
		AController* Controller = *Iterator;
		if (Controller->GetPawn() != NULL && Controller->PlayerState != NULL)
		{
			AUTCharacter* Char = Cast<AUTCharacter>(Controller->GetPawn());
			if (Char != NULL)
			{
				AUTPlayerState* PS = Cast<AUTPlayerState>(Controller->PlayerState);
				uint8 TeamNum = PS->GetTeamNum();
				if (TeamNum < BestPlayers.Num())
				{
					if (BestPlayers[TeamNum] == NULL || PS->Score > BestPlayers[TeamNum]->PlayerState->Score || Cast<AUTCTFFlag>(PS->CarriedObject) != NULL)
					{
						BestPlayers[TeamNum] = Char;
					}
				}
			}
		}
	}	

	// Tell the controllers to look at own team flag
	for( FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator )
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
		if (PC != NULL)
		{
			PC->ClientHalftime();
			if (!PC->PlayerState->bOnlySpectator)
			{
				int32 TeamToWatch = (PC->GetTeamNum() < Teams.Num()) ? PC->GetTeamNum() : BestTeam;
				PC->SetViewTarget(CTFGameState->FlagBases[TeamToWatch]);
			}
		}
	}
	
	// Freeze all of the pawns
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		if (*It)
		{
			(*It)->TurnOff();
		}
	}

	CTFGameState->bHalftime = true;
	CTFGameState->OnHalftimeChanged();
	CTFGameState->bPlayingAdvantage = false;
	CTFGameState->AdvantageTeamIndex = 255;
	CTFGameState->SetTimeLimit(HalftimeDuration);	// Reset the Game Clock for Halftime

	SetMatchState(MatchState::MatchIsAtHalftime);
	BroadcastLocalized(this, UUTCTFGameMessage::StaticClass(), 11, NULL, NULL, NULL);
}

void AUTCTFGameMode::HalftimeIsOver()
{
	SetMatchState(MatchState::MatchExitingHalftime);
}

void AUTCTFGameMode::HandleExitingHalftime()
{
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		// Detach all controllers from their pawns
		if ((*Iterator)->GetPawn() != NULL)
		{
			(*Iterator)->UnPossess();
		}
	}

	TArray<APawn*> PawnsToDestroy;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		if (*It)
		{
			PawnsToDestroy.Add(*It);
		}
	}

	for (int i=0;i<PawnsToDestroy.Num();i++)
	{
		APawn* Pawn = PawnsToDestroy[i];
		if (Pawn != NULL && !Pawn->IsPendingKill())
		{
			Pawn->Destroy();	
		}
	}

	// swap sides, if desired
	AUTWorldSettings* Settings = Cast<AUTWorldSettings>(GetWorld()->GetWorldSettings());
	if (Settings != NULL && Settings->bAllowSideSwitching)
	{
		CTFGameState->ChangeTeamSides(1);
	}

	// reset everything
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		if (It->GetClass()->ImplementsInterface(UUTResetInterface::StaticClass()))
		{
			IUTResetInterface::Execute_Reset(*It);
		}
	}

	//now respawn all the players
	for( FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator )
	{
		AController* Controller = *Iterator;
		if (Controller->PlayerState != NULL && !Controller->PlayerState->bOnlySpectator)
		{
			RestartPlayer(Controller);
		}
	}

	// Send all flags home..
	CTFGameState->ResetFlags();
	CTFGameState->bHalftime = false;
	CTFGameState->OnHalftimeChanged();
	if (CTFGameState->bSecondHalf)
	{
		SetMatchState(MatchState::MatchEnteringOvertime);
	}
	else
	{
		CTFGameState->bSecondHalf = true;
		CTFGameState->SetTimeLimit(TimeLimit);		// Reset the GameClock for the second time.
		SetMatchState(MatchState::InProgress);
	}
}

void AUTCTFGameMode::GameObjectiveInitialized(AUTGameObjective* Obj)
{
	AUTCTFFlagBase* FlagBase = Cast<AUTCTFFlagBase>(Obj);
	if (FlagBase != NULL && FlagBase->MyFlag)
	{
		CTFGameState->CacheFlagBase(FlagBase);
	}
}

bool AUTCTFGameMode::PlayerCanRestart( APlayerController* Player )
{
	// Can't restart in overtime
	if (!CTFGameState->IsMatchInProgress() || CTFGameState->IsMatchAtHalftime() || CTFGameState->IsMatchInSuddenDeath()|| 
			Player == NULL || Player->IsPendingKillPending())
	{
		return false;
	}
	
	// Ask the player controller if it's ready to restart as well
	return Player->CanRestartPlayer();
}

void AUTCTFGameMode::ScoreDamage(int32 DamageAmount, AController* Victim, AController* Attacker)
{
	Super::ScoreDamage(DamageAmount, Victim, Attacker);
	CTFScoring->ScoreDamage(DamageAmount, Victim, Attacker);
}

void AUTCTFGameMode::ScoreKill(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	CTFScoring->ScoreKill(Killer, Other, KilledPawn, DamageType);
	if ((Killer != NULL && Killer != Other))
	{
		AUTPlayerState* AttackerPS = Cast<AUTPlayerState>(Killer->PlayerState);
		if (AttackerPS != NULL)
		{
			if (!bFirstBloodOccurred)
			{
				BroadcastLocalized(this, UUTFirstBloodMessage::StaticClass(), 0, AttackerPS, NULL, NULL);
				bFirstBloodOccurred = true;
			}
			AttackerPS->IncrementKills(DamageType, true);
			FindAndMarkHighScorer();
		}
	}
	if (BaseMutator != NULL)
	{
		BaseMutator->ScoreKill(Killer, Other, DamageType);
	}
}

bool AUTCTFGameMode::IsMatchInSuddenDeath()
{
	return CTFGameState->IsMatchInSuddenDeath();
}

void AUTCTFGameMode::CallMatchStateChangeNotify()
{
	Super::CallMatchStateChangeNotify();

	if (MatchState == MatchState::MatchEnteringHalftime)
	{
		HandleEnteringHalftime();
	}
	else if (MatchState == MatchState::MatchIsAtHalftime)
	{
		HandleHalftime();
	}
	else if (MatchState == MatchState::MatchExitingHalftime)
	{
		HandleExitingHalftime();
	}
	else if (MatchState == MatchState::MatchEnteringSuddenDeath)
	{
		HandleEnteringSuddenDeath();
	}
	else if (MatchState == MatchState::MatchIsInSuddenDeath)
	{
		HandleSuddenDeath();
	}
}

void AUTCTFGameMode::HandleEnteringOvertime()
{
	CTFGameState->SetTimeLimit(OvertimeDuration);
	SetMatchState(MatchState::MatchIsInOvertime);
	CTFGameState->bPlayingAdvantage = false;
}

void AUTCTFGameMode::HandleEnteringSuddenDeath()
{
	BroadcastLocalized(this, UUTCTFGameMessage::StaticClass(), 12, NULL, NULL, NULL);
	CTFGameState->bPlayingAdvantage = false;
}

void AUTCTFGameMode::HandleSuddenDeath()
{
	CTFGameState->bPlayingAdvantage = false;
}

void AUTCTFGameMode::HandleMatchInOvertime()
{
	BroadcastLocalized(this, UUTCTFGameMessage::StaticClass(), 12, NULL, NULL, NULL);
	CTFGameState->bPlayingAdvantage = false;
}

uint8 AUTCTFGameMode::TeamWithAdvantage()
{
	AUTCTFFlag* Flags[2];
	if (CTFGameState == NULL || CTFGameState->FlagBases.Num() < 2 || CTFGameState->FlagBases[0] == NULL || CTFGameState->FlagBases[1] == NULL)
	{
		return false;	// Fix for crash when CTF transitions to a map without flags.
	}

	Flags[0] = CTFGameState->FlagBases[0]->MyFlag;
	Flags[1] = CTFGameState->FlagBases[1]->MyFlag;

	if ( (Flags[0]->ObjectState == Flags[1]->ObjectState) ||
		 (Flags[0]->ObjectState != CarriedObjectState::Held && Flags[1]->ObjectState != CarriedObjectState::Held))
	{
		// Both flags share the same state so noone has advantage
		return 255;
	}

	int8 AdvantageNum = Flags[0]->ObjectState == CarriedObjectState::Held ? 1 : 0;

	return AdvantageNum;
}

bool AUTCTFGameMode::CheckAdvantage()
{
	if (!CTFGameState || !CTFGameState->FlagBases[0] || !CTFGameState->FlagBases[1])
	{
		return false;
	}
	AUTCTFFlag* Flags[2];
	Flags[0] = CTFGameState->FlagBases[0]->MyFlag;
	Flags[1] = CTFGameState->FlagBases[1]->MyFlag;

	uint8 OtherTeam = 1 - CTFGameState->AdvantageTeamIndex;

	// The Flag was returned so advantage lost
	if (!Flags[0] || !Flags[1] || Flags[OtherTeam]->ObjectState == CarriedObjectState::Home)
	{
		return false;	
	}

	// If our flag is held, then look to see if our advantage is lost.. has to be held for 5 seconds.
	if (Flags[CTFGameState->AdvantageTeamIndex]->ObjectState == CarriedObjectState::Held)
	{
		AdvantageGraceTime++;
		if (AdvantageGraceTime >= 5)		// 5 second grace period.. make this config :)
		{
			return false;
		}
		else
		{
			BroadcastLocalized(this, UUTCTFGameMessage::StaticClass(), 7, NULL, NULL, CTFGameState->Teams[CTFGameState->AdvantageTeamIndex]);
		}
	}
	else
	{
		AdvantageGraceTime = 0;
	}
		
	return true;
}

void AUTCTFGameMode::BuildServerResponseRules(FString& OutRules)
{
	OutRules += FString::Printf(TEXT("Goal Score\t%i\t"), GoalScore);
	OutRules += FString::Printf(TEXT("Time Limit\t%i\t"), TimeLimit);
	OutRules += FString::Printf(TEXT("Forced Respawn\t%s\t"), bForceRespawn ?  TEXT("True") : TEXT("False"));

	if (TimeLimit > 0)
	{
		if (HalftimeDuration <= 0)
		{
			OutRules += FString::Printf(TEXT("No Halftime\tTrue\t"));
		}
		else
		{
			OutRules += FString::Printf(TEXT("Halftime\tTrue\t"));
			OutRules += FString::Printf(TEXT("Halftime Duration\t%i\t"), HalftimeDuration);
		}
	}

	OutRules += FString::Printf(TEXT("Allow Overtime\t%s\t"), bAllowOvertime ? TEXT("True") : TEXT("False"));
	if (bAllowOvertime)
	{
		OutRules += FString::Printf(TEXT("Overtime Duration\t%i\t"), OvertimeDuration);;
	}

	if (bSuddenDeath)
	{
		OutRules += FString::Printf(TEXT("Sudden Death\tTrue\t"));
	}

	AUTMutator* Mut = BaseMutator;
	while (Mut)
	{
		OutRules += FString::Printf(TEXT("Mutator\t%s\t"), *Mut->DisplayName.ToString());
		Mut = Mut->NextMutator;
	}
}
void AUTCTFGameMode::EndGame(AUTPlayerState* Winner, FName Reason )
{
	// Dont ever end the game in PIE
	if (GetWorld()->WorldType == EWorldType::PIE) return;

	// Send all of the flags home...
	CTFGameState->ResetFlags();
	Super::EndGame(Winner, Reason);
}


void AUTCTFGameMode::SetEndGameFocus(AUTPlayerState* Winner)
{
	AUTCTFFlagBase* WinningBase = NULL;
	if (Winner != NULL)
	{
		WinningBase = CTFGameState->FlagBases[Winner->GetTeamNum()];

		PlacePlayersAroundFlagBase(Winner->GetTeamNum());
	}

	for( FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator )
	{
		AUTPlayerController* Controller = Cast<AUTPlayerController>(*Iterator);
		if (Controller && Controller->UTPlayerState)
		{
			AUTCTFFlagBase* BaseToView = WinningBase;
			// If we don't have a winner, view my base
			if (BaseToView == NULL)
			{
				AUTTeamInfo* MyTeam = Controller->UTPlayerState->Team;
				if (MyTeam)
				{
					BaseToView = CTFGameState->FlagBases[MyTeam->GetTeamNum()];
				}
			}
		
			if (BaseToView)
			{
				Controller->GameHasEnded(BaseToView , Controller->UTPlayerState->Team == Winner->Team );
			}
		}
	}
}

void AUTCTFGameMode::UpdateSkillRating()
{
	for (int32 PlayerIdx = 0; PlayerIdx < UTGameState->PlayerArray.Num(); PlayerIdx++)
	{
		AUTPlayerState* PS = Cast<AUTPlayerState>(UTGameState->PlayerArray[PlayerIdx]);
		if (PS && !PS->bOnlySpectator)
		{
			PS->UpdateTeamSkillRating(FName(TEXT("CTFSkillRating")), PS->Team == UTGameState->WinningTeam, &UTGameState->PlayerArray, &InactivePlayerArray);
		}
	}

	for (int32 PlayerIdx = 0; PlayerIdx < InactivePlayerArray.Num(); PlayerIdx++)
	{
		AUTPlayerState* PS = Cast<AUTPlayerState>(InactivePlayerArray[PlayerIdx]);
		if (PS && !PS->bOnlySpectator)
		{
			PS->UpdateTeamSkillRating(FName(TEXT("CTFSkillRating")), PS->Team == UTGameState->WinningTeam, &UTGameState->PlayerArray, &InactivePlayerArray);
		}
	}
}

#if !UE_SERVER
void AUTCTFGameMode::BuildPlayerInfo(TSharedPtr<SVerticalBox> Panel, AUTPlayerState* PlayerState)
{
	Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
	[
		NewPlayerInfoLine(FString("Captures"), FString::Printf(TEXT("%i"), PlayerState->FlagCaptures))
	];
	Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
	[
		NewPlayerInfoLine(FString("Returns"), FString::Printf(TEXT("%i"), PlayerState->FlagReturns))
	];
	Panel->AddSlot().Padding(30.0, 5.0, 30.0, 15.0)
	[
		NewPlayerInfoLine(FString("Assists"), FString::Printf(TEXT("%i"), PlayerState->Assists))
	];

	Super::BuildPlayerInfo(Panel, PlayerState);
}
#endif
