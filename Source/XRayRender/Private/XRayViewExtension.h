// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"

struct FXRayRenderSettings
{
	float MinAttenuation;
	float MaxAttenuation;
	float ExpAttenuation;
};

class FXRayViewExtension final : public FSceneViewExtensionBase
{
public:
	static FXRayViewExtension& Get();

	FXRayViewExtension(const FAutoRegister& AutoRegister);
	virtual ~FXRayViewExtension() override;

	//~ ISceneViewExtension interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override {}
	virtual void PreRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override {}
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	//~ ISceneViewExtension interface

private:
	mutable FRHITexture* CachedRenderTargetRHI;
	mutable FXRayRenderSettings XRayRenderSettings;
};