namespace EGenericAnalyticParam
{
	enum Type
	{
		PlayerGUID,
		ServerInstanceGUID,
		ServerMatchGUID,
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

		UTTutorialPickupToken,
		TokenID,
		AnnouncementID,
		OptionalObjectName,
		UTTutorialPlayInstruction,

		UTTutorialStarted,
		UTTutorialCompleted,
		TutorialMap,
		MovementTutorialCompleted,
		WeaponTutorialCompleted,
		PickupsTutorialCompleted,
		RealServerFPS,

		NUM_GENERIC_PARAMS
	};
}