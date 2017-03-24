// editor-creatable bot character data
// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTBotCharacter.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTBotCharacter : public UDataAsset
{
	GENERATED_BODY()
public:
	/** if set a UTProfileItem is required for this character to be available */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (DisplayName = "Requires Online Item"))
	bool bRequiresItem;

	/** Modifier to base game difficulty for this bot. */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (ClampMin = "-1.0", UIMin = "1.0", ClampMax = "1.5", UIMax = "1.5"))
		float SkillAdjust;

	/** Minimum skill rating this bot will appear at. */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (ClampMin = "0", UIMin = "0", ClampMax = "7.0", UIMax = "7.0"))
		float MinimumSkill;

	/** bot personality attributes affecting behavior */
	UPROPERTY(EditAnywhere)
	FBotPersonality Personality;
	
	/** character content (primary mesh/material) */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTCharacterContent"))
	FStringClassReference Character;

	/** hat to use */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTHat"))
	FStringClassReference HatType;
	/** optional hat variant ID */
	UPROPERTY(EditAnywhere)
	int32 HatVariantId;
	/** eyewear to use */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTEyewear"))
	FStringClassReference EyewearType;
	/** optional eyewear variant ID */
	UPROPERTY(EditAnywhere)
	int32 EyewearVariantId;

	/* Voice associated with this character. */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTCharacterVoice"))
	FStringClassReference CharacterVoice;
};