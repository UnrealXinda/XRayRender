// Fill out your copyright notice in the Description page of Project Settings.

#include "XRayRTShaders.h"

IMPLEMENT_GLOBAL_SHADER(FXRayMainRG, "/Plugin/XRayRender/XRayRTShaders.usf", "XRayMainRG", SF_RayGen);
IMPLEMENT_GLOBAL_SHADER(FXRayMainCHS, "/Plugin/XRayRender/XRayRTShaders.usf", "XRayMainCHS", SF_RayHitGroup);
IMPLEMENT_GLOBAL_SHADER(FXRayMainAHS, "/Plugin/XRayRender/XRayRTShaders.usf", "closesthit=XRayMainCHS anyhit=XRayMainAHS", SF_RayHitGroup);
IMPLEMENT_GLOBAL_SHADER(FXRayMainMS,  "/Plugin/XRayRender/XRayRTShaders.usf", "XRayMainMS", SF_RayMiss);