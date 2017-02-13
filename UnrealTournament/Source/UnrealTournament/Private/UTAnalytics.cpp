// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTGameMode.h"
#include "UTGameState.h"
#include "UTAnalytics.h"
#include "AnalyticsET.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PerfCountersHelpers.h"
#include "QosInterface.h"

#include "UTFlagRunGame.h"
#include "UTFlagRunGameState.h"

#if WITH_PROFILE
#include "GameServiceMcp.h"
#include "OnlineSubsystemMcp.h"
#include "OnlineHttpRequest.h"
#endif

#include "OnlineSubsystemUtils.h"
#include "UTMcpUtils.h"

#include "UTPickupToken.h"


#if WITH_QOSREPORTER
	#include "QoSReporter.h"
#endif // WITH_QOSREPORTER
#include "IAnalyticsProvider.h"
#include "IAnalyticsProviderET.h"
#include "Analytics.h"

DEFINE_LOG_CATEGORY(LogUTAnalytics);

TArray<FString> FUTAnalytics::GenericParamNames;

const FString FUTAnalytics::AnalyticsLoggedGameOption = "AnalyticsLogged";
const FString FUTAnalytics::AnalyticsLoggedGameOptionTrue = "?AnalyticsLogged=true";

bool FUTAnalytics::bIsInitialized = false;
TSharedPtr<IAnalyticsProviderET> FUTAnalytics::Analytics = NULL;
// initialize to a dummy value to ensure the first time we set the AccountID it detects it as a change.
FString FUTAnalytics::CurrentAccountID(TEXT("__UNINITIALIZED__"));
FUTAnalytics::EAccountSource FUTAnalytics::CurrentAccountSource = FUTAnalytics::EAccountSource::EFromRegistry;

/**
 * On-demand construction of the singleton. 
 */
IAnalyticsProviderET& FUTAnalytics::GetProvider()
{
	checkf(bIsInitialized && Analytics.IsValid(), TEXT("FUTAnalytics::GetProvider called outside of Initialize/Shutdown."));
	return *Analytics.Get();
}

TSharedPtr<IAnalyticsProviderET> FUTAnalytics::GetProviderPtr()
{
	return Analytics;
}

static const FString SecureAnalyticsEndpoint = TEXT("https://datarouter.ol.epicgames.com/");

void FUTAnalytics::Initialize()
{
	if (IsRunningCommandlet() || GIsEditor || GIsPlayInEditorWorld)
	{
		return;
	}

	checkf(!bIsInitialized, TEXT("FUTAnalytics::Initialize called more than once."));
	
	if (!Analytics.IsValid())
	{
		FString GameAppId = FString::Printf(TEXT("UnrealTournament.%s"), *GetBuildType());
		Analytics = FAnalyticsET::Get().CreateAnalyticsProvider(FAnalyticsET::Config(GameAppId, SecureAnalyticsEndpoint, GetDefault<UGeneralProjectSettings>()->ProjectVersion + TEXT(" - %VERSION%")));
	}

	// Set the UserID using the AccountID regkey if present.
	LoginStatusChanged(FString());

	InitializeAnalyticParameterNames();
	bIsInitialized = true;
}

FString FUTAnalytics::GetBuildType()
{
	FString BuildType = FString();

#if WITH_PROFILE
	FOnlineSubsystemMcp* WorldMcp = (FOnlineSubsystemMcp*)Online::GetSubsystem(nullptr, MCP_SUBSYSTEM);
	if (WorldMcp)
	{
		FGameServiceMcpPtr MCPService = WorldMcp->GetMcpGameService();
		if (MCPService.IsValid())
		{
			BuildType = MCPService->GetGameBackendName();
		}
	}
#endif

	if (BuildType.IsEmpty())
	{
		//BuildType = FAnalytics::ToString(FAnalytics::Get().GetBuildType());
	}

	return BuildType;
}


void FUTAnalytics::InitializeAnalyticParameterNames()
{
	//Generic STATS
	GenericParamNames.Reserve(EGenericAnalyticParam::NUM_GENERIC_PARAMS);
	GenericParamNames.SetNum(EGenericAnalyticParam::NUM_GENERIC_PARAMS);


	// server stats
#define AddGenericParamName(ParamName) \
	GenericParamNames[EGenericAnalyticParam::ParamName] = TEXT(#ParamName)

	AddGenericParamName(PlayerGUID);
	AddGenericParamName(PlayerList);
	AddGenericParamName(InactivePlayerList);
	AddGenericParamName(ServerInstanceGUID);
	AddGenericParamName(ServerMatchGUID);
	AddGenericParamName(ContextGUID);
	AddGenericParamName(MatchTime);
	AddGenericParamName(MapName);
	AddGenericParamName(GameModeName);

	AddGenericParamName(Hostname);
	AddGenericParamName(SystemId);
	AddGenericParamName(InstanceId);
	AddGenericParamName(BuildVersion);
	AddGenericParamName(MachinePhysicalRAMInGB);
	AddGenericParamName(NumLogicalCoresAvailable);
	AddGenericParamName(NumPhysicalCoresAvailable);
	AddGenericParamName(MachineCPUSignature);
	AddGenericParamName(MachineCPUBrandName);
	AddGenericParamName(NumClients);
	
	AddGenericParamName(Platform);
	AddGenericParamName(Location);
	AddGenericParamName(SocialPartyCount);
	AddGenericParamName(IsIdle);
	AddGenericParamName(RegionId);
	AddGenericParamName(PlayerContextLocationPerMinute);

	AddGenericParamName(HitchThresholdInMs);
	AddGenericParamName(NumHitchesAboveThreshold);
	AddGenericParamName(TotalUnplayableTimeInMs);
	AddGenericParamName(ServerUnplayableCondition);

	AddGenericParamName(WeaponName);
	AddGenericParamName(NumKills);
	AddGenericParamName(UTServerWeaponKills);
	AddGenericParamName(WeaponInfo);

	AddGenericParamName(UTFPSCharts);
	AddGenericParamName(UTServerFPSCharts);

	AddGenericParamName(Team);
	AddGenericParamName(MaxRequiredTextureSize);

	AddGenericParamName(FlagRunRoundEnd);
	AddGenericParamName(PlayerUsedRally);
	AddGenericParamName(RallyPointBeginActivate);
	AddGenericParamName(RallyPointCompleteActivate);
	AddGenericParamName(OffenseKills);
	AddGenericParamName(DefenseKills);
	AddGenericParamName(DefenseLivesRemaining);
	AddGenericParamName(DefensePlayersEliminated);
	AddGenericParamName(PointsScored);
	AddGenericParamName(DefenseWin);
	AddGenericParamName(TimeRemaining);
	AddGenericParamName(RoundNumber);
	AddGenericParamName(FinalRound);
	AddGenericParamName(EndedInTieBreaker);
	AddGenericParamName(RedTeamBonusTime);
	AddGenericParamName(BlueTeamBonusTime);

	AddGenericParamName(UTEnterMatch);
	AddGenericParamName(EnterMethod);
	AddGenericParamName(UTStartRankedMatch);
	AddGenericParamName(UTEndRankedMatch);
	AddGenericParamName(ELOPlayerInfo);

	AddGenericParamName(UTTutorialPickupToken);
	AddGenericParamName(TokenID);
	AddGenericParamName(TokenDescription);
	AddGenericParamName(HasTokenBeenPickedUpBefore);
	AddGenericParamName(AnnouncementName);
	AddGenericParamName(AnnouncementID);
	AddGenericParamName(OptionalObjectName);
	AddGenericParamName(UTTutorialPlayInstruction);

	AddGenericParamName(UTTutorialStarted);
	AddGenericParamName(UTTutorialCompleted);
	AddGenericParamName(UTTutorialQuit);
	AddGenericParamName(UTCancelOnboarding);
	AddGenericParamName(TutorialMap);
	AddGenericParamName(TokensCollected);
	AddGenericParamName(TokensAvailable);
	AddGenericParamName(MovementTutorialCompleted);
	AddGenericParamName(WeaponTutorialCompleted);
	AddGenericParamName(PickupsTutorialCompleted);

	AddGenericParamName(RealServerFPS);
	
	AddGenericParamName(UTServerPlayerJoin);
	AddGenericParamName(UTServerPlayerDisconnect);

	AddGenericParamName(UTGraphicsSettings);
	AddGenericParamName(AAMode);
	AddGenericParamName(ScreenPercentage);
	AddGenericParamName(IsHRTFEnabled);
	AddGenericParamName(IsKeyboardLightingEnabled);
	AddGenericParamName(ScreenResolution);
	AddGenericParamName(DesktopResolution);
	AddGenericParamName(FullscreenMode);
	AddGenericParamName(IsVSyncEnabled);
	AddGenericParamName(FrameRateLimit);
	AddGenericParamName(OverallScalabilityLevel);
	AddGenericParamName(ViewDistanceQuality);
	AddGenericParamName(ShadowQuality);
	AddGenericParamName(AntiAliasingQuality);
	AddGenericParamName(TextureQuality);
	AddGenericParamName(VisualEffectQuality);
	AddGenericParamName(PostProcessingQuality);
	AddGenericParamName(FoliageQuality);
}

void FUTAnalytics::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	Analytics.Reset();
	bIsInitialized = false;
}

void FUTAnalytics::LoginStatusChanged(FString NewAccountID)
{
	if (IsAvailable())
	{
		// track the source of the account ID. If we get one, it's from OSS. Otherwise, we'll get it from the registry.
		EAccountSource AccountSource = EAccountSource::EFromOnlineSubsystem;
		// empty AccountID tells us we are logged out, and get the cached one from the registry.
		if (NewAccountID.IsEmpty())
		{
			NewAccountID = FPlatformMisc::GetEpicAccountId();
			AccountSource = EAccountSource::EFromRegistry;
		}
		// Restart the session if the AccountID or AccountSource is changing. This prevents spuriously creating new sessions.
		// We restart the session if the AccountSource is changing because it is part of the UserID. This way the analytics
		// system will know that our source for the AccountID is different, even if the ID itself is the same.
		if (NewAccountID != CurrentAccountID || AccountSource != CurrentAccountSource)
		{
			// this will do nothing if a session is not already started.
			Analytics->EndSession();
			PrivateSetUserID(NewAccountID, AccountSource);
			Analytics->StartSession();
		}
	}
}

void FUTAnalytics::PrivateSetUserID(const FString& AccountID, EAccountSource AccountSource)
{
	// Set the UserID to "LoginID|AccountID|OSID|AccountIDSource".
	const TCHAR* AccountSourceStr = AccountSource == EAccountSource::EFromRegistry ? TEXT("Reg") : TEXT("OSS");
	Analytics->SetUserID(FString::Printf(TEXT("%s|%s|%s|%s"), *FPlatformMisc::GetLoginId(), *AccountID, *FPlatformMisc::GetOperatingSystemId(), AccountSourceStr));
	// remember the current value so we don't spuriously restart the session if the user logs in later with the same ID.
	CurrentAccountID = AccountID;
	CurrentAccountSource = AccountSource;
}

FString FUTAnalytics::GetGenericParamName(EGenericAnalyticParam::Type InGenericParam)
{
	if (GIsEditor)
	{
		if (InGenericParam < GenericParamNames.Num())
		{
			return GenericParamNames[InGenericParam];
		}
		return FString();
	}
	else
	{
		check(InGenericParam < GenericParamNames.Num());
		return GenericParamNames[InGenericParam];
	}
}

void FUTAnalytics::SetClientInitialParameters(AUTPlayerController* UTPC, TArray<FAnalyticsEventAttribute>& ParamArray, bool bNeedMatchTime)
{
	if (UTPC)
	{
		if (UTPC->GetWorld())
		{
			AUTGameState* UTGS = UTPC->GetWorld()->GetGameState<AUTGameState>();
			if (UTGS)
			{
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ServerInstanceGUID), UTGS->ServerInstanceGUID.ToString(EGuidFormats::Digits)));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ServerMatchGUID), UTGS->ReplayID));
			
				AUTBaseGameMode* UTGM = Cast<AUTBaseGameMode>(UTPC->GetWorld()->GetAuthGameMode());
				if (UTGM)
				{
					ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ContextGUID), UTGM->ContextGUID));
				}
			}
		}

		if (bNeedMatchTime == true)
		{
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MatchTime), GetMatchTime(UTPC)));
		}

		FString MapName = GetMapName(UTPC);
		const int32 Team = UTPC->GetTeamNum();
		
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPC)));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MapName), MapName));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::Platform), GetPlatform()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::Team), Team));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::GameModeName), GetGameModeName(UTPC)));
	}
}

void FUTAnalytics::SetMatchInitialParameters(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& ParamArray, bool bNeedMatchTime, bool bIsRankedMatch)
{
	if (bNeedMatchTime)
	{
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MatchTime), GetMatchTime(UTGM)));
	}

	if (UTGM)
	{
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::GameModeName), UTGM->DisplayName.ToString()));
		
		if (UTGM->UTGameState)
		{
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ServerInstanceGUID), UTGM->UTGameState->ServerInstanceGUID.ToString(EGuidFormats::Digits)));
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ServerMatchGUID), UTGM->UTGameState->ReplayID));
		}

		if (UTGM->GetWorld())
		{
			//Add all ELO information
			TMap<FString, int32> ELOStats;
			for (FConstPlayerControllerIterator It = UTGM->GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				AUTPlayerController* UTPC = Cast<AUTPlayerController>(*It);
				if (UTPC && UTPC->UTPlayerState && !UTPC->UTPlayerState->bOnlySpectator)
				{
					ELOStats.Add(GetEpicAccountName(UTPC), UTGM->GetEloFor(UTPC->UTPlayerState, bIsRankedMatch));
				}
			}
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ELOPlayerInfo), ELOStats));
		}

		AddPlayerListToParameters(UTGM, ParamArray);
	}

	FString MapName = GetMapName(UTGM);
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MapName), MapName));
}

void FUTAnalytics::SetServerInitialParameters(TArray<FAnalyticsEventAttribute>& ParamArray)
{
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::Hostname), FPlatformProcess::ComputerName()));
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::SystemId), FPlatformMisc::GetOperatingSystemId()));
#if WITH_QOSREPORTER
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::InstanceId), FQoSReporter::GetQoSReporterInstanceId()));
#endif
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::BuildVersion), ENGINE_VERSION_STRING));

	// include some machine and CPU info for easier tracking
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MachinePhysicalRAMInGB), MemoryStats.TotalPhysicalGB));
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::NumLogicalCoresAvailable), FPlatformMisc::NumberOfCoresIncludingHyperthreads()));
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::NumPhysicalCoresAvailable), FPlatformMisc::NumberOfCores()));
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MachineCPUSignature), FPlatformMisc::GetCPUInfo()));
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MachineCPUBrandName), FPlatformMisc::GetCPUBrand()));

#if USE_SERVER_PERF_COUNTERS
	// include information about number of clients in all server events
	ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::NumClients), PerfCountersGet(TEXT("NumClients"), 0)));
#endif

}

void FUTAnalytics::AddPlayerListToParameters(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& ParamArray)
{
	if (UTGM && UTGM->GetWorld())
	{
		TMap<FString,int32> PlayerGUIDs;
		TArray<FString> InactivePlayerGUIDS;

		for (FConstControllerIterator It = UTGM->GetWorld()->GetControllerIterator(); It; ++It)
		{
			AUTPlayerController* UTPC = Cast<AUTPlayerController>(*It);
			if (UTPC && UTPC->PlayerState && !UTPC->PlayerState->bOnlySpectator)
			{
				PlayerGUIDs.Add(GetEpicAccountName(UTPC),UTPC->GetTeamNum());
			}
		}
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerList), PlayerGUIDs));
	
		for (APlayerState* PS : UTGM->InactivePlayerArray)
		{
			AUTPlayerState* UTPS = Cast<AUTPlayerState>(PS);
			if (UTPS)
			{
				InactivePlayerGUIDS.Add(GetEpicAccountName(UTPS));
			}
		}
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::InactivePlayerList), InactivePlayerGUIDS));
	}
}

FString FUTAnalytics::GetPlatform()
{
	FString PlayerPlatform = FString();
	if (PLATFORM_WINDOWS)
	{
		if (PLATFORM_64BITS)
		{
			PlayerPlatform = "Win64";
		}
		else
		{
			PlayerPlatform = "Win32";
		}
	}
	else if (PLATFORM_LINUX)
	{
		PlayerPlatform = "Linux";
	}
	else if (PLATFORM_MAC)
	{
		PlayerPlatform = "Mac";
	}
	else
	{
		PlayerPlatform = "Unknown";
	}

	return PlayerPlatform;
}

int32 FUTAnalytics::GetMatchTime(AUTPlayerController* UTPC)
{
	int32 MatchTime = 0;
	if (UTPC && UTPC->GetWorld())
	{
		if (AUTGameState* UTGameState = Cast<AUTGameState>(UTPC->GetWorld()->GetGameState()))
		{
			MatchTime = UTGameState->GetServerWorldTimeSeconds();
		}
	}

	return MatchTime;
}

int32 FUTAnalytics::GetMatchTime(AUTGameMode* UTGM)
{
	int32 MatchTime = 0;
	if (UTGM)
	{
		AUTGameState* UTGameState = Cast<AUTGameState>(UTGM->GameState);
		if (UTGameState)
		{
			MatchTime = UTGameState->GetServerWorldTimeSeconds();
		}
	}

	return MatchTime;
}


FString FUTAnalytics::GetMapName(AUTPlayerController* UTPC)
{
	FString MapName;
	if (UTPC && UTPC->GetLevel() && UTPC->GetLevel()->OwningWorld)
	{
		MapName = UTPC->GetLevel()->OwningWorld->GetMapName();
	}

	return MapName;
}

FString FUTAnalytics::GetMapName(AUTGameMode* UTGM)
{
	FString MapName;
	if (UTGM && UTGM->GetLevel() && UTGM->GetLevel()->OwningWorld)
	{
		MapName = UTGM->GetLevel()->OwningWorld->GetMapName();
	}

	return MapName;
}

FString FUTAnalytics::GetGameModeName(AUTPlayerController* UTPC)
{
	FString GameModeName;
	if (UTPC && UTPC->GetWorld())
	{
		AUTGameState* UTGS = Cast<AUTGameState>(UTPC->GetWorld()->GetGameState());
		if (UTGS)
		{
			const AUTGameMode* UTGM = UTGS->GetDefaultGameMode<AUTGameMode>();
			if (UTGM)
			{
				GameModeName = UTGM->DisplayName.ToString();
			}
		}
	}

	return GameModeName;
}

FString FUTAnalytics::GetEpicAccountName(AUTPlayerController* UTPC)
{
	if (UTPC && UTPC->GetWorld() && UTPC->UTPlayerState)
	{
		AUTGameState* UTGS = UTPC->GetWorld()->GetGameState<AUTGameState>();
		if (UTGS)
		{
			TSharedRef<const FUniqueNetId> UserId = MakeShareable(new FUniqueNetIdString(*UTPC->UTPlayerState->StatsID));
			FText EpicAccountName = UTGS->GetEpicAccountNameForAccount(UserId);

			return EpicAccountName.ToString();
		}
	}

	return FString();
}

FString FUTAnalytics::GetEpicAccountName(AUTPlayerState* UTPS)
{
	if (UTPS->GetWorld() && UTPS)
	{
		AUTGameState* UTGS = UTPS->GetWorld()->GetGameState<AUTGameState>();
		if (UTGS)
		{
			TSharedRef<const FUniqueNetId> UserId = MakeShareable(new FUniqueNetIdString(*UTPS->StatsID));
			FText EpicAccountName = UTGS->GetEpicAccountNameForAccount(UserId);

			return EpicAccountName.ToString();
		}
	}

	return FString();
}

/*
* @EventName UTFPSCharts
*
* @Trigger Fires at the end of the match with the FPS Charts stats for each player
*
* @Type Sent by client
*
* @EventParam PlayerGUID string The GUID identifying the player.
* @EventParam MapName string The name of the played map
* @EventParam GameModeName string The name of the currently played game mode
* @EventParam PlaylistId int32 The playlist of the current match (4=PvP, 5=Coop, 6=Solo)
* @EventParam Bucket_%i_%i_TimePercentage float The percentage of time that the FPS amount was between x - y
* @EventParam Hitch_%i_%i_HitchCount int32 The number of hitches between x - y milliseconds long
* @EventParam Hitch_%i_%i_HitchTime float The time spent in hitchy frames that lasted between x - y milliseconds long
* @EventParam TotalHitches int32 The total number of hitches
* @EventParam TotalGameBoundHitches	int32 The total number of game thread bound hitches
* @EventParam TotalRenderBoundHitches int32 The total number of render thread bound hitches
* @EventParam TotalGPUBoundHitches int32 The total number of gpu bound hitches
* @EventParam TotalTimeInHitchFrames float The total time spent in all hitch buckets
* @EventParam PercentSpentHitching float The percentage of time spent hitching (has the desired frame time subtracted out of hitch frames before computing)
* @EventParam HitchesPerMinute float The avg. number of hitches per minute played
* @EventParam ChangeList string The change list that was played
* @EventParam BuildType string The games build type that was played
* @EventParam DateStamp string The date stamp that the FPS charts stats took place
* @EventParam Platform string The platform this build was played on
* @EventParam OS string The OS of the client
* @EventParam CPU string The CPU of the client
* @EventParam DesktopGPU string The desktop GPU of the client (may not be teh one we end up using for rendering, see GPUAdapter)
* @EventParam ResolutionQuality float The resolution quality of the client
* @EventParam ViewDistanceQuality int32 The view distance quality of the client
* @EventParam AntiAliasingQuality int32 The anti-aliasing quality of the client
* @EventParam ShadowQuality int32 The shadow quality of the client
* @EventParam PostProcessQuality int32 The post process quality of the client
* @EventParam TextureQuality int32 The texture quality of the client
* @EventParam FXQuality int32 The effects quality of the client
* @EventParam AvgFPS float The average fps of the client
* @EventParam PercentAbove30 float The time percentage when the fps was above 30
* @EventParam PercentAbove60 float The time percentage when the fps was above 60
* @EventParam PercentAbove120 float The time percentage when the fps was above 120
* @EventParam MVP30 float The estimated percentage of missed vsyncs at a target framerate of 30
* @EventParam MVP60 float The estimated percentage of missed vsyncs at a target framerate of 60
* @EventParam MVP120 float The estimated percentage of missed vsyncs at a target framerate of 120
* @EventParam TimeDisregarded float The time that the FPS chart didn't count anywhere
* @EventParam Time float The amount of time FPS Charts was capturing data
* @EventParam FrameCount float The total number of frames
* @EventParam AvgGPUTime float The average time the GPU took to render the frame
* @EventParam PercentGameThreadBound float The percentage of game thread bound frames
* @EventParam PercentRenderThreadBound float The percentage of render thread bound frames
* @EventParam PercentGPUBound float The percentage of GPU bound frames
* @EventParam TotalPhysical The total amount of CPU physical memory detected
* @EventParam TotalVirtual The total amount of CPU virtual memory detected
* @EventParam VRAM The amount of VRAM detected
* @EventParam VSYS The amound of video system memory detected
* @EventParam VSHR The amount of shared video memory detected
* @EventParam CPU_NumCoresP The number of physical CPU cores detected
* @EventParam CPU_NumCoresL The number of logical CPU cores detected (e.g., hyperthreading)
* @EventParam GPUAdapter The GPU adapter string we actually created the D3D11 device for
* @EventParam GPUVendorID The vendor ID for the GPU adapter
* @EventParam GPUDeviceID The device ID for the GPU adapter
* @EventParam GPUDriverVerI The internal driver version string for the GPU
* @EventParam GPUDriverVerU The user facing driver version string for the GPU
* @EventParam CPUBM The last cached value of the CPU benchmark result
* @EventParam GPUBM The last cached value of the GPU benchmark result
* @EventParam ScreenPct The screen percentage for 3D rendering
* @EventParam WindowMode The fullscreen/windowing mode
* @EventParam SizeX The width of the display screen
* @EventParam SizeY The height of the display screen
* @EventParam VSync Whether or not VSync is enabled
* @EventParam FrameRateLimit The frame rate limit
* @EventParam MaxRequiredTextureSize The maximum required texture memory size recorded
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTFPSCharts(AUTPlayerController* UTPC, TArray<FAnalyticsEventAttribute>& InParamArray)
{
	if (UTPC)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			SetClientInitialParameters(UTPC, InParamArray, false);
			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTFPSCharts), InParamArray);
		}
	}
}

/*
* @EventName UTServerFPSCharts
*
* @Trigger Fires at the end of the match with the FPS Charts stats for the server
*
* @Type Sent by the server for the match
*
* @EventParam MapName string The name of the played map
* @EventParam GameModeName string The name of the currently played game mode
* @EventParam Bucket_%i_%i_TimePercentage float The percentage of time that the FPS amount was between x - y
* @EventParam Hitch_%i_%i_HitchCount int32 The number of hitches between x - y milliseconds long
* @EventParam Hitch_%i_%i_HitchTime float The time spent in hitchy frames that lasted between x - y milliseconds long
* @EventParam TotalHitches int32 The total number of hitches
* @EventParam TotalGameBoundHitches	int32 The total number of game thread bound hitches
* @EventParam TotalRenderBoundHitches int32 The total number of render thread bound hitches
* @EventParam TotalGPUBoundHitches int32 The total number of gpu bound hitches
* @EventParam TotalTimeInHitchFrames float The total time spent in all hitch buckets
* @EventParam PercentSpentHitching float The percentage of time spent hitching (has the desired frame time subtracted out of hitch frames before computing)
* @EventParam HitchesPerMinute float The avg. number of hitches per minute played
* @EventParam ChangeList string The change list that was played
* @EventParam BuildType string The games build type that was played
* @EventParam DateStamp string The date stamp that the FPS charts stats took place
* @EventParam Platform string The platform this build was played on
* @EventParam OS string The OS of the server
* @EventParam CPU string The CPU of the server
* @EventParam GPU string The GPU of the server
* @EventParam ResolutionQuality float The resolution quality of the server
* @EventParam ViewDistanceQuality int32 The view distance quality of the server
* @EventParam AntiAliasingQuality int32 The anti-aliasing quality of the server
* @EventParam ShadowQuality int32 The shadow quality of the client
* @EventParam PostProcessQuality int32 The post process quality of the server
* @EventParam TextureQuality int32 The texture quality of the server
* @EventParam FXQuality int32 The effects quality of the server
* @EventParam AvgFPS float This value is used to show performance of the server. IE: If this value is 600, we could run 10 servers per core at 60fps or 20 servers per core at 30 fps.
* @EventParam PercentAbove30 float The time percentage when the fps was above 30
* @EventParam PercentAbove60 float The time percentage when the fps was above 60
* @EventParam PercentAbove120 float The time percentage when the fps was above 120
* @EventParam MVP30 float The estimated percentage of missed vsyncs at a target framerate of 30
* @EventParam MVP60 float The estimated percentage of missed vsyncs at a target framerate of 60
* @EventParam MVP120 float The estimated percentage of missed vsyncs at a target framerate of 120
* @EventParam TimeDisregarded float The time that the FPS chart didn't count anywhere
* @EventParam Time float The amount of time FPS Charts was capturing data
* @EventParam FrameCount float The total number of frames
* @EventParam AvgGPUTime float The average time the GPU took to render the frame
* @EventParam PercentGameThreadBound float The percentage of game thread bound frames
* @EventParam PercentRenderThreadBound float The percentage of render thread bound frames
* @EventParam PercentGPUBound float The percentage of GPU bound frames
* @EventParam RealServerFPS float This value is the actual server FPS.
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTServerFPSCharts(AUTGameMode* UTGM, TArray<FAnalyticsEventAttribute>& InParamArray)
{
	extern float ENGINE_API GAverageFPS;
	InParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RealServerFPS), GAverageFPS));
	
	if (UTGM)
	{
		SetMatchInitialParameters(UTGM, InParamArray, false);

		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTServerFPSCharts), InParamArray);
		}

#if WITH_QOSREPORTER
		if (FQoSReporter::IsAvailable())
		{
			FQoSReporter::GetProvider().RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTServerFPSCharts), InParamArray);
		}
#endif

	}
}

/*
* @EventName ServerUnplayableCondition
*
* @Trigger Fires during the match (on the server) when server reports an unplayable condition.
*
* @Type Sent by the server for the match
*
* @EventParam MatchTime int32 The match time when this event fired
* @EventParam MapName string The name of the played map
* @EventParam Hostname string Machine network name (hostname), can also be used to identify VM in the cloud
* @EventParam SystemId string Unique Id of an operating system install (essentially means "machine id" unless the OS is reinstalled between runs)
* @EventParam InstanceId string Unique Id of a single server instance (stays the same between matches for the whole lifetime of the server)
* @EventParam EngineVersion string Engine version string
* @EventParam MachinePhysicalRAMInGB int32 Total physical memory on the machine in GB
* @EventParam NumLogicalCoresAvailable int32 Number of logical cores available to the process on the machine
* @EventParam NumPhysicalCoresAvailable int32 Number of physical cores available to the process on the machine
* @EventParam MachineCPUSignature int32 CPU signature (CPUID with EAX=1, i.e. family, model, stepping combined).
* @EventParam MachineCPUBrandName string CPU brand string.
* @EventParam NumClients int32 Instantaneous number of clients

* @EventParam HitchThresholdInMs float Unplayable hitch threshold (in ms) as currently configured.
* @EventParam NumHitchesAboveThreshold int32 Number of hitches above the unplayable threshold
* @EventParam TotalUnplayableTimeInMs float Total unplayable time (in ms) as current

* @Comments Reports about an unplayable condition on the server.
*/
void FUTAnalytics::FireEvent_ServerUnplayableCondition(AUTGameMode* UTGM, double HitchThresholdInMs, int32 NumHitchesAboveThreshold, double TotalUnplayableTimeInMs)
{
	if (UTGM)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			AUTGameState* GameState = UTGM->GetGameState<AUTGameState>();
			if (GameState)
			{
				TArray<FAnalyticsEventAttribute> ParamArray;
				SetMatchInitialParameters(UTGM, ParamArray, true);
				SetServerInitialParameters(ParamArray);

				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::HitchThresholdInMs), HitchThresholdInMs));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::NumHitchesAboveThreshold), NumHitchesAboveThreshold));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TotalUnplayableTimeInMs), TotalUnplayableTimeInMs));

				AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::ServerUnplayableCondition), ParamArray);
#if WITH_QOSREPORTER
				if (FQoSReporter::IsAvailable())
				{
					FQoSReporter::GetProvider().RecordEvent(GetGenericParamName(EGenericAnalyticParam::ServerUnplayableCondition), ParamArray);
				}
#endif //WITH_QOSREPORTER
			}
		}
	}
}

/*
* @EventName UTServerWeaponKills
*
* @Trigger Fires every game
*
* @Type Sent by the server
* @EventParam MatchTime int32 The match time when this event fired
* @EventParam MapName string The name of the played map
* @EventParam Hostname string Machine network name (hostname), can also be used to identify VM in the cloud
* @EventParam SystemId string Unique Id of an operating system install (essentially means "machine id" unless the OS is reinstalled between runs)
* @EventParam InstanceId string Unique Id of a single server instance (stays the same between matches for the whole lifetime of the server)
* @EventParam EngineVersion string Engine version string
* @EventParam MachinePhysicalRAMInGB int32 Total physical memory on the machine in GB
* @EventParam NumLogicalCoresAvailable int32 Number of logical cores available to the process on the machine
* @EventParam NumPhysicalCoresAvailable int32 Number of physical cores available to the process on the machine
* @EventParam MachineCPUSignature int32 CPU signature (CPUID with EAX=1, i.e. family, model, stepping combined).
* @EventParam MachineCPUBrandName string CPU brand string.
* @EventParam NumClients int32 Instantaneous number of clients
*
* @EventParam WeaponName string the name of the weapon that got the kills
* @EventParam NumKills int32 the number of kills the weapon got
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTServerWeaponKills(AUTGameMode* UTGM, TMap<TSubclassOf<UDamageType>, int32>* KillsArray)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid() && UTGM && KillsArray)
	{
		TArray<FAnalyticsEventAttribute> ParamArray;
		SetMatchInitialParameters(UTGM, ParamArray, true);
		SetServerInitialParameters(ParamArray);

		TMap<FString, int32> WeaponInfo;
		for (auto& KillElement : *KillsArray)
		{
			WeaponInfo.Add(*KillElement.Key->GetName(), KillElement.Value);
		}

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::WeaponInfo), WeaponInfo));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTServerWeaponKills), ParamArray);
	}
}

/*
* @EventName PlayerContextLocationPerMinute
*
* @Trigger Fires every minute with the players location
*
* @Type Sent by the client
*
* @EventParam Platform string The platform the client is on.
* @EventParam Location string The context location of the player
* @EventParam SocialPartyCount int32 The number of people in a players social party (counts the player as a party member)
* @EventParam GameModeName string The Name of the game mode the player is currently in.
* @EventParam RegionId the region reported by the user (automatic from ping, or self selected in settings)
*
* @Comments
*/
void FUTAnalytics::FireEvent_PlayerContextLocationPerMinute(AUTPlayerController* UTPC, FString& PlayerContextLocation, const int32 NumSocialPartyMembers)
{
	if (UTPC)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			TArray<FAnalyticsEventAttribute> ParamArray;
			
			SetClientInitialParameters(UTPC, ParamArray, true);
			
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::Platform), GetPlatform()));
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::Location), PlayerContextLocation));
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::SocialPartyCount), NumSocialPartyMembers));
			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::GameModeName), GetGameModeName(UTPC)));

			if (FQosInterface::Get()->GetRegionId().IsEmpty() || (FQosInterface::Get()->GetRegionId() == "None"))
			{
				UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(UTPC->GetLocalPlayer());
				if (LP)
				{
					if (LP->IsLoggedIn())
					{
						ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RegionId), TEXT("Unknown")));
					}
					else
					{
						ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RegionId), TEXT("LoggedOut")));
					}
				}
			}
			else
			{
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RegionId), FQosInterface::Get()->GetRegionId()));
			}

			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::PlayerContextLocationPerMinute), ParamArray);
		
			//UE_LOG(UT, Log, TEXT("Sending PlayerContext Location Per Minute Event"));
		}
	}
}


/*
* @EventName FlagRunRoundEnd
*
* @Trigger Fires at the end of each round of Flag Run
*
* @Type Sent by the Server
*
* @EventParam OffenseKills int32 Total Kills on the Offense team this round.
* @EventParam DefenseKills int32 Total Kills on the Defense team this round.
* @EventParam DefenseLivesRemaining int32 Total number of lives left among all defenders in a round.
* @EventParam DefensePlayersEliminated int32 How many eliminated players
* @EventParam PointsScored int32 How many points were scored this round.
* @EventParam DefenseWin bool Was this round won by the defense or offense?
* @EventParam TimeRemaining int32 Time left in the round.
* @EventParam MapName string What Map we were playing on.
* @EventParam RoundNumber int32 What Round Number is this
* @EventParam FinalRound bool Was this the final round? IE: Do we arleady have a winner.
* @EventParam EndedInTieBreaker bool Was this match ended in a tiebreaker, or by a score descrepency.
* @EventParam RedTeamBonusTime int32 How much bonus the Red Team had at this round.
* @EventParam BlueTeamBonusTime int32 How much bonus the Blue Team had at this round.
*
* @Comments
*/
void FUTAnalytics::FireEvent_FlagRunRoundEnd(AUTFlagRunGame* UTGame, bool bIsDefenseRoundWin, bool bIsFinalRound)
{
	if (UTGame)
	{
		AUTFlagRunGameState* UTGS = UTGame->GetGameState<AUTFlagRunGameState>();
		if (UTGS)
		{
			const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
			if (AnalyticsProvider.IsValid())
			{
				TArray<FAnalyticsEventAttribute> ParamArray;
				
				SetMatchInitialParameters(UTGame, ParamArray, true);

				int LivesRemaining = 0;
				int PlayersEliminated = 0;
				for (APlayerState* PS : UTGS->PlayerArray)
				{
					AUTPlayerState* UTPS = Cast<AUTPlayerState>(PS);
					if ((UTPS) && (UTGS->IsTeamOnDefense(UTPS->GetTeamNum())))
					{
						LivesRemaining += UTPS->RemainingLives;
						if (UTPS->bOutOfLives)
						{
							++PlayersEliminated;
						}
					}
				}
				
				bool bEndedInTie = false;
				int RedTeamBonusTime = 0;
				int BlueTeamBonusTime = 0;
				if (UTGS->Teams.Num() > 1)
				{
					bEndedInTie = UTGS->Teams[0]->Score == UTGS->Teams[1]->Score;
					RedTeamBonusTime = UTGS->Teams[0]->RoundBonus;
					BlueTeamBonusTime = UTGS->Teams[1]->RoundBonus;
				}
				
 				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::OffenseKills), UTGS->OffenseKills));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::DefenseKills), UTGS->DefenseKills));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::DefenseLivesRemaining), LivesRemaining));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::DefensePlayersEliminated), PlayersEliminated));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PointsScored), bIsDefenseRoundWin ? UTGame->GetDefenseScore() : UTGame->GetFlagCapScore()));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::DefenseWin), bIsDefenseRoundWin));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TimeRemaining), UTGS->GetRemainingTime()));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MapName), GetMapName(UTGame)));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RoundNumber), UTGS->CTFRound));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::FinalRound), bIsFinalRound));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::EndedInTieBreaker), bEndedInTie));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::RedTeamBonusTime), RedTeamBonusTime));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::BlueTeamBonusTime), BlueTeamBonusTime));

				AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::FlagRunRoundEnd), ParamArray);
			}
		}
	}
}

/*
* @EventName PlayerUsedRally
*
* @Trigger Fires whenever a player uses the rally to move to the rally point
*
* @Type Sent by the Server
*
* @EventParam PlayerGUID FString account name of the player that used the rally
*
* @Comments
*/
void FUTAnalytics::FireEvent_PlayerUsedRally(AUTGameMode* UTGM, AUTPlayerState* UTPS)
{
	if (UTGM && UTPS)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			TArray<FAnalyticsEventAttribute> ParamArray;

			SetMatchInitialParameters(UTGM, ParamArray, true);

			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPS)));

			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::PlayerUsedRally), ParamArray);
		}
	}
}

/*
* @EventName RallyPointBeginActivate
*
* @Trigger Fires whenever a player begins charging a rally point
*
* @Type Sent by the Server
*
* @EventParam PlayerGUID FString account name of the player that is charging the rally
*
* @Comments
*/
void FUTAnalytics::FireEvent_RallyPointBeginActivate(AUTGameMode* UTGM, AUTPlayerState* UTPS)
{
	if (UTGM && UTPS)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			TArray<FAnalyticsEventAttribute> ParamArray;

			SetMatchInitialParameters(UTGM, ParamArray, true);

			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPS)));

			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::RallyPointBeginActivate), ParamArray);
		}
	}
}

/*
* @EventName RallyPointCompleteActivate
*
* @Trigger Fires whenever a player finishes charging a rally and it becomes powered
*
* @Type Sent by the Server
*
* @EventParam PlayerGUID FString account name of the player that completed the rally
*
* @Comments
*/
void FUTAnalytics::FireEvent_RallyPointCompleteActivate(AUTGameMode* UTGM, AUTPlayerState* UTPS)
{
	if (UTGM && UTPS)
	{
		const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
		if (AnalyticsProvider.IsValid())
		{
			TArray<FAnalyticsEventAttribute> ParamArray;

			SetMatchInitialParameters(UTGM, ParamArray, true);

			ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPS)));

			AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::RallyPointCompleteActivate), ParamArray);
		}
	}
}

/*
* @EventName UTEntermatch
*
* @Trigger Fires when a client enters a game. This is to track the way they entered the game.
*
* @Type Sent by the Client
*
* @EventParam EnterMethod string String representation of how the game was entered
*
* @Comments
*/
void FUTAnalytics::FireEvent_EnterMatch(AUTPlayerController* UTPC, FString EnterMethod)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, false);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::EnterMethod), EnterMethod));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTEnterMatch), ParamArray);
	}
}

/*
* @EventName UTTutorialPickupToken
*
* @Trigger Fires when a client is given some instruction inside of a tutorial.
*
* @Type Sent by the Client
*
* @EventParam TokenID string Unique ID of the picked up token.
* @EventParam TokenDescription string Token Description of the picked up token.
* @EventParam HasTokenBeenPickedUpBefore bool If this token was collected on a previous run of the tutorial.
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTTutorialPickupToken(AUTPlayerController* UTPC, FName TokenID, FString TokenDescription)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (UTPC && AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, true);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TokenID), TokenID.ToString()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TokenDescription), TokenDescription));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::HasTokenBeenPickedUpBefore), UUTGameplayStatics::HasTokenBeenPickedUpBefore(UTPC->GetWorld(), TokenID)));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTTutorialPickupToken), ParamArray);
	}
}

/*
* @EventName UTTutorialPlayInstruction
*
* @Trigger Fires when a client is given some instruction inside of a tutorial.
*
* @Type Sent by the Client
*
* @EventParam AnnouncementName FString Name of the announcement that the announcement playing
* @EventParam AnnouncementId int32 ID of the announcement that is played in the tutorial.
* @EventParam OptionalObjectName FString Name of the object this tutorial is about. IE: If it is a weapon it will be the weapon name
* @Comments
*/
void FUTAnalytics::FireEvent_UTTutorialPlayInstruction(AUTPlayerController* UTPC, FString AnnouncementName, int32 AnnoucementID, FString OptionalObjectName)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, true);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::AnnouncementName), AnnouncementName));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::AnnouncementID), AnnoucementID));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::OptionalObjectName), OptionalObjectName));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTTutorialPlayInstruction), ParamArray);
	}
}


/*
* @EventName UTTutorialStarted
*
* @Trigger Fires when a client starts a tutorial
*
* @Type Sent by the Client
*
* @EventParam TutorialMap FString Name of the tutorial map.
* @EventParam MovementTutorialCompleted bool If the movement tutorial has been previously completed
* @EventParam WeaponTutorialCompleted bool If the movement tutorial has been previously completed
* @EventParam PickupsTutorialCompleted If the movement tutorial has been previously completed
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTTutorialStarted(AUTPlayerController* UTPC, FString TutorialMap)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		if (UTPC)
		{
			SetClientInitialParameters(UTPC, ParamArray, true);

			UUTProfileSettings* ProfileSettings = UTPC->GetProfileSettings();
			if (ProfileSettings)
			{
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MovementTutorialCompleted), ((ProfileSettings->TutorialMask & TUTORIAL_Movement) == TUTORIAL_Movement)));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::WeaponTutorialCompleted), ((ProfileSettings->TutorialMask & TUTOIRAL_Weapon) == TUTOIRAL_Weapon)));
				ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PickupsTutorialCompleted), ((ProfileSettings->TutorialMask & TUTORIAL_Pickups) == TUTORIAL_Pickups)));
			}
		}
	
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TutorialMap), TutorialMap));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTTutorialStarted), ParamArray);
	}
}

/*
* @EventName UTTutorialCompleted
*
* @Trigger Fires when a client completes a tutorial
*
* @Type Sent by the Client
*
* @EventParam TutorialMap FString Name of the tutorial map.
* @EventParam TokensCollected int Number of Tokens collected on the map
* @EventParam MaxTokensAvailable int Number of Tokens total on the map.
* @EventParam GameTime float Time 
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTTutorialCompleted(AUTPlayerController* UTPC, FString TutorialMap)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, true);

		int TokensCollected = 0;
		int TokensAvailable = 0;
		if (UTPC && UTPC->GetWorld())
		{
			for (TActorIterator<AUTPickupToken> It(UTPC->GetWorld()); It; ++It)
			{
				if (It->bIsPickedUp)
				{
					++TokensCollected;
				}

				++TokensAvailable;
			}
		}

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TutorialMap), TutorialMap));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TokensCollected), TokensCollected));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TokensAvailable), TokensAvailable));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MatchTime), GetMatchTime(UTPC)));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTTutorialCompleted), ParamArray);
	}
}

/*
* @EventName UTTutorialQuit
*
* @Trigger Fires when a client quits a tutorial
*
* @Type Sent by the Client
*
* @EventParam TutorialMap FString Name of the tutorial map.
* @EventParam GameTime float Time
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTTutorialQuit(AUTPlayerController* UTPC, FString TutorialMap)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, true);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TutorialMap), TutorialMap));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::MatchTime), GetMatchTime(UTPC)));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTTutorialQuit), ParamArray);
	}
}

/*
* @EventName UTCancelOnboarding
*
* @Trigger Fires when a client is forced into onboarding, and cancels out of the onboarding process
*
* @Type Sent by the Client
*
* @EventParam PlayerGUID string GUID to identify the player
* 
* @Comments
*/
void FUTAnalytics::FireEvent_UTCancelOnboarding(AUTPlayerController* UTPC)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, false);

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTCancelOnboarding), ParamArray);
	}
}

/*
* @EventName UTGraphicsSettings,
*
* @Trigger Fires after login by client. Reports all loaded graphics settings.
*
* @Type Sent by the Client
*
* @EventParam AAMode int32 User's AAMode Setting
* @EventParam ScreenPercentage int32 User's ScreenPercentage Setting
* @EventParam IsHRTFEnabled bool If the user has HRTF Enabled
* @EventParam IsKeyboardLightingEnabled bool If the user has Keyboard Lighting Enabled
* @EventParam ScreenResolution string User's In-Game resolution.
* @EventParam DesktopResolution string User's out-of-game desktop resolution.
* @EventParam FullscreenMode int32 User's Fullscreen setting. 0 = Fullscreen, 1 = Windowed Fullscreen 2 = Windowed 
* @EventParam IsVSyncEnabled bool If the user has VSync Enabled
* @EventParam FrameRateLimit float User's Frame Rate Limit Setting. 0 = disabled.
* @EventParam OverallScalabilityLevel int32 User's OverallScalabilityLevel Setting
* @EventParam ViewDistanceQuality int32 User's ViewDistanceQuality Setting
* @EventParam ShadowQuality int32 User's ShadowQuality Setting
* @EventParam AntiAliasingQuality int32 User's AntiAliasingQuality Setting
* @EventParam TextureQuality int32 User's TextureQuality Setting
* @EventParam VisualEffectQuality int32 User's VisualEffectQuality Setting
* @EventParam PostProcessingQuality int32 User's PostProcessingQuality Setting
* @EventParam FoliageQuality int32 User's FoliageQuality Setting
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTGraphicsSettings(AUTPlayerController* UTPC)
{
	UUTGameUserSettings* UserSettings = Cast<UUTGameUserSettings>(GEngine->GetGameUserSettings());

	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (UserSettings && UTPC && AnalyticsProvider.IsValid())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetClientInitialParameters(UTPC, ParamArray, false);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::AAMode), UserSettings->GetAAMode()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ScreenPercentage), UserSettings->GetScreenPercentage()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::IsHRTFEnabled), UserSettings->IsHRTFEnabled()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::IsKeyboardLightingEnabled), UserSettings->IsKeyboardLightingEnabled()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ScreenResolution), UserSettings->GetScreenResolution().ToString()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::DesktopResolution), UserSettings->GetDesktopResolution().ToString()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::FullscreenMode), static_cast<int32>(UserSettings->GetFullscreenMode())));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::IsVSyncEnabled), UserSettings->IsVSyncEnabled()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::FrameRateLimit), UserSettings->GetFrameRateLimit()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::OverallScalabilityLevel), UserSettings->GetOverallScalabilityLevel()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ViewDistanceQuality), UserSettings->GetViewDistanceQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::ShadowQuality), UserSettings->GetShadowQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::AntiAliasingQuality), UserSettings->GetAntiAliasingQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::TextureQuality), UserSettings->GetTextureQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::VisualEffectQuality), UserSettings->GetVisualEffectQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PostProcessingQuality), UserSettings->GetPostProcessingQuality()));
		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::FoliageQuality), UserSettings->GetFoliageQuality()));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTGraphicsSettings), ParamArray);
	}
}

/*
* @EventName UTStartRankedMatch
*
* @Trigger Fires when a server beings a ranked match
*
* @Type Sent by the Server
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTStartRankedMatch(AUTGameMode* UTGM)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid() && UTGM && UTGM->GetWorld())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetMatchInitialParameters(UTGM, ParamArray, false, true);

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTStartRankedMatch), ParamArray);
	}
}

/*
* @EventName UTEndRankedMatch
*
* @Trigger Fires when a server ends a ranked match
*
* @Type Sent by the Server
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTEndRankedMatch(AUTGameMode* UTGM)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid() && UTGM && UTGM->GetWorld())
	{
		TArray<FAnalyticsEventAttribute> ParamArray;
		
		SetMatchInitialParameters(UTGM, ParamArray, false, true);

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTEndRankedMatch), ParamArray);
	}
}

/*
* @EventName UTServerPlayerJoin
*
* @Trigger Fires when the server has a player connect
*
* @Type Sent by the Server
*
* @EventParam PlayerGUID string GUID to identify the player
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTServerPlayerJoin(AUTGameMode* UTGM, AUTPlayerState* UTPS)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid() && UTGM && UTGM->GetWorld() && UTPS)
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetMatchInitialParameters(UTGM, ParamArray, false, true);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPS)));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTServerPlayerJoin), ParamArray);
	}
}

/*
* @EventName UTServerPlayerDisconnect
*
* @Trigger Fires when the server has a player disconnect
*
* @Type Sent by the Server
*
* @EventParam PlayerGUID string GUID to identify the player
*
* @Comments
*/
void FUTAnalytics::FireEvent_UTServerPlayerDisconnect(AUTGameMode* UTGM, AUTPlayerState* UTPS)
{
	const TSharedPtr<IAnalyticsProvider>& AnalyticsProvider = GetProviderPtr();
	if (AnalyticsProvider.IsValid() && UTGM && UTGM->GetWorld() && UTPS)
	{
		TArray<FAnalyticsEventAttribute> ParamArray;

		SetMatchInitialParameters(UTGM, ParamArray, false, true);

		ParamArray.Add(FAnalyticsEventAttribute(GetGenericParamName(EGenericAnalyticParam::PlayerGUID), GetEpicAccountName(UTPS)));

		AnalyticsProvider->RecordEvent(GetGenericParamName(EGenericAnalyticParam::UTServerPlayerDisconnect), ParamArray);
	}
}