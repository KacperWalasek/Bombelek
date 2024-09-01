#pragma once

#include "CoreMinimal.h"
#include "ShaderCompilerModule.h"

/**************************************************************************************/
/* This is just an interface we use to keep all the pixel shading code in one file.   */
/**************************************************************************************/
class FPixelShaderExample
{
public:
	static void DrawToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderCompilerParameters& DrawParameters, FTextureRHIRef ComputeShaderOutput);
};
