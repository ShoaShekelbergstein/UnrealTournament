// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnalyticsET.h"
#include "IAnalyticsProviderET.h"
#include "UnrealTemplate.h"
#include "UTAnalyticParams.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUTAnalytics, Display, All);

class IAnalyticsProvider;
class FUTAnalytics : FNoncopyable
{
public:
	static UNREALTOURNAMENT_API IAnalyticsProviderET& GetProvider();

	static UNREALTOURNAMENT_API TSharedPtr<IAnalyticsProviderET> GetProviderPtr();
	
	/** Helper function to determine if the provider is valid. */
	static UNREALTOURNAMENT_API bool IsAvailable() 
	{ 
		return Analytics.IsValid(); 
	}
	
	/** Called to initialize the singleton. */
	static void Initialize();
	
	/** Called to shut down the singleton */
	static void Shutdown();

	/** 
	 * Called when the login status has changed. Checks IsAvailable() internally, so external code doesn't need to.
	 * @param NewAccountID The new AccountID of the user, or empty if the user logged out.
	 */
	static void LoginStatusChanged(FString NewAccountID);

	const static FString AnalyticsLoggedGameOption;
	const static FString AnalyticsLoggedGameOptionTrue;

/** Analytics events*/
public:
	/* Server metrics */
	static void FireEvent_ServerUnplayableCondition(AUTGameMode* UTGM, double HitchThresholdInMs, int32 NumHitchesAboveThreshold, double TotalUnplayableTimeInMs);
	static void FireEvent_PlayerContextLocationPerMinute(AUTPlayerController* UTPC, FString& PlayerContextLocation, const int32 NumSocialPartyMembers);
	static void FireEvent_UTServerFPSCharts(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& InParamArray);
	static void FireEvent_UTServerWeaponKills(AUTGameMode* UTGM, TMap<TSubclassOf<UDamageType>, int32>* KillsArray);
	static void FireEvent_UTStartRankedMatch(AUTGameMode* UTGM);
	static void FireEvent_UTEndRankedMatch(AUTGameMode* UTGM);
	static void FireEvent_UTServerPlayerJoin(AUTGameMode* UTGM, AUTPlayerState* UTPS);
	static void FireEvent_UTServerPlayerDisconnect(AUTGameMode* UTGM, AUTPlayerState* UTPS);

	/* Client metrics */
	static void FireEvent_UTFPSCharts(AUTPlayerController* UTPC, TArray<FAnalyticsEventAttribute>& InParamArray);
	static void FireEvent_EnterMatch(AUTPlayerController* UTPC, FString EnterMethod);
	static void FireEvent_UTTutorialPickupToken(AUTPlayerController* UTPC, FName TokenID, FString TokenDescription);
	static void FireEvent_UTTutorialPlayInstruction(AUTPlayerController* UTPC, FString TutorialName, int32 InstructionID, FString OptionalObjectName = FString());
	static void FireEvent_UTTutorialStarted(AUTPlayerController* UTPC, FString TutorialMap);
	static void FireEvent_UTTutorialCompleted(AUTPlayerController* UTPC, FString TutorialMap);
	static void FireEvent_UTTutorialQuit(AUTPlayerController* UTPC, FString TutorialMap);
	static void FireEvent_UTCancelOnboarding(AUTPlayerController* UTPC);
	static void FireEvent_UTGraphicsSettings(AUTPlayerController* UTPC);

	/* GameMode Metrics*/
	static void FireEvent_FlagRunRoundEnd(class AUTFlagRunGame* UTGame, bool bIsDefenseRoundWin, bool bIsFinalRound);
	static void FireEvent_PlayerUsedRally(AUTGameMode* UTGM, AUTPlayerState* UTPS);
	static void FireEvent_RallyPointBeginActivate(AUTGameMode* UTGM, AUTPlayerState* UTPS);
	static void FireEvent_RallyPointCompleteActivate(AUTGameMode* UTGM, AUTPlayerState* UTPS);


	//Param name generalizer
	static FString GetGenericParamName(EGenericAnalyticParam::Type InGenericParam);

	/*Parameter Array Helpers*/
	static void SetMatchInitialParameters(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& ParamArray, bool bNeedMatchTime, bool bIsRankedMatch = false);
	static void SetServerInitialParameters(TArray<FAnalyticsEventAttribute>& ParamArray);
	static void SetClientInitialParameters(AUTPlayerController* UTPC, TArray<FAnalyticsEventAttribute>& ParamArray, bool bNeedMatchTime);
	static void AddPlayerListToParameters(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& ParamArray);

private:
	/** Initialize the FString Array of Analytic Parameters */
	static void InitializeAnalyticParameterNames();

/** Analytics Helper Functions */
private:
	static FString GetPlatform();
	static int32 GetMatchTime(AUTPlayerController* UTPC);
	static int32 GetMatchTime(AUTGameMode* UTGM);
	static FString GetMapName(AUTPlayerController* UTPC);
	static FString GetMapName(AUTGameMode* UTGM);
	static FString GetGameModeName(AUTPlayerController* UTPC);
	static FString GetEpicAccountName(AUTPlayerController* UTPC);
	static FString GetEpicAccountName(AUTPlayerState* UTPS);
	static FString GetBuildType();

	enum class EAccountSource
	{
		EFromRegistry,
		EFromOnlineSubsystem,
	};

	/** Private helper for setting the UserID. Assumes the instance is valid. Not to be used by external code. */
	static void PrivateSetUserID(const FString& AccountID, EAccountSource AccountSource);

	static bool bIsInitialized;
	static TSharedPtr<IAnalyticsProviderET> Analytics;
	static FString CurrentAccountID;
	static EAccountSource CurrentAccountSource;

	static TArray<FString> GenericParamNames;
};