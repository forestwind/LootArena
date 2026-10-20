// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LootInventory : ModuleRules
{
	public LootInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"DeveloperSettings",
				"UMG",
				"Slate",
				"SlateCore",
			}
			);
		
		// 에디터 빌드일 때만 추가 (쿡 목록 검사용) 
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("DeveloperToolSettings");
		}
	}
}
