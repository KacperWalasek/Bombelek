#include "ShaderCompilerModule.h"

#include "ComputeShaderExample.h"
#include "PixelShaderExample.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

IMPLEMENT_MODULE(FShaderCompilerModule, ShaderCompiler)

// Declare some GPU stats so we can track them later
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Render, TEXT("ShaderPlugin: Root Render"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Compute, TEXT("ShaderPlugin: Render Compute Shader"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Pixel, TEXT("ShaderPlugin: Render Pixel Shader"));

void FShaderCompilerModule::StartupModule()
{
	OnPostResolvedSceneColorHandle.Reset();
	bCachedParametersValid = false;

	//auto var1 = IPluginManager::Get().FindPlugin(TEXT("ShaderCompiler"));
	//auto var2 = var1->GetBaseDir();
	//FString PluginShaderDir = FPaths::Combine(var2, TEXT("Shaders"));
	//AddShaderSourceDirectoryMapping(TEXT("/ShaderCompiler"), PluginShaderDir);
}

void FShaderCompilerModule::ShutdownModule()
{
	EndRendering();
}

void FShaderCompilerModule::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	bCachedParametersValid = false;

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FShaderCompilerModule::PostResolveSceneColor_RenderThread);
	}
}

void FShaderCompilerModule::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FShaderCompilerModule::UpdateParameters(FShaderCompilerParameters& DrawParameters)
{
	RenderEveryFrameLock.Lock();
	CachedShaderUsageExampleParameters = DrawParameters;
	bCachedParametersValid = true;
	RenderEveryFrameLock.Unlock();
}

void FShaderCompilerModule::PostResolveSceneColor_RenderThread(FRDGBuilder& builder, const FSceneTextures& SceneTexture)
{
	if (!bCachedParametersValid)
	{
		return;
	}

	// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
	RenderEveryFrameLock.Lock();
	FShaderCompilerParameters Copy = CachedShaderUsageExampleParameters;
	RenderEveryFrameLock.Unlock();

	Draw_RenderThread(Copy);
}

void FShaderCompilerModule::Draw_RenderThread(const FShaderCompilerParameters& DrawParameters)
{
	check(IsInRenderingThread());

	if (!DrawParameters.RenderTarget)
	{
		return;
	}

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_Render); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Render); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	if (!ComputeShaderOutput.IsValid())
	{
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(DrawParameters.GetRenderTargetSize(), PF_R32_UINT, FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		ComputeShaderOutputDesc.DebugName = TEXT("ShaderPlugin_ComputeShaderOutput");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("ShaderPlugin_ComputeShaderOutput"));
	}

	FComputeShaderExample::RunComputeShader_RenderThread(RHICmdList, DrawParameters, RHICmdList.CreateUnorderedAccessView(ComputeShaderOutput->GetRHI()));
	FPixelShaderExample::DrawToRenderTarget_RenderThread(RHICmdList, DrawParameters, ComputeShaderOutput->GetRHI());
}
