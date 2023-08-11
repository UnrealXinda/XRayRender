// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RayTracingPayloadType.h"
#include "ShaderParameterStruct.h"

class XRAYSHADERS_API FXRayMainRG : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FXRayMainRG);
	SHADER_USE_ROOT_PARAMETER_STRUCT(FXRayMainRG, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FMatrix44f, ClipToTranslatedWorld)
		SHADER_PARAMETER(FMatrix44f, ClipToView)
		SHADER_PARAMETER(float, NearPlane)
		SHADER_PARAMETER(float, MinAttenuation)
		SHADER_PARAMETER(float, MaxAttenuation)
		SHADER_PARAMETER(float, ExpAttenuation)

		SHADER_PARAMETER_SRV(RaytracingAccelerationStructure, TLAS)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, AttenuationTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
	}

	static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
	{
		return ERayTracingPayloadType::Minimal;
	}
};

class XRAYSHADERS_API FXRayMainCHS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FXRayMainCHS);
	SHADER_USE_ROOT_PARAMETER_STRUCT(FXRayMainCHS, FGlobalShader)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
	}

	static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
	{
		return ERayTracingPayloadType::Minimal;
	}

	using FParameters = FEmptyShaderParameters;
};

class XRAYSHADERS_API FXRayMainAHS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FXRayMainAHS);
	SHADER_USE_ROOT_PARAMETER_STRUCT(FXRayMainAHS, FGlobalShader)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return RHISupportsRayTracingShaders(Parameters.Platform);
	}

	static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
	{
		return ERayTracingPayloadType::Minimal;
	}

	using FParameters = FEmptyShaderParameters;
};

class XRAYSHADERS_API FXRayMainMS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FXRayMainMS);
	SHADER_USE_ROOT_PARAMETER_STRUCT(FXRayMainMS, FGlobalShader)

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return RHISupportsRayTracingShaders(Parameters.Platform);
	}

	static ERayTracingPayloadType GetRayTracingPayloadType(const int32 PermutationId)
	{
		return ERayTracingPayloadType::Minimal;
	}

	using FParameters = FEmptyShaderParameters;
};