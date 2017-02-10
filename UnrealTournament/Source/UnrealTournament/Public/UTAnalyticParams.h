namespace EGenericAnalyticParam
{
	enum Type
	{
		PlayerGUID,
		PlayerList,
		ServerInstanceGUID,
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

		HitchThresholdInMs,
		NumHitchesAboveThreshold,
		TotalUnplayableTimeInMs,
		ServerUnplayableCondition,

		WeaponName,
		NumKills,
		UTServerWeaponKills,
		WeaponInfo,

		UTFPSCharts,
		UTServerFPSCharts,

		Team,
		MaxRequiredTextureSize,
		QuickMatch,

		FlagRunRoundEnd,
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

		UTEnterMatch,
		EnterMethod,
		UTStartRankedMatch,
		UTEndRankedMatch,
		ELOPlayerInfo,

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
		UTCancelOnboarding,
		TutorialMap,
		TokensCollected,
		TokensAvailable,
		MovementTutorialCompleted,
		WeaponTutorialCompleted,
		PickupsTutorialCompleted,
		RealServerFPS,

		NUM_GENERIC_PARAMS
	};
}