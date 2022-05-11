/**
 * @file CameraTrajectory.h
 *
 * @brief The camera trajectory for AutoRGBDCamera
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 25.05.2021
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "CameraTrajectory.generated.h"

UCLASS()
class AUTONOMOUSRGBDCAMERA_API ACameraTrajectory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Constructor for CameraTrajectory
	ACameraTrajectory();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Load the JSON file from the project's config directory
	void LoadJsonFile();

	// Initialize the variables using the JSON file
	void InitializeVariables();


	// JSON file content
	UPROPERTY()
	FString FileContent;

	// Minimum value for X
	UPROPERTY(EditAnywhere)
	float XMin;

	// Maximum value for X
	UPROPERTY(EditAnywhere)
	float XMax;
	
	// Step length for X
	UPROPERTY(EditAnywhere)
	float XStep;


	// Minimum value for Y
	UPROPERTY(EditAnywhere)
	float YMin;
	
	// Maximum value for Y
	UPROPERTY(EditAnywhere)
	float YMax;
	
	// Step length for Y
	UPROPERTY(EditAnywhere)
	float YStep;


	// Minimum value for Z
	UPROPERTY(EditAnywhere)
	float ZMin;
	
	// Maximum value for Z
	UPROPERTY(EditAnywhere)
	float ZMax;
	
	// Step length for Z
	UPROPERTY(EditAnywhere)
	float ZStep;


	// Minimum value for Roll
	UPROPERTY(EditAnywhere)
	float RollMin;
	
	// Maximum value for Roll
	UPROPERTY(EditAnywhere)
	float RollMax;
	
	// Step length for Roll
	UPROPERTY(EditAnywhere)
	float RollStep;


	// Minimum value for Pitch
	UPROPERTY(EditAnywhere)
	float PitchMin;
	
	// Maximum value for Pitch
	UPROPERTY(EditAnywhere)
	float PitchMax;
	
	// Step length for Pitch
	UPROPERTY(EditAnywhere)
	float PitchStep;


	// Minimum value for Yaw
	UPROPERTY(EditAnywhere)
	float YawMin;
	
	// Maximum value for Yaw
	UPROPERTY(EditAnywhere)
	float YawMax;
	
	// Step length for Yaw
	UPROPERTY(EditAnywhere)
	float YawStep;
};