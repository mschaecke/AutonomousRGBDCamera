/**
 * @file SceneObject.h
 *
 * @brief A single object within the scene
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 27.05.2021
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "SceneObject.generated.h"

UCLASS()
class AUTONOMOUSRGBDCAMERA_API ASceneObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor for SceneObject
	ASceneObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Get scene object ID
	int32 GetSceneObjectID();
	
	// Set scene object ID
	void SetSceneObjectID(int32 pSceneObjectID);

	// Set the mesh path
	void SetMeshPath(FString pMeshPath);

	// Create the mesh using MeshPath and StaticMeshComponent
	void CreateMesh();

	// Set the material path
	void SetMaterialPath(FString pMaterialPath);

	// Create the material using MaterialPath and StaticMeshComponent
	void CreateMaterial();


	// Root component
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;

	// Static mesh component
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	// Scene object ID
	UPROPERTY(EditAnywhere)
	int32 SceneObjectID;

	// Mesh path
	UPROPERTY(EditAnywhere)
	FString MeshPath;

	// Material path
	UPROPERTY(EditAnywhere)
	FString MaterialPath;
};