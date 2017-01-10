// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTProgressionStorage.h"
#include "UTProfileSettings.h"

UUTProgressionStorage::UUTProgressionStorage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bNeedsUpdate = false;
}

void UUTProgressionStorage::VersionFixup()
{
	TokensCommit(); // just in case any achievements failed to unlock previously due to bug
}

bool UUTProgressionStorage::HasTokenBeenPickedUpBefore(FName TokenUniqueID)
{
	return FoundTokenUniqueIDs.Contains(TokenUniqueID);
}

void UUTProgressionStorage::TokenPickedUp(FName TokenUniqueID)
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.AddUnique(TokenUniqueID);
}

void UUTProgressionStorage::TokenRevoke(FName TokenUniqueID)
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Remove(TokenUniqueID);
}

void UUTProgressionStorage::TokensCommit()
{
	for (FName ID : TempFoundTokenUniqueIDs)
	{
		FoundTokenUniqueIDs.AddUnique(ID);
	}

	// see if all achievement tokens have been picked up
	if (!Achievements.Contains(AchievementIDs::TutorialComplete))
	{
		bool bCompletedTutorial = true;
		static TArray<FName, TInlineAllocator<60>> TutorialTokens = []()
		{
			TArray<FName, TInlineAllocator<60>> List;
			FNumberFormattingOptions Options;
			Options.MinimumIntegralDigits = 3;
			for (int32 i = 0; i < 15; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("movementtraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 15; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("weapontraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 10; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("pickuptraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("tuba_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("outpost23_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("face_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("asdf_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			return List;
		}();
		for (FName TestToken : TutorialTokens)
		{
			if (!FoundTokenUniqueIDs.Contains(TestToken))
			{
				bCompletedTutorial = false;
				break;
			}
		}
		if (bCompletedTutorial)
		{
			Achievements.Add(AchievementIDs::TutorialComplete);
			UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(GEngine->GetFirstGamePlayer(GWorld));	
			if (LP)
			{
				LP->ShowToast(NSLOCTEXT("UT", "ItemRewardVise", "You earned Visse - The Armor of Sacrifce!"));
			}
		}
	}

	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
}

void UUTProgressionStorage::TokensReset()
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
}

void UUTProgressionStorage::TokensClear()
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
	FoundTokenUniqueIDs.Empty();
}

bool UUTProgressionStorage::GetBestTime(FName TimingName, float& OutBestTime)
{
	OutBestTime = 0;

	float* BestTime = BestTimes.Find(TimingName);
	if (BestTime)
	{
		OutBestTime = *BestTime;
		return true;
	}

	return false;
}

void UUTProgressionStorage::SetBestTime(FName TimingName, float InBestTime)
{
	if (!BestTimes.Contains(TimingName) || BestTimes[TimingName] > InBestTime)
	{
		BestTimes.Add(TimingName, InBestTime);
		bNeedsUpdate = true;

		// hacky halloween reward implementation
		if (TimingName == AchievementIDs::FacePumpkins)
		{
			if (InBestTime >= 6666.0f)
			{
				for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
				{
					if (It->GetProgressionStorage() == this)
					{
						It->AwardAchievement(AchievementIDs::FacePumpkins);
					}
				}
			}

			if (InBestTime >= 5000.0f)
			{
				for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
				{
					if (It->GetProgressionStorage() == this)
					{
						It->AwardAchievement(AchievementIDs::PumpkinHead2015Level3);
					}
				}
			}

			if (InBestTime >= 1000.0f)
			{
				for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
				{
					if (It->GetProgressionStorage() == this)
					{
						It->AwardAchievement(AchievementIDs::PumpkinHead2015Level2);
					}
				}
			}

			if (InBestTime >= 200.0f)
			{
				for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
				{
					if (It->GetProgressionStorage() == this)
					{
						It->AwardAchievement(AchievementIDs::PumpkinHead2015Level1);
					}
				}
			}
		}
	}
}

/** Kind of hacky fix up until we move to the ladder system */
void UUTProgressionStorage::FixupBestTimes(int32& TutorialMask)
{
	// These are converted
	static FName OldMovementTrainingName = FName(TEXT("movementtraining_timingsection"));	
	static FName OldWeaponTrainingName = FName(TEXT("weapontraining_timingsection"));	
	static FName OldPickupTrainingName = FName(TEXT("pickuptraining_timingsection"));	

	// These are just removed
	static FName OldFlagRunTrainingName = FName(TEXT("flagrun_timingsection"));	
	static FName OldDMTrainingName = FName(TEXT("deathmatch_timingsection"));
	static FName OldTDMTrainingName = FName(TEXT("teamdeathmatch_timingsection"));
	static FName OldCTFTrainingName = FName(TEXT("capturetheflag_timingsection"));
	static FName OldDuelTrainingName = FName(TEXT("duel_timingsection"));

	if (BestTimes.Contains(OldMovementTrainingName))
	{
		BestTimes.Add(ETutorialTags::TUTTAG_Movement, BestTimes[OldMovementTrainingName]);
		BestTimes.Remove(OldMovementTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_Movement;
	}
		
	if (BestTimes.Contains(OldWeaponTrainingName))
	{
		BestTimes.Add(ETutorialTags::TUTTAG_Weapons, BestTimes[OldWeaponTrainingName]);
		BestTimes.Remove(OldWeaponTrainingName);
		TutorialMask = TutorialMask | TUTOIRAL_Weapon;
	}

	if (BestTimes.Contains(OldPickupTrainingName))
	{
		BestTimes.Add(ETutorialTags::TUTTAG_Pickups, BestTimes[OldPickupTrainingName]);
		BestTimes.Remove(OldPickupTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_Pickups;
	}

	if (BestTimes.Contains(OldFlagRunTrainingName))
	{
		BestTimes.Remove(OldFlagRunTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_FlagRun;
	}

	if (BestTimes.Contains(OldDMTrainingName))
	{
		BestTimes.Remove(OldDMTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_DM;
	}

	if (BestTimes.Contains(OldTDMTrainingName))
	{
		BestTimes.Remove(OldTDMTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_TDM;
	}

	if (BestTimes.Contains(OldCTFTrainingName))
	{
		BestTimes.Remove(OldCTFTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_CTF;
	}

	if (BestTimes.Contains(OldDuelTrainingName))
	{
		BestTimes.Remove(OldDuelTrainingName);
		TutorialMask = TutorialMask | TUTORIAL_Showdown;
	}

}
