// Fill out your copyright notice in the Description page of Project Settings.

#include "XRayViewExtension.h"

#include "CopyTexturePS.h"
#include "ScenePrivate.h"
#include "XRayRenderSettings.h"
#include "XRayRTShaders.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Renderer/Private/ScreenPass.h"

namespace
{
	FRHITexture* GetRHITexture(const UTextureRenderTarget2D* InTargetTexture)
	{
		FRHITexture* RHITexture = nullptr;

		if (IsValid(InTargetTexture))
		{
			const FTextureReferenceRHIRef TargetTextureRHI = InTargetTexture->TextureReference.TextureReferenceRHI;
			if (TargetTextureRHI.IsValid())
			{
				if (const FRHITextureReference* TextureReference = TargetTextureRHI->GetTextureReference())
				{
					RHITexture = TextureReference->GetReferencedTexture();
				}
			}
		}

		return RHITexture;
	}
}

FXRayViewExtension& FXRayViewExtension::Get()
{
	static TSharedPtr<FXRayViewExtension> Instance;

	if (!Instance.IsValid())
	{
		Instance = FSceneViewExtensions::NewExtension<FXRayViewExtension>();
	}

	return *Instance;
}

FXRayViewExtension::FXRayViewExtension(const FAutoRegister& AutoRegister) :
	FSceneViewExtensionBase{AutoRegister},
	CachedRenderTargetRHI(nullptr),
	XRayRenderSettings()
{
}

FXRayViewExtension::~FXRayViewExtension()
{
}

bool FXRayViewExtension::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	check((IsInGameThread()));

	const UXRayRenderSettings* Settings = GetDefault<UXRayRenderSettings>();
	check(IsValid(Settings));

	if (!Settings->bEnabled)
	{
		return false;
	}

	const UWorld* World = Context.GetWorld();
	const bool bIsEditorOrGameView = IsValid(World) && (World->IsGameWorld() || World->WorldType == EWorldType::Editor);
	if (!bIsEditorOrGameView)
	{
		return false;
	}

	const UTextureRenderTarget2D* RenderTarget = Cast<UTextureRenderTarget2D>(Settings->OptionalRenderTarget.TryLoad());
	if (!IsValid(RenderTarget))
	{
		return false;
	}

	CachedRenderTargetRHI = GetRHITexture(RenderTarget);
	if (!CachedRenderTargetRHI)
	{
		return false;
	}

	// Render target may not be ready upon engine start
	const ETextureCreateFlags RenderTargetFlags = CachedRenderTargetRHI->GetDesc().Flags;
	if (!EnumHasAnyFlags(RenderTargetFlags, TexCreate_RenderTargetable | TexCreate_ResolveTargetable))
	{
		return false;
	}

	XRayRenderSettings.MinAttenuation = Settings->MinAttenuation;
	XRayRenderSettings.MaxAttenuation = Settings->MaxAttenuation;
	XRayRenderSettings.ExpAttenuation = Settings->ExpAttenuation;

	return true;
}

void FXRayViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View,
	const FPostProcessingInputs& Inputs)
{
	check(View.bIsViewInfo);
	check(CachedRenderTargetRHI);

	const FViewInfo& ViewInfo = StaticCast<const FViewInfo&>(View);
	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	FSceneInterface* SceneInterface = ViewInfo.Family->Scene;
	if (!SceneInterface)
	{
		return;
	}

	FScene* Scene = SceneInterface->GetRenderScene();
	if (!Scene)
	{
		return;
	}

	FRHIRayTracingScene* RayTracingScene = Scene->RayTracingScene.GetRHIRayTracingScene();
	ensureMsgf(RayTracingScene, TEXT("Ray tracing scene is expected to be created at this point."));
	if (!RayTracingScene)
	{
		return;
	}

	const int32 ViewWidth = ViewInfo.ViewRect.Width();
	const int32 ViewHeight = ViewInfo.ViewRect.Height();
	const FIntPoint TextureSize(ViewWidth, ViewHeight);
	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		TextureSize,
		PF_R16F,
		FClearValueBinding::None,
		TexCreate_ShaderResource | TexCreate_UAV);

	// Step 1: Create X-Ray texture
	const TShaderMapRef<FXRayMainRG> RayGenShader{GlobalShaderMap};
	const TShaderMapRef<FXRayMainAHS> AnyHitShader{GlobalShaderMap};
	const TShaderMapRef<FXRayMainMS> MissShader{GlobalShaderMap};

	const FViewMatrices& ViewMatrices = ViewInfo.ViewMatrices;
	FRHIShaderResourceView* TLAS = Scene->RayTracingScene.GetLayerSRVChecked(ERayTracingSceneLayer::Base);
	FRDGTextureRef XRayTexture = GraphBuilder.CreateTexture(Desc, TEXT("XRayTexture"));
	FRDGTextureUAVRef XRayTextureUAV = GraphBuilder.CreateUAV(XRayTexture);

	{
		FXRayMainRG::FParameters* Parameters = GraphBuilder.AllocParameters<FXRayMainRG::FParameters>();
		Parameters->TLAS                        = TLAS;
		Parameters->AttenuationTexture          = XRayTextureUAV;
		Parameters->ClipToView                  = FMatrix44f(ViewMatrices.GetInvProjectionMatrix());
		Parameters->ClipToTranslatedWorld       = FMatrix44f(ViewMatrices.GetInvTranslatedViewProjectionMatrix());
		Parameters->NearPlane                   = ViewMatrices.ComputeNearPlane();
		Parameters->MinAttenuation              = XRayRenderSettings.MinAttenuation;
		Parameters->MaxAttenuation              = XRayRenderSettings.MaxAttenuation;
		Parameters->ExpAttenuation              = XRayRenderSettings.ExpAttenuation;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("XRayRendering"),
			Parameters,
			ERDGPassFlags::Compute,
			[RayGenShader, AnyHitShader, MissShader, Parameters, ViewWidth, ViewHeight, RayTracingScene, this](FRHICommandList& RHICmdList)
			{
				FRHIRayTracingShader* RayGenShaderTable[] = { RayGenShader.GetRayTracingShader() };
				FRHIRayTracingShader* HitGroupTable[]     = { AnyHitShader.GetRayTracingShader() };
				FRHIRayTracingShader* MissShaderTable[]   = { MissShader.GetRayTracingShader() };

				FRayTracingPipelineStateInitializer Initializer{};
				Initializer.MaxPayloadSizeInBytes = GetRayTracingPayloadTypeMaxSize(ERayTracingPayloadType::Minimal);
				Initializer.SetRayGenShaderTable(RayGenShaderTable);
				Initializer.SetHitGroupTable(HitGroupTable);
				Initializer.SetMissShaderTable(MissShaderTable);
				Initializer.bAllowHitGroupIndexing = false; // Use the same hit shader for all geometry in the scene by disabling SBT indexing.

				FRayTracingPipelineState* Pipeline = PipelineStateCache::GetAndOrCreateRayTracingPipelineState(RHICmdList, Initializer);
				RHICmdList.SetRayTracingMissShader(RayTracingScene, 0, Pipeline, 0 /* ShaderIndexInPipeline */, 0, nullptr, 0);

				FRayTracingShaderBindingsWriter GlobalResources{};
				SetShaderParameters(GlobalResources, RayGenShader, *Parameters);

				RHICmdList.RayTraceDispatch(
					Pipeline,
					RayGenShader.GetRayTracingShader(),
					RayTracingScene,
					GlobalResources,
					ViewWidth,
					ViewHeight);
			});
	}

	// Step 2: Copy X-Ray texture to final output
	{
		const FRDGTextureRef RenderTargetRDG = RegisterExternalTexture(GraphBuilder, CachedRenderTargetRHI, TEXT("XRayRenderTarget"));
		const FScreenPassTextureViewport InputViewport{XRayTexture};
		const FScreenPassTextureViewport OutputViewport{RenderTargetRDG};

		FCopyTexturePS::FParameters* Parameters = GraphBuilder.AllocParameters<FCopyTexturePS::FParameters>();
		Parameters->InputTexture = XRayTexture;
		Parameters->InputSampler = TStaticSamplerState<>::GetRHI();
		Parameters->RenderTargets[0] = FRenderTargetBinding{RenderTargetRDG, ERenderTargetLoadAction::ELoad};

		const TShaderMapRef<FCopyTexturePS> CopyTexturePS{GlobalShaderMap};

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("XRay_CopyToRenderTarget"),
			ViewInfo,
			OutputViewport,
			InputViewport,
			CopyTexturePS,
			Parameters);
	}
}