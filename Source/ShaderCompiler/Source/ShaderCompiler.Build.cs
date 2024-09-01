// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ShaderCompiler : ModuleRules
{
	public ShaderCompiler(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(new string[] 
		{
			"ShaderCompiler/Private"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects"
		});
	}
}
