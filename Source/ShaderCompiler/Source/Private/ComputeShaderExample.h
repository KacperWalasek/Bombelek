#pragma once

#include "CoreMinimal.h"
#include "ShaderCompilerModule.h"

/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FComputeShaderExample
{
public:
	static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderCompilerParameters& DrawParameters, FUnorderedAccessViewRHIRef ComputeShaderOutputUAV);
};
