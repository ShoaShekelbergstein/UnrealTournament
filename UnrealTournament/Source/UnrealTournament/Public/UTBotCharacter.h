// editor-creatable bot character data
// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTBotCharacter.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTBotCharacter : public UDataAsset
{
	GENERATED_UCLASS_BODY()
public:
	/** if set a UTProfileItem is required for this character to be available */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (DisplayName = "Requires Online Item"))
	bool bRequiresItem;

	/** Modifier to base game difficulty for this bot. */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (ClampMin = "-1.0", UIMin = "1.0", ClampMax = "1.5", UIMax = "1.5"), Category = Skill)
		float SkillAdjust;

	/** Minimum skill rating this bot will appear at. */
	UPROPERTY(EditAnywhere, AssetRegistrySearchable, Meta = (ClampMin = "0", UIMin = "0", ClampMax = "7.0", UIMax = "7.0"), Category = Skill)
		float MinimumSkill;

	/** bot personality attributes affecting behavior */
	UPROPERTY(EditAnywhere, Category = Skill)
	FBotPersonality Personality;
	
	/** character content (primary mesh/material) */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTCharacterContent"), Category = Cosmetics)
	FStringClassReference Character;

	/** hat to use */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTHat"), Category=Cosmetics)
	FStringClassReference HatType;
	/** optional hat variant ID */
	UPROPERTY(EditAnywhere)
	int32 HatVariantId;
	/** eyewear to use */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTEyewear"), Category = Cosmetics)
	FStringClassReference EyewearType;
	/** optional eyewear variant ID */
	UPROPERTY(EditAnywhere)
	int32 EyewearVariantId;

	/* Voice associated with this character. */
	UPROPERTY(EditAnywhere, Meta = (MetaClass = "UTCharacterVoice"), Category = Cosmetics)
	FStringClassReference CharacterVoice;

	UPROPERTY(EditAnywhere, Category = Movement)
		bool bCanDodge;

	UPROPERTY(EditAnywhere, Category = Movement)
		float DodgeResetInterval;

	UPROPERTY(EditAnywhere, Category = Movement)
		int32 MaxMultiJumpCount;

	UPROPERTY(EditAnywhere, Category = Movement)
		float MultiJumpImpulse;

	UPROPERTY(EditAnywhere, Category = Movement)
		float MaxRunSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
		float AirControl;

	UPROPERTY(EditAnywhere, Category = Movement)
		float JumpImpulse;

};

/*
Add default inventory modifiers
Health, armor
Ultimate power

MaxAdditiveDodgeJumpSpeed = 700.f;
DodgeJumpImpulse = 600.f;
MaxSlideSpeed = 1230.f;

MaxFloorSlideSpeed = 900.f;
MaxInitialFloorSlideSpeed = 1350.f;
FloorSlideDuration = 0.7f;
FloorSlideEndingSpeedFactor = 0.4f;
FloorSlideSlopeBraking = 2.7f;

MaxMultiJumpZSpeed = 280.f;
DodgeImpulseHorizontal = 1500.f;
DodgeMaxHorizontalVelocity = 1700.f;
WallDodgeSecondImpulseVertical = 320.f;
DodgeImpulseVertical = 500.f;
WallDodgeImpulseHorizontal = 1300.f;
WallDodgeImpulseVertical = 470.f;

MaxSlideRiseZ = 650.f;
MaxSlideFallZ = -120.f;
SlideGravityScaling = 0.08f;
MinWallSlideSpeed = 500.f;
MaxSlideWallDist = 20.f;
*/