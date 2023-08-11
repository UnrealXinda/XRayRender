// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class XRayRender : ModuleRules
{
	public XRayRender(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(GetModuleDirectory("Renderer"), "Private"),
				System.IO.Path.Combine(GetModuleDirectory("RenderCore"), "Private"),
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"RenderCore",
				"Renderer",
				"RHI",
				"Projects",
				"XRayShaders",
			}
		);
	}
}