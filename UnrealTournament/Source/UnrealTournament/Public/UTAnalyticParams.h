namespace EGenericAnalyticParam
{
	enum Type
	{
		UniqueAnalyticSessionGuid,

		PlayerGUID,
		PlayerList,
		PlayerKills,
		PlayerDeaths,
		BotList,
		BotSkill,
		InactivePlayerList,
		ServerInstanceGUID,
		MatchReplayGUID,
		ServerMatchGUID,
		ContextGUID,
		MatchTime,
		MapName,
		GameModeName,

		Hostname,
		SystemId,
		InstanceId,
		BuildVersion,
		MachinePhysicalRAMInGB,
		NumLogicalCoresAvailable,
		NumPhysicalCoresAvailable,
		MachineCPUSignature,
		MachineCPUBrandName,
		NumClients,

		Platform,
		Location,
		SocialPartyCount,
		IsIdle,
		RegionId,
		PlayerContextLocationPerMinute,

		PlaylistId,
		bRanked,
		TeamElo,
		EloRange,
		SeekTime,

		UTMatchMakingStart,
		UTMatchMakingCancelled,
		UTMatchMakingJoinGame,
		UTMatchMakingFailed,
		LastMatchMakingSessionId,

		HitchThresholdInMs,
		NumHitchesAboveThreshold,
		TotalUnplayableTimeInMs,
		ServerUnplayableCondition,

		WeaponName,
		NumKills,
		UTServerWeaponKills,
		WeaponInfo,

		//NumKills above as part of weapon analytics
		NumDeaths,
		NumAssists,
		
		ApplicationStart,
		ApplicationStop,

		UTFPSCharts,
		UTServerFPSCharts,

		Team,
		MaxRequiredTextureSize,
		QuickMatch,

		FlagRunRoundEnd,
		PlayerUsedRally,
		RallyPointBeginActivate,
		RallyPointCompleteActivate,
		OffenseKills,
		DefenseKills,
		DefenseLivesRemaining,
		DefensePlayersEliminated,
		PointsScored,
		DefenseWin,
		TimeRemaining,
		RoundNumber,
		FinalRound,
		EndedInTieBreaker,
		RedTeamBonusTime,
		BlueTeamBonusTime,
		WinningTeamNum,

		UTEnterMatch,
		EnterMethod,
		UTStartMatch,
		UTInitContext,
		UTInitMatch,
		UTEndMatch,
		ELOPlayerInfo,

		bIsMenu,
		bIsOnline,
		bIsRanked,
		bIsQuickMatch,

		Reason,

		UTTutorialPickupToken,
		TokenID,
		TokenDescription,
		HasTokenBeenPickedUpBefore,
		AnnouncementName,
		AnnouncementID,
		OptionalObjectName,
		UTTutorialPlayInstruction,

		UTTutorialStarted,
		UTTutorialCompleted,
		UTTutorialQuit,
		UTCancelOnboarding,
		TutorialMap,
		TokensCollected,
		TokensAvailable,
		MovementTutorialCompleted,
		WeaponTutorialCompleted,
		PickupsTutorialCompleted,
		DMTutorialCompleted,
		TDMTutorialCompleted,
		CTFTutorialCompleted,
		DuelTutorialCompleted,
		FlagRunTutorialCompleted,
		ShowdownTutorialCompleted,
		
		RealServerFPS,

		UTServerPlayerJoin,
		UTServerPlayerDisconnect,

		UTGraphicsSettings,
		AAMode,
		ScreenPercentage,
		IsHRTFEnabled,
		IsKeyboardLightingEnabled,
		ScreenResolution,
		DesktopResolution,
		FullscreenMode,
		IsVSyncEnabled,
		FrameRateLimit,
		OverallScalabilityLevel,
		ViewDistanceQuality,
		ShadowQuality,
		AntiAliasingQuality,
		TextureQuality,
		VisualEffectQuality,
		PostProcessingQuality,
		FoliageQuality,

		ServerName,
		ServerID,
		IsCustomRuleset,
		GameOptions,
		RequiredPackages,
		CurrentGameState,
		IsSpectator,
		UTHubBootUp,
		UTHubNewInstance,
		UTHubInstanceClosing,
		UTHubPlayerJoinLobby,
		UTHubPlayerEnterInstance,

		NUM_GENERIC_PARAMS
	};
}