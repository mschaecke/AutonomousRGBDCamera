// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Public/SkeletalRenderPublic.h"
#include "DefaultSegmentationComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class AUTONOMOUSRGBDCAMERA_API UDefaultSegmentationComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	UDefaultSegmentationComponent(const FObjectInitializer& ObjectInitializer);
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;
	void SetSegmentationColor(FColor SegmentationColor);
	FColor GetSegmentationColor();
	virtual void OnRegister() override;

private:
	UPROPERTY()
	UMaterial* SegmentationMaterial;
	UPROPERTY()
	UMaterialInstanceDynamic* SegmentationMID;
	FColor SegmentationColor;
	FPrimitiveSceneProxy* CreateSceneProxy(UStaticMeshComponent* StaticMeshComponent);
	FPrimitiveSceneProxy* CreateSceneProxy(USkeletalMeshComponent* SkeletalMeshComponent);
};