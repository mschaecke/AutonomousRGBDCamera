/**
 * @file AutoRGBDCamera.cpp
 *
 * @brief An autonomous version of RGBDCamera
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 07.04.2021
 *
 */


#include "AutoRGBDCamera.h"

// Contructor for AutoRGBDCamera
AAutoRGBDCamera::AAutoRGBDCamera()
{
    // Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Checkbox to enable Tick()
	EnableTick = true;

    // Set the interval for Tick() in seconds
	TickInterval = 2.0f;

    // Ticks with physics for scene objects, negative value for permanent physics
	TicksWithPhysics = 0;

    // Maximum value for scene object Z axis rotation in a random direction
	MaxZRotationSceneObject = 10.0;

    // Show spatial relationships of scene objects in Output Log
	ShowSpatialRelationships = true;
}

// Called when the game starts or when spawned
void AAutoRGBDCamera::BeginPlay()
{
	Super::BeginPlay();

    // Spawn CameraTrajectory
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Owner = this;
    CameraTrajectory = 
        GetWorld()->SpawnActor<ACameraTrajectory>
        (ACameraTrajectory::StaticClass(), Location, Rotation, SpawnInfo);

    // Spawn SceneConfiguration
    SceneConfiguration = 
        GetWorld()->SpawnActor<ASceneConfiguration>
        (ASceneConfiguration::StaticClass(), Location, Rotation, SpawnInfo);

    // Enable physics for scene objects if TicksWithPhysics > 0
    if (TicksWithPhysics > 0) {
        SceneConfiguration->EnablePhysicsSceneObjects();
    }

    // Set maximum value for scene object Z axis rotation in a random direction
    SceneConfiguration->SetMaxZRotationSceneObject(MaxZRotationSceneObject);

    // Set ShowSpatialRelationships
	SceneConfiguration->SetBShowSpatialRelationships(ShowSpatialRelationships);

    // Initialize the variables
    InitializeVariables();

    // Get X, Y, Z, Roll, Pitch and Yaw values
    GetLocationAndRotationValues();
    
    // Enable Tick() if the camera is located within bounds
    if (CheckCameraInBounds() && EnableTick) 
    {
        EnableTickUsingTickInterval();
    } 
    else if (!CheckCameraInBounds()) 
    {
        this->SetActorTickEnabled(false);
        UE_LOG(LogTemp, Warning, TEXT("ERROR: AutoRGBDCamera is out of bounds."));
        UE_LOG(LogTemp, Warning, TEXT("Tick() was disabled. Please modify the camera location or CameraTrajectory.json."));
    }
    else {
        this->SetActorTickEnabled(false);
    }
}

// Called when the game starts or when spawned
void AAutoRGBDCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// Called every frame
void AAutoRGBDCamera::Tick(float DeltaTime)
{
	// Update TicksWithPhysics and disable physics if necessary
	if (TicksWithPhysics > 0) {
        --TicksWithPhysics;
        return;
    }

    if (TicksWithPhysics == 0) {
        SceneConfiguration->DisablePhysicsSceneObjects();
        --TicksWithPhysics;
    }

    // Update SceneGraph using the current annotation data
    UpdateSceneGraph();

    // Generate data using the Tick() function from RGBDCamera
    Super::Tick(DeltaTime);
    
    // Generate new values for X, Y, Z, Roll, Pitch and Yaw
    XValue = GenerateValueInBounds("X");
    YValue = GenerateValueInBounds("Y");
    ZValue = GenerateValueInBounds("Z");
    RollValue = GenerateValueInBounds("Roll");
    PitchValue = GenerateValueInBounds("Pitch");
    YawValue = GenerateValueInBounds("Yaw");

    // Set the new AutoRGBDCamera location and rotation
    this->SetActorLocationAndRotation(FVector(XValue, YValue, ZValue),FRotator(PitchValue, YawValue, RollValue));

    // Modify the scene configuration using the Tick() function from SceneConfiguration
    SceneConfiguration->Tick(DeltaTime);
}

// Initialize the variables
void AAutoRGBDCamera::InitializeVariables()
{
    // Initialize the Min, Max and Step values for X, Y, Z, Roll, Pitch and Yaw
    XMin = CameraTrajectory->XMin;
    XMax = CameraTrajectory->XMax;
    XStep = CameraTrajectory->XStep;

    YMin = CameraTrajectory->YMin;
    YMax = CameraTrajectory->YMax;
    YStep = CameraTrajectory->YStep;

    ZMin = CameraTrajectory->ZMin;
    ZMax = CameraTrajectory->ZMax;
    ZStep = CameraTrajectory->ZStep;

    RollMin = CameraTrajectory->RollMin;
    RollMax = CameraTrajectory->RollMax;
    RollStep = CameraTrajectory->RollStep;

    PitchMin = CameraTrajectory->PitchMin;
    PitchMax = CameraTrajectory->PitchMax;
    PitchStep = CameraTrajectory->PitchStep;

    YawMin = CameraTrajectory->YawMin;
    YawMax = CameraTrajectory->YawMax;
    YawStep = CameraTrajectory->YawStep;

    // Initialize directions for X, Y, Z, Roll, Pitch and Yaw
    XDirection = FMath::RandRange(-1,1);
    YDirection = FMath::RandRange(-1,1);
    ZDirection = FMath::RandRange(-1,1);
    RollDirection = FMath::RandRange(-1,1);
    PitchDirection = FMath::RandRange(-1,1);
    YawDirection = FMath::RandRange(-1,1);
}

// Check if the camera is located within bounds
bool AAutoRGBDCamera::CheckCameraInBounds()
{
    if ((XValue < XMin) || (XValue > XMax)){
        return false;
    } else if ((YValue < YMin) || (YValue > YMax)){
        return false;
    } else if ((ZValue < ZMin) || (ZValue > ZMax)){
        return false;
    } else if ((RollValue < RollMin) || (RollValue > RollMax)){
        return false;
    } else if ((PitchValue < PitchMin) || (PitchValue > PitchMax)){
        return false;
    } else if ((YawValue < YawMin) || (YawValue > YawMax)){
        return false;
    } else {
        return true;
    }
}

// Enable Tick() using TickInterval
void AAutoRGBDCamera::EnableTickUsingTickInterval()
{
    PrimaryActorTick.Target = this;
    PrimaryActorTick.TickInterval = TickInterval;
    PrimaryActorTick.SetTickFunctionEnable(true);
    PrimaryActorTick.RegisterTickFunction(GetLevel());
}

// Get X, Y, Z, Roll, Pitch and Yaw values
void AAutoRGBDCamera::GetLocationAndRotationValues()
{
    // Get the actor's location vector and rotator
    ActorLocation = this->AActor::GetActorLocation();
    ActorRotation = this->AActor::GetActorRotation();
    
    // Get X, Y, Z, Roll, Pitch and Yaw values
    XValue = ActorLocation.X;
    YValue = ActorLocation.Y;
    ZValue = ActorLocation.Z;
    RollValue = ActorRotation.Roll;
    PitchValue = ActorRotation.Pitch;
    YawValue = ActorRotation.Yaw;
}

// Generates a value within bounds for the variable used as the parameter
float AAutoRGBDCamera::GenerateValueInBounds(FString VariableName)
{
    if (VariableName == "X"){
        XDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(XMin, XMax, XStep, XValue, XDirection)){
           XDirection = FMath::RandRange(-1,1); 
        }
        return (XValue+(XStep*XDirection));
    } 
    
    else if (VariableName == "Y"){
        YDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(YMin, YMax, YStep, YValue, YDirection)){
           YDirection = FMath::RandRange(-1,1);
        }
        return (YValue+(YStep*YDirection));
    }

    else if (VariableName == "Z"){
        ZDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(ZMin, ZMax, ZStep, ZValue, ZDirection)){
           ZDirection = FMath::RandRange(-1,1);
        }
        return (ZValue+(ZStep*ZDirection));
    }

    else if (VariableName == "Roll"){
        RollDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(RollMin, RollMax, RollStep, RollValue, RollDirection)){
           RollDirection = FMath::RandRange(-1,1);
        }
        return (RollValue+(RollStep*RollDirection));
    }

    else if (VariableName == "Pitch"){
        PitchDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(PitchMin, PitchMax, PitchStep, PitchValue, PitchDirection)){
           PitchDirection = FMath::RandRange(-1,1);
        }
        return (PitchValue+(PitchStep*PitchDirection));
    }

    else if (VariableName == "Yaw"){
        YawDirection = FMath::RandRange(-1,1);
        while(!CheckInBounds(YawMin, YawMax, YawStep, YawValue, YawDirection)){
           YawDirection = FMath::RandRange(-1,1);
        }
        return (YawValue+(YawStep*YawDirection));
    }

    else {
        return 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Can't generate a new value for: %s"), *VariableName);
        UE_LOG(LogTemp, Warning, TEXT("0.0f is used instead."));
    }
}

// Check if value is >= min and <= max (bounds) after the step in a certain direction
bool AAutoRGBDCamera::CheckInBounds(float Min, float Max, float Step, float Value, int32 Direction)
{
    float NewValue = Value+(Step*Direction);
    if (NewValue >= Min && NewValue <= Max){
        return true;
    } else {
        return false;
    }
}

// Update SceneGraph using the current annotation data
void AAutoRGBDCamera::UpdateSceneGraph()
{
    // Empty the arrays
    SceneGraph.Objects.Empty();
    SceneGraph.Relations.Empty();

    // Update SceneGraph.Objects
    for (ASceneObject* SceneObject : SceneConfiguration->ArrayOfSceneObjects)
    {
        PacketBuffer::ObjectDescription ObjectDescription;
        PacketBuffer::ObjectProperty ObjectProperty;
        ObjectProperty.ID = SceneObject->SceneObjectID;
        ObjectProperty.Mesh = SceneObject->MeshPath;
        ObjectProperty.Material = SceneObject->MaterialPath;
        FVector Location = SceneObject->GetActorLocation();
        ObjectProperty.Location.Add(Location.X);
        ObjectProperty.Location.Add(Location.Y);
        ObjectProperty.Location.Add(Location.Z);
        FRotator Rotation = SceneObject->GetActorRotation();
        ObjectProperty.Rotation.Add(Rotation.Roll);
        ObjectProperty.Rotation.Add(Rotation.Pitch);
        ObjectProperty.Rotation.Add(Rotation.Yaw);

        ObjectDescription.Properties.Add(ObjectProperty);
        SceneGraph.Objects.Add(ObjectDescription);
    }

    // Update SceneGraph.Relations
    for (TTuple<int32, FString, int32> SceneObjectRelationship : SceneConfiguration->ArrayOfSceneObjectRelationships)
    {
        PacketBuffer::ObjectRelation ObjectRelation;
        ObjectRelation.ID1 = SceneObjectRelationship.Get<0>();
        ObjectRelation.SpatialRelationship = SceneObjectRelationship.Get<1>();
        ObjectRelation.ID2 = SceneObjectRelationship.Get<2>();

        SceneGraph.Relations.Add(ObjectRelation);
    }
}