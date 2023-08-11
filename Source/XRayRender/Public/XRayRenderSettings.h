// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XRayRenderSettings.generated.h"

class UTextureRenderTarget2D;

UCLASS(Config=XRayRenderSettings, defaultconfig)
class XRAYRENDER_API UXRayRenderSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Config)
	bool bEnabled;

	UPROPERTY(EditDefaultsOnly, Config, meta=(ClampMin = 0.0))
	float MinAttenuation = 0.001f;

	UPROPERTY(EditDefaultsOnly, Config, meta=(ClampMin = 0.0))
	float MaxAttenuation = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Config, meta=(ClampMin = 0.0))
	float ExpAttenuation = 1.0f;

	// The optional render target that the X-Ray attenuation texture is copied back into
	// This can be used by post process materials for custom blending
	UPROPERTY(EditDefaultsOnly, Config, meta=(AllowedClasses="TextureRenderTarget2D"))
	FSoftObjectPath OptionalRenderTarget;
};