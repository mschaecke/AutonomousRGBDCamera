/**
 * @file SceneConfiguration.h
 *
 * @brief A configuration of scene objects
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 29.05.2021
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/List.h"
#include "SceneObject.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "SceneConfiguration.generated.h"

UCLASS()
class AUTONOMOUSRGBDCAMERA_API ASceneConfiguration : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor for SceneConfiguration
	ASceneConfiguration();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Load the JSON file from the project's config directory
	void LoadJsonFile();

	// Spawn the scene objects and initialize spatial relationships
	void UseJsonFileContent();

	// Enable all scene objects
	void EnableAllSceneObjects();

	// Enable scene object
	void EnableSceneObject(ASceneObject* pSceneObject);

	// Randomly swap scene objects
	void RandomlySwapSceneObjects();

	// Swap scene objects using the argument, a random swap partner and new rotations for both
	void SwapSceneObjects(ASceneObject* pSceneObject);

	// Get a random partner for SwapSceneObjects
	ASceneObject* GetSwapPartner(ASceneObject* pSceneObject);

	// Set MaxZRotationSceneObject
	void SetMaxZRotationSceneObject(float pMaxZRotationSceneObject);

	// Randomly disable scene objects
	void RandomlyDisableSceneObjects();

	// Disable scene object
	void DisableSceneObject(ASceneObject* pSceneObject);

	// Enable physics for all scene objects
	void EnablePhysicsSceneObjects();

	// Disable physics for all scene objects
	void DisablePhysicsSceneObjects();

	// Update the spatial relationships of scene objects after swapping
	void UpdateSpatialRelationshipSceneObjects(ASceneObject* pSceneObject1, ASceneObject* pSceneObject2);

	// Set bShowSpatialRelationships
	void SetBShowSpatialRelationships(bool pBShowSpatialRelationships);

	// Show spatial relationships of scene objects in Output Log
	void ShowSpatialRelationships();

	// Check if a scene object is contained by another scene object
	bool CheckIsContained(ASceneObject* pSceneObject);

	// Move objects contained by pSceneObject to the new location
	void MoveContainedObjects(ASceneObject* pSceneObject, FVector pSceneObjectOldLocation);


	// JSON file content
	UPROPERTY()
	FString FileContent;

	// Array of scene objects
	UPROPERTY(EditAnywhere)
	TArray<ASceneObject*> ArrayOfSceneObjects;

	// Maximum value for scene object Z axis rotation in a random direction
	UPROPERTY(EditAnywhere)
	float MaxZRotationSceneObject;

	// Array of the spatial relationships of scene objects
	TArray<TTuple<int32, FString, int32>> ArrayOfSceneObjectRelationships;

	// Array of contained scene objects
	TArray<TTuple<int32, int32>> ArrayOfContainedSceneObjects;

	// Show spatial relationships of scene objects in Output Log
	UPROPERTY(EditAnywhere)
	bool bShowSpatialRelationships;
};