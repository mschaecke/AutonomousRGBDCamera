/**
 * @file AutoRGBDCamera.h
 *
 * @brief An autonomous version of RGBDCamera
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 07.04.2021
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "DefaultRGBDCamera.h"
#include "CameraTrajectory.h"
#include "SceneConfiguration.h"
#include "AutoRGBDCamera.generated.h"

UCLASS()
class AUTONOMOUSRGBDCAMERA_API AAutoRGBDCamera : public ADefaultRGBDCamera
{
	GENERATED_BODY()
	
public:	
	// Contructor for AutoRGBDCamera
	AAutoRGBDCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Initialize the variables
	void InitializeVariables();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Check if the camera is located within bounds
	bool CheckCameraInBounds();

	// Enable Tick() using TickInterval
	void EnableTickUsingTickInterval();

	// Get X, Y, Z, Roll, Pitch and Yaw values
	void GetLocationAndRotationValues();

	// Generates a value within bounds for the variable used as the parameter
	float GenerateValueInBounds(FString VariableName);

	// Check if value is >= min and <= max (bounds) after the step in a certain direction
	bool CheckInBounds(float Min, float Max, float Step, float Value, int32 Direction);

	// Update SceneGraph using the current annotation data
	void UpdateSceneGraph();


	// Checkbox to enable Tick()
	UPROPERTY(EditAnywhere)
	bool EnableTick;

	// Tick interval
	UPROPERTY(EditAnywhere)
	float TickInterval;

	// Ticks with physics for scene objects, negative value for permanent physics
	UPROPERTY(EditAnywhere)
	int TicksWithPhysics;

	// Maximum value for scene object Z axis rotation in a random direction
	UPROPERTY(EditAnywhere)
	float MaxZRotationSceneObject;

	// Show spatial relationships of scene objects in Output Log
	UPROPERTY(EditAnywhere)
	bool ShowSpatialRelationships;


	// Camera trajectory
	UPROPERTY(EditAnywhere)
    ACameraTrajectory* CameraTrajectory;

	// SceneConfiguration
	UPROPERTY(EditAnywhere)
	ASceneConfiguration* SceneConfiguration;


	// Actor location
	FVector ActorLocation;

	// Actor rotation
	FRotator ActorRotation;


	// Minimum value for X
	UPROPERTY()
	float XMin;

	// Maximum value for X
	UPROPERTY()
	float XMax;
	
	// Step length for X
	UPROPERTY()
	float XStep;

	// X Location
	float XValue;

	UPROPERTY(VisibleAnywhere)
	// Direction for X
	int32 XDirection;


	// Minimum value for Y
	UPROPERTY()
	float YMin;
	
	// Maximum value for Y
	UPROPERTY()
	float YMax;
	
	// Step length for Y
	UPROPERTY()
	float YStep;

	// Y Location
	float YValue;

	// Direction for Y
	UPROPERTY(VisibleAnywhere)
	int32 YDirection;


	// Minimum value for Z
	UPROPERTY()
	float ZMin;
	
	// Maximum value for Z
	UPROPERTY()
	float ZMax;
	
	// Step length for Z
	UPROPERTY()
	float ZStep;

	// Z Location
	float ZValue;

	// Direction for Z
	UPROPERTY(VisibleAnywhere)
	int32 ZDirection;


	// Minimum value for Roll
	UPROPERTY()
	float RollMin;
	
	// Maximum value for Roll
	UPROPERTY()
	float RollMax;
	
	// Step length for Roll
	UPROPERTY()
	float RollStep;

	// Roll value
	float RollValue;	

	// Direction for Roll
	UPROPERTY(VisibleAnywhere)
	int32 RollDirection;


	// Minimum value for Pitch
	UPROPERTY()
	float PitchMin;
	
	// Maximum value for Pitch
	UPROPERTY()
	float PitchMax;
	
	// Step length for Pitch
	UPROPERTY()
	float PitchStep;

	// Pitch value
	float PitchValue;

	// Direction for Pitch
	UPROPERTY(VisibleAnywhere)
	int32 PitchDirection;


	// Minimum value for Yaw
	UPROPERTY()
	float YawMin;
	
	// Maximum value for Yaw
	UPROPERTY()
	float YawMax;
	
	// Step length for Yaw
	UPROPERTY()
	float YawStep;

	// Yaw value
	float YawValue;	

	// Direction for Yaw
	UPROPERTY(VisibleAnywhere)
	int32 YawDirection;
};