// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTUITypes.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnItemClicked, UUserWidget*, Widget);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnItemSelected, UUserWidget*, Widget, bool, Selected);

/** Types of currency */
UENUM(BlueprintType)
enum class EUTCurrencyType : uint8
{
	TimeCurrency,
	MtxCurrency,
	RealMoney,

	MAX UMETA(Hidden)
};

/** The types of levels that get tracked */
UENUM(BlueprintType)
enum class EUTLevelTypes : uint8
{
	Account,
	Weapon,
	WeaponSkin,

	MAX UMETA(Hidden)
};

/** The different style sizes that can be applied to various UT widgets */
UENUM(BlueprintType)
namespace EUTWidgetStyleSize
{
	enum Type
	{
		Small,
		Medium,
		Large,
		MAX UMETA(Hidden)
	};
}

/** The colors that an UT text style supports */
UENUM(BlueprintType)
namespace EUTTextColor
{
	enum Type
	{
		Light,
		Dark,
		Emphasis,
		Black,
		// USE THIS SPARINGLY! Only for special occasions.
		Custom,
		MAX UMETA(Hidden)
	};
}

DECLARE_DELEGATE(FUTLootRewardDismissed);

/** loot notification */
USTRUCT()
struct FUtLootNotification
{
	GENERATED_BODY()

	static const FString TypeStr;

	UPROPERTY()
	FString LootSource;

	/** unique id of the source asset (item instance or specific mission/campaign). Interpretation depends on LootSource */
	UPROPERTY()
	FString LootSourceInstance;

	/** the actual loot result */
	UPROPERTY()
	FMcpLootResult LootGranted;

	/** Used for daily, hero or account leveling rewards */
	UPROPERTY()
	int Level; 
};

/** Describes a loot reward */
USTRUCT(BlueprintType)
struct UNREALTOURNAMENT_API FUTLootReward
{
	GENERATED_BODY()

public:

	/** The definition of the granted item */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	class UUtMcpDefinition* ItemDefinition;

	/** The number of items granted */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	int32 Quantity;

	/** The source of the reward */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FName SourceName;

	/** The level associated with the reward, if any. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	int32 Level;

	/** The loot group this reward is from, if any. (random rewards only) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FString LootGroupName;

	/** True if this reward is MTX (needed because mtx is literally just a number) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bIsMtxReward;

	/** True if this reward is Time Currency */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bIsTimeCurrencyReward;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bIsNegative;

	FUTLootRewardDismissed OnDismissed;

	//@todo: We need reward quality levels!!

	FUTLootReward()
		: ItemDefinition(nullptr)
		, Quantity(0)
		, Level(0)
		, LootGroupName()
		, bIsMtxReward(0)
	{}

	FUTLootReward(const FUtLootNotification& LootNotification, const FMcpLootEntry& LootEntry);

	bool IsValid() const;
	bool IsRandomReward() const;
};