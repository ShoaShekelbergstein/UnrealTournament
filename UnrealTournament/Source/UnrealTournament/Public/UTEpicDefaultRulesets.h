// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTATypes.h"
#include "UTReplicatedGameRuleset.h"
#include "UTEpicDefaultRulesets.generated.h"


UCLASS()
class UNREALTOURNAMENT_API UUTEpicDefaultRulesets : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	static void GetEpicRulesets(TArray<FString>& Rules)
	{
		Rules.Add(EEpicDefaultRuleTags::FlagRun);
		Rules.Add(EEpicDefaultRuleTags::FlagRunVSAI);
		Rules.Add(EEpicDefaultRuleTags::Deathmatch);
		Rules.Add(EEpicDefaultRuleTags::Siege);
		Rules.Add(EEpicDefaultRuleTags::CTF);
		Rules.Add(EEpicDefaultRuleTags::TDM);
		Rules.Add(EEpicDefaultRuleTags::BIGCTF);
		Rules.Add(EEpicDefaultRuleTags::COMPCTF);
		Rules.Add(EEpicDefaultRuleTags::SHOWDOWN);
		Rules.Add(EEpicDefaultRuleTags::TEAMSHOWDOWN);
		Rules.Add(EEpicDefaultRuleTags::BigDM);
		Rules.Add(EEpicDefaultRuleTags::DUEL);
		Rules.Add(EEpicDefaultRuleTags::iDM);
		Rules.Add(EEpicDefaultRuleTags::iTDM);
		Rules.Add(EEpicDefaultRuleTags::iCTF);
		Rules.Add(EEpicDefaultRuleTags::iCTFT);
	}

	static void InsureEpicDefaults(FUTGameRuleset* NewRuleset)
	{
		// TODO: This should pull from a file that is pushed from the MCP if the MCP is available

		if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::Deathmatch, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Featured"));

			NewRuleset->Title = TEXT("Deathmatch");
			NewRuleset->Tooltip = TEXT("Standard free-for-all Deathmatch.");
			NewRuleset->Description = TEXT("Standard free-for-all deathmatch.\n\n<UT.Hub.RulesText_Small>TimeLimit : %TimeLimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players : %MaxPlayers%</>");
			NewRuleset->MaxPlayers = 8;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_DM.GB_DM'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTDMGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=10?GoalScore=0"));
			NewRuleset->bTeamGame = false;
						
			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Outpost23";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Lea/DM-Lea";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-BioTower";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Deadfall";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-NickTest1";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-ASDF";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/DM-Chill"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/DM-Outpost23"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/DM-Underland"));
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::BigDM, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Classic"));

			NewRuleset->Title = TEXT("Big Deathmatch");
			NewRuleset->Tooltip = TEXT("Deathmatch with large player counts on big maps.");
			NewRuleset->Description = TEXT("Deathmatch with large player counts on big maps.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players: %maxplayers%</>");
			NewRuleset->MaxPlayers = 12;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_LargeDM.GB_LargeDM'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTDMGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=10?GoalScore=0"));
			NewRuleset->bTeamGame = false;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Outpost23";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Deadfall";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Temple";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::TDM, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Classic"));

			NewRuleset->Title = TEXT("Team Deathmatch");
			NewRuleset->Tooltip = TEXT("Red versus blue team deathmatch.");
			NewRuleset->Description = TEXT("Red versus blue team deathmatch.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players: %maxplayers%</>");
			NewRuleset->MaxPlayers = 10;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_TDM.GB_TDM'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTTeamDMGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Outpost23";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Lea/DM-Lea";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Deadfall";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Temple";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-NickTest1";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-ASDF";


			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::TEAMSHOWDOWN, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty();
			NewRuleset->Categories.Add(TEXT("Competitive"));

			NewRuleset->Title = TEXT("Showdown");
			NewRuleset->Tooltip = TEXT("Red versus blue team showdown.");
			NewRuleset->Description = TEXT("Red versus blue team showdown.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minute rounds</>\n<UT.Hub.RulesText_Small>Scoring : First to %goalscore% wins</>\n<UT.Hub.RulesText_Small>Maximum players: %maxplayers%</>");
			NewRuleset->MaxPlayers = 6;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_TDM.GB_TDM'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTTeamShowdownGame");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=2?GoalScore=5"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList = 16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Lea/DM-Lea";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Temple";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-ASDF";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::DUEL, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Competitive"));

			NewRuleset->Title = TEXT("Duel");
			NewRuleset->Tooltip = TEXT("One vs one test of deathmatch skill.");
			NewRuleset->Description = TEXT("One vs one test of deathmatch skill.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players: %maxplayers%</>");
			NewRuleset->MaxPlayers = 2;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_Duel.GB_Duel'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTDuelGame");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=10?GoalScore=0"));
			NewRuleset->bTeamGame = true;
			NewRuleset->bCompetitiveMatch = true;
			NewRuleset->MaxMapsInList=16;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/WIP/DM-ASDF";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Lea/DM-Lea";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/WIP/DM-ASDF";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::SHOWDOWN, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Competitive"));
			NewRuleset->bCompetitiveMatch = true;
			NewRuleset->Title = TEXT("1v1 Showdown");
			NewRuleset->Tooltip = TEXT("New School one vs one test of deathmatch skill.");
			NewRuleset->Description = TEXT("New School one vs one test of deathmatch skill.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minute rounds</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 2;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_Duel.GB_Duel'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTShowdownGame");
			NewRuleset->GameOptions = FString(TEXT("?Timelimit=2?GoalScore=5?RequireFull=1"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/WIP/DM-ASDF";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Lea/DM-Lea";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/WIP/DM-ASDF";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::CTF, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Classic"));

			NewRuleset->Title = TEXT("Capture the Flag");
			NewRuleset->Tooltip = TEXT("Capture the Flag.");
			NewRuleset->Description = TEXT("Capture the Flag, with guns.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes with halftime</>\n<UT.Hub.RulesText_Small>Mercy Rule : On</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 10;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_CTF.GB_CTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTCTFGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-Face";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Pistola/CTF-Pistola";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Polaris/CTF-Polaris";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Blank";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Quick";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Plaza";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-BigRock";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Volcano";
			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/CTF-TitanPass"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/CTF-Face"));
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::BIGCTF, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Classic"));

			NewRuleset->Title = TEXT("Big CTF");
			NewRuleset->Tooltip = TEXT("Capture the Flag with large teams.");
			NewRuleset->Description = TEXT("Capture the Flag with large teams.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes with halftime</>\n<UT.Hub.RulesText_Small>Mercy Rule : On</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 20;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_LargeCTF.GB_LargeCTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTCTFGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-Face";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Volcano";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-BigRock";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Dam";
			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/CTF-Face";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::COMPCTF, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty();
			NewRuleset->Categories.Add(TEXT("Competitive"));

			NewRuleset->Title = TEXT("Competitive CTF");
			NewRuleset->Tooltip = TEXT("Capture the Flag with competition rules.");
			NewRuleset->Description = TEXT("Capture the Flag, with guns.\n\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes with halftime</>\n<UT.Hub.RulesText_Small>Mercy Rule : Off</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 10;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_CTF.GB_CTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTCTFGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0?RequireReady=1?MercyScore=0"));
			NewRuleset->bCompetitiveMatch = true;
			NewRuleset->bTeamGame = true;
			NewRuleset->MaxMapsInList = 16;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-Face";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Pistola/CTF-Pistola";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Polaris/CTF-Polaris";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Blank";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Quick";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Plaza";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-BigRock";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Volcano";
			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/CTF-TitanPass"));
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::iDM, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Instagib"));

			NewRuleset->Title = TEXT("Instagib DM");
			NewRuleset->Tooltip = TEXT("One hit one kill Deathmatch.");
			NewRuleset->Description = TEXT("One hit one kill Deathmatch.\n\n<UT.Hub.RulesText_Small>Mutators : Instagib</>\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 10;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_InstagibDM.GB_InstagibDM'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTDMGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=10?GoalScore=0?Mutator=Instagib"));
			NewRuleset->bTeamGame = false;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Outpost23";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Deadfall";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Temple";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-NickTest1";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-ASDF";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::iTDM, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Instagib"));

			NewRuleset->Title = TEXT("Instagib TDM");
			NewRuleset->Tooltip = TEXT("One hit one kill Team Deathmatch.");
			NewRuleset->Description = TEXT("One hit one kill Team Deathmatch.\n\n<UT.Hub.RulesText_Small>Mutators : Instagib</>\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 20;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_InstagibDuel.GB_InstagibDuel'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTTeamDMGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0?Mutator=Instagib"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/DM-Outpost23";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Chill";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/DM-Underland";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Unsaved/DM-Unsaved";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Backspace/DM-Backspace";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Salt/DM-Salt";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Batrankus/DM-Batrankus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Sand";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Spacer";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Cannon";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Deadfall";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Temple";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Focus";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-NickTest1";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Solo";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-Decktest";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/DM-ASDF";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/DM-Chill";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::iCTF, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Instagib"));

			NewRuleset->Title = TEXT("Instagib CTF");
			NewRuleset->Tooltip = TEXT("Instagib CTF");
			NewRuleset->Description = TEXT("Capture the Flag with Instagib rifles.\n\n<UT.Hub.RulesText_Small>Mutators : Instagib</>\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% minutes with halftime</>\n<UT.Hub.RulesText_Small>Mercy Rule : On</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 20;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_InstagibCTF.GB_InstagibCTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTCTFGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0?Mutator=Instagib"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-Face";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Pistola/CTF-Pistola";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Polaris/CTF-Polaris";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Blank";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-BigRock";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Volcano";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Quick";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Plaza";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::iCTFT, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty(); 
			NewRuleset->Categories.Add(TEXT("Instagib"));

			NewRuleset->Title = TEXT("Translocator iCTF");
			NewRuleset->Tooltip = TEXT("Translocator iCTF");
			NewRuleset->Description = TEXT("Capture the Flag with Instagib rifles and Translocators.\n\n<UT.Hub.RulesText_Small>Mutators : Instagib</>\n<UT.Hub.RulesText_Small>TimeLimit : %timelimit% with halftime</>\n<UT.Hub.RulesText_Small>Mercy Rule : On</>\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 20;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_InstagibCTF.GB_InstagibCTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTCTFGameMode");
			NewRuleset->GameOptions = FString(TEXT("?TimeLimit=20?GoalScore=0?Mutator=Instagib,AddTrans"));
			NewRuleset->bTeamGame = true;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->MaxMapsInList=16;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/CTF-Face";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Pistola/CTF-Pistola";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/EpicInternal/Polaris/CTF-Polaris";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Blank";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-BigRock";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Volcano";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Quick";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/CTF-Plaza";

			NewRuleset->DefaultMap = "/Game/RestrictedAssets/Maps/CTF-TitanPass";
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::FlagRun, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty();
			NewRuleset->Categories.Add(TEXT("Featured"));

			NewRuleset->Title = TEXT("Flag Run");
			NewRuleset->Tooltip = TEXT("Attackers must deliver their flag to the enemy base.");
			NewRuleset->Description = TEXT("Flag Run.\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 10;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_CTF.GB_CTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTFlagRunGame");
			NewRuleset->GameOptions = FString(TEXT(""));
			NewRuleset->bTeamGame = true;
			NewRuleset->MaxMapsInList = 16;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_All;

			NewRuleset->EpicMaps ="/Game/RestrictedAssets/Maps/WIP/FR-Fort";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-MeltDown";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Loh";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-BlackStone";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Heist";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-HighRoad";

			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Fort"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Meltdown"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Loh"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Blackstone"));
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::FlagRunVSAI, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty();
			NewRuleset->Categories.Add(TEXT("Featured"));

			NewRuleset->Title = TEXT("FlagRun vs AI");
			NewRuleset->Tooltip = TEXT("Co-op vs AI.  Attackers must deliver their flag to the enemy base.");
			NewRuleset->Description = TEXT("Flag Run Coop vs AI.\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 5;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_CTF.GB_CTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTFlagRunGame");
			NewRuleset->GameOptions = FString(TEXT("?VSAI=1"));
			NewRuleset->bTeamGame = true;
			NewRuleset->MaxMapsInList = 16;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_BotSkill;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/WIP/FR-Fort";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-MeltDown";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Loh";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-BlackStone";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Heist";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-HighRoad";

			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Fort"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Meltdown"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Loh"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Blackstone"));
		}
		else if (NewRuleset->UniqueTag.Equals(EEpicDefaultRuleTags::Siege, ESearchCase::IgnoreCase))
		{
			NewRuleset->Categories.Empty();
			NewRuleset->Categories.Add(TEXT("Featured"));

			NewRuleset->Title = TEXT("Siege");
			NewRuleset->Tooltip = TEXT("Prototype PVE mode.  Defend your base against hordes of incoming enemies trying to deliver their flag.");
			NewRuleset->Description = TEXT("PROTOTYPE PVE mode.  Defend your base against hordes of incoming enemies trying to deliver their flag..\n<UT.Hub.RulesText_Small>Maximum players : %maxplayers%</>");
			NewRuleset->MaxPlayers = 5;
			NewRuleset->DisplayTexture = TEXT("Texture2D'/Game/RestrictedAssets/UI/GameModeBadges/GB_CTF.GB_CTF'");
			NewRuleset->GameMode = TEXT("/Script/UnrealTournament.UTFlagRunPvEGame");
			NewRuleset->GameOptions = FString(TEXT(""));
			NewRuleset->bTeamGame = true;
			NewRuleset->MaxMapsInList = 16;

			NewRuleset->OptionFlags = GAME_OPTION_FLAGS_BotSkill + GAME_OPTION_FLAGS_RequireFull;

			NewRuleset->EpicMaps = "/Game/RestrictedAssets/Maps/WIP/FR-Fort";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-MeltDown";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Loh";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-BlackStone";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-Heist";
			NewRuleset->EpicMaps = NewRuleset->EpicMaps + ",/Game/RestrictedAssets/Maps/WIP/FR-HighRoad";

			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Fort"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Meltdown"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Loh"));
			NewRuleset->QuickPlayMaps.Add(TEXT("/Game/RestrictedAssets/Maps/WIP/FR-Blackstone"));
		}
	}
};



