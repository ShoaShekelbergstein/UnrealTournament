// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonUI : ModuleRules
{
	public CommonUI(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
				"InputCore",
				"Slate",
                "UMG",
                "WidgetCarousel",
                "BlueprintContext"
			}
		);

		PrivateDependencyModuleNames.AddRange(
		new string[]
			{
				"SlateCore",
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
                "CommonUI/Private",
			}
		);

		PublicIncludePaths.AddRange(
			new string[]
			{
                "CommonUI/Public",
			}
		);
	}
}
