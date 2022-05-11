// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "DefaultSegmentationComponent.h"
#include "SkeletalSegmentationSceneProxy.h"
#include "StaticSegmentationSceneProxy.h"
#include <fstream>
#include <sstream>
#include "Components/SceneCaptureComponent2D.h"
#include "Camera/CameraComponent.h"
#include "ConstructorHelpers.h"
#include "EngineUtils.h"
#include "StopTime.h"
#include "Server.h"
#include "PacketBuffer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <cmath>
#include <condition_variable>

UDefaultSegmentationComponent::UDefaultSegmentationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FString MaterialPath = TEXT("Material'/AutonomousRGBDCamera/AnnotationColor.AnnotationColor'");
	static ConstructorHelpers::FObjectFinder<UMaterial> SegmentationMaterialObject(*MaterialPath);
	SegmentationMaterial = SegmentationMaterialObject.Object;
	this->PrimaryComponentTick.bCanEverTick = true;
}

void UDefaultSegmentationComponent::OnRegister()
{
	Super::OnRegister();
	SegmentationMID = UMaterialInstanceDynamic::Create(SegmentationMaterial, this, TEXT("AnnotationMaterialMID"));
	if (!IsValid(SegmentationMID))
	{
		OUT_INFO( TEXT("SegmentationMaterial is not correctly initialized"));
		return;
	}
	const float OneOver255 = 1.0f / 255.0f;
	FLinearColor LinearSegmentationColor = FLinearColor(
		this->SegmentationColor.R * OneOver255,
		this->SegmentationColor.G * OneOver255,
		this->SegmentationColor.B * OneOver255,
		1.0
	);
	SegmentationMID->SetVectorParameterValue("AnnotationColor", LinearSegmentationColor);
}


void UDefaultSegmentationComponent::SetSegmentationColor(FColor NewSegmentationColor)
{
	this->SegmentationColor = NewSegmentationColor;
	const float OneOver255 = 1.0f / 255.0f;
	FLinearColor LinearSegmentationColor = FLinearColor(
		SegmentationColor.R * OneOver255,
		SegmentationColor.G * OneOver255,
		SegmentationColor.B * OneOver255,
		1.0
	);

	if (IsValid(SegmentationMID))
	{
		SegmentationMID->SetVectorParameterValue("AnnotationColor", LinearSegmentationColor);
	}
}

FColor UDefaultSegmentationComponent::GetSegmentationColor()
{
	return SegmentationColor;
}

FPrimitiveSceneProxy* UDefaultSegmentationComponent::CreateSceneProxy(UStaticMeshComponent* StaticMeshComponent)
{
	UMaterialInterface* ProxyMaterial = SegmentationMID;
	UStaticMesh* ParentStaticMesh = StaticMeshComponent->GetStaticMesh();
	if(ParentStaticMesh == NULL
		|| ParentStaticMesh->RenderData == NULL
		|| ParentStaticMesh->RenderData->LODResources.Num() == 0)
	{
		OUT_INFO(TEXT("ParentStaticMesh is invalid."));
		return NULL;
	}
	FPrimitiveSceneProxy* Proxy = ::new FStaticSegmentationSceneProxy(StaticMeshComponent, false, ProxyMaterial);
	return Proxy;
}


FPrimitiveSceneProxy* UDefaultSegmentationComponent::CreateSceneProxy(USkeletalMeshComponent* SkeletalMeshComponent)
{
	UMaterialInterface* ProxyMaterial = SegmentationMID;
	ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->FeatureLevel;
	FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
	if (SkelMeshRenderData &&
		SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
		SkeletalMeshComponent->MeshObject) 
	{
		return new FSkeletalSegmentationSceneProxy(SkeletalMeshComponent, SkelMeshRenderData, ProxyMaterial);
	}
	else
	{
	        OUT_INFO(TEXT("The data of SkeletalMeshComponent %s is invalid."), *SkeletalMeshComponent->GetName());
		return nullptr;
	}
}

FPrimitiveSceneProxy* UDefaultSegmentationComponent::CreateSceneProxy()
{
	USceneComponent* ParentComponent = this->GetAttachParent();
	if (!IsValid(ParentComponent))
	{
		OUT_INFO(TEXT("Parent component is invalid."));
		return nullptr;
	}
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
	if (IsValid(StaticMeshComponent))
	{
		return CreateSceneProxy(StaticMeshComponent);
	}
	else if (IsValid(SkeletalMeshComponent))
	{
		return CreateSceneProxy(SkeletalMeshComponent);
	}
	else
	{
		OUT_INFO(TEXT("The type of ParentMeshComponent : %s can not be supported."), *ParentComponent->GetClass()->GetName());
		return nullptr;
	}
	return nullptr;
}

FBoxSphereBounds UDefaultSegmentationComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	USceneComponent* Parent = this->GetAttachParent();
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Parent);
	if (IsValid(StaticMeshComponent))
	{
		return StaticMeshComponent->CalcBounds(LocalToWorld);
	}

	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Parent);
	if (IsValid(SkeletalMeshComponent))
	{
		return SkeletalMeshComponent->CalcBounds(LocalToWorld);
	}

	FBoxSphereBounds DefaultBounds;
	return DefaultBounds;
}