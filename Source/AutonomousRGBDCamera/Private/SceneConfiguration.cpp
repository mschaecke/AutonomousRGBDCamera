/**
 * @file SceneConfiguration.cpp
 *
 * @brief A configuration of scene objects
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 29.05.2021
 *
 */


#include "SceneConfiguration.h"

// Constructor for SceneConfiguration
ASceneConfiguration::ASceneConfiguration()
{
    // Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = false;

    // Maximum value for scene object Z axis rotation in a random direction
	MaxZRotationSceneObject = 0.0;

    // Show spatial relationships of scene objects in Output Log
	bShowSpatialRelationships = true;
}

// Called when the game starts or when spawned
void ASceneConfiguration::BeginPlay()
{
	Super::BeginPlay();

    // Load the JSON file from the project's config directory
    LoadJsonFile();
    
    // Spawn scene objects using the JSON file
    UseJsonFileContent();
}

// Called every frame
void ASceneConfiguration::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Enable all scene objects
    EnableAllSceneObjects();

    // Randomly swap scene objects
    RandomlySwapSceneObjects();

    // Randomly disable scene objects
    RandomlyDisableSceneObjects();

    // Show spatial relationships of scene objects in Output Log
    ShowSpatialRelationships();
}

// Load the JSON file from the project's config directory
void ASceneConfiguration::LoadJsonFile()
{
    // Use the project's config directory to store the JSON file
    FString FileName = FPaths::ProjectConfigDir();
    FileName.Append(TEXT("SceneConfiguration.json"));
    IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();
    
    if (FileManager.FileExists(*FileName)) {
        // Load the content of the JSON file as FString into FileContent
        if (FFileHelper::LoadFileToString(FileContent, *FileName, FFileHelper::EHashOptions::None)) {
            UE_LOG(LogTemp, Warning, TEXT("Text from SceneConfiguration.json: %s"), *FileContent);
        } else {
            UE_LOG(LogTemp, Warning, TEXT("Unable to load text from SceneConfiguration.json."));
        }
    }
    // Error and required directory and JSON file name in the Output Log
    else {
        UE_LOG(LogTemp, Warning, TEXT("ERROR: Can't read SceneConfiguration.json because it wasn't found."));
        UE_LOG(LogTemp, Warning, TEXT("Expected file location and name: %s"), *FileName);
    }
}

// Spawn the scene objects and initialize spatial relationships
void ASceneConfiguration::UseJsonFileContent() 
{
    // Read FileContent and convert it to a JSON object
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);
    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {

        // Spawn the scene objects
        const auto SceneObjects = JsonObject->GetArrayField("SceneObjects");
        for (int i = 0; i < SceneObjects.Num(); ++i)
        {
            // Get the values
            auto CurrentSceneObject = SceneObjects[i]->AsObject();

            float X = CurrentSceneObject->GetNumberField("X");
            float Y = CurrentSceneObject->GetNumberField("Y");
            float Z = CurrentSceneObject->GetNumberField("Z");

            float Roll = CurrentSceneObject->GetNumberField("Roll");
            float Pitch = CurrentSceneObject->GetNumberField("Pitch");
            float Yaw = CurrentSceneObject->GetNumberField("Yaw");

            // Spawn SceneObject
            FVector Location(X, Y, Z);
            FRotator Rotation(Pitch, Yaw, Roll);
            FActorSpawnParameters SpawnInfo;
            SpawnInfo.Owner = this;
            ASceneObject *SceneObject =
                GetWorld()->SpawnActor<ASceneObject>(ASceneObject::StaticClass(), Location, Rotation, SpawnInfo);

            SceneObject->SetSceneObjectID(CurrentSceneObject->GetIntegerField("ID"));
            SceneObject->SetMeshPath(CurrentSceneObject->GetStringField("MeshPath"));
            SceneObject->SetMaterialPath(CurrentSceneObject->GetStringField("MaterialPath"));

            ArrayOfSceneObjects.Add(SceneObject);
        }

        EnableAllSceneObjects();

        // Initialize spatial relationships
        const auto SpatialRelationships = JsonObject->GetArrayField("SpatialRelationships");
        for (int i = 0; i < SpatialRelationships.Num(); ++i)
        {
            // Get the values
            auto CurrentSR = SpatialRelationships[i]->AsObject();

            int32 ID1 = CurrentSR->GetIntegerField("ID1");
            FString SpatialRelationship = CurrentSR->GetStringField("SpatialRelationship");
            int32 ID2 = CurrentSR->GetIntegerField("ID2");

            // Add spatial relationship tuple to array
            ArrayOfSceneObjectRelationships.Add(MakeTuple(ID1, SpatialRelationship, ID2));

            // Add tuple to array of contained scene objects if "contain" spatial relationship exists
            if (SpatialRelationship == "contain") 
            {
                ArrayOfContainedSceneObjects.Add(MakeTuple(ID1, ID2));
            }
        }
    }
}

// Enable all scene objects
void ASceneConfiguration::EnableAllSceneObjects() 
{
    for (ASceneObject* SceneObject : ArrayOfSceneObjects) 
    {
        EnableSceneObject(SceneObject);
    }
}

// Enable scene object
void ASceneConfiguration::EnableSceneObject(ASceneObject* pSceneObject) 
{
    pSceneObject->SetActorHiddenInGame(false);
    pSceneObject->SetActorEnableCollision(true);
}

// Randomly swap scene objects
void ASceneConfiguration::RandomlySwapSceneObjects() 
{
    for (ASceneObject* SceneObject : ArrayOfSceneObjects) 
    {
        // Randomly decide what should be changed
        int32 SwapSceneObjectRandom = FMath::RandRange(0,1);
        
        if (ArrayOfSceneObjects.Num() > 1 && SwapSceneObjectRandom == 1) {
            SwapSceneObjects(SceneObject);
        }
    }
}

// Swap scene objects using the argument, a random swap partner and new rotations for both
void ASceneConfiguration::SwapSceneObjects(ASceneObject *pSceneObject)
{
    ASceneObject *SwapPartner = GetSwapPartner(pSceneObject);

    // Only continue if both scene objects aren't contained by another scene object
    if (CheckIsContained(pSceneObject) || CheckIsContained(SwapPartner)) 
    {
        return;
    }

    FVector pSceneObjectLocation = pSceneObject->GetActorLocation();
    FRotator pSceneObjectRotation = pSceneObject->GetActorRotation();
    FVector SwapPartnerLocation = SwapPartner->GetActorLocation();
    FRotator SwapPartnerRotation = SwapPartner->GetActorRotation();

    // Get random rotation values within the specified step length
    float CurrentZAxisStep = FMath::RandRange(-MaxZRotationSceneObject,MaxZRotationSceneObject);
    float SwapZAxisStep = FMath::RandRange(-MaxZRotationSceneObject,MaxZRotationSceneObject);

    // New locations and rotations using the steps
    FVector NewpSceneObjectLocation = FVector(SwapPartnerLocation.X, SwapPartnerLocation.Y, pSceneObjectLocation.Z);
    FRotator NewpSceneObjectRotation = FRotator(pSceneObjectRotation.Pitch, pSceneObjectRotation.Yaw+CurrentZAxisStep, pSceneObjectRotation.Roll);
    FVector NewSwapPartnerLocation = FVector(pSceneObjectLocation.X, pSceneObjectLocation.Y, SwapPartnerLocation.Z);
    FRotator NewSwapPartnerRotation = FRotator(SwapPartnerRotation.Pitch, SwapPartnerRotation.Yaw+SwapZAxisStep, SwapPartnerRotation.Roll);

    // Disable SwapPartner so pSceneObject can move to it's location
    DisableSceneObject(SwapPartner);
    EnableSceneObject(pSceneObject);

    // Swap the scene objects if possible
    if (pSceneObject->SetActorLocationAndRotation(NewpSceneObjectLocation, NewpSceneObjectRotation)) 
    {
        DisableSceneObject(pSceneObject);
        EnableSceneObject(SwapPartner);

        // Reset pSceneObject if SwapPartner can't move
        if (!SwapPartner->SetActorLocationAndRotation(NewSwapPartnerLocation, NewSwapPartnerRotation)) 
        {
            pSceneObject->SetActorLocationAndRotation(pSceneObjectLocation, pSceneObjectRotation);
        }
        // Move contained objects and update the spatial relationships if both moved 
        else 
        {
            MoveContainedObjects(pSceneObject, pSceneObjectLocation);
            MoveContainedObjects(SwapPartner, SwapPartnerLocation);
            UpdateSpatialRelationshipSceneObjects(pSceneObject, SwapPartner);
        }
    }

    EnableSceneObject(pSceneObject);
}

// Get a random partner for SwapSceneObjects
ASceneObject* ASceneConfiguration::GetSwapPartner(ASceneObject* pSceneObject) 
{
    ASceneObject* SwapPartner = NULL;

    while (SwapPartner == NULL || SwapPartner == pSceneObject)
    {
        int32 RandomArrayIndex = FMath::RandRange(0, ArrayOfSceneObjects.Num()-1);
        SwapPartner = ArrayOfSceneObjects[RandomArrayIndex];
    }
    return SwapPartner;
}

// Set MaxZRotationSceneObject
void ASceneConfiguration::SetMaxZRotationSceneObject(float pMaxZRotationSceneObject) 
{
    MaxZRotationSceneObject = pMaxZRotationSceneObject;
}

// Randomly disable scene objects
void ASceneConfiguration::RandomlyDisableSceneObjects() 
{
    for (ASceneObject* SceneObject : ArrayOfSceneObjects) 
    {
        int32 SceneObjectEnabledRandom = FMath::RandRange(0,1);

        if (SceneObjectEnabledRandom == 0) 
        {
           DisableSceneObject(SceneObject);
        }
    }
}

// Disable scene object
void ASceneConfiguration::DisableSceneObject(ASceneObject* pSceneObject) 
{
    pSceneObject->SetActorHiddenInGame(true);
    pSceneObject->SetActorEnableCollision(false);
}

// Enable physics for all scene objects
void ASceneConfiguration::EnablePhysicsSceneObjects() 
{
    for (ASceneObject* SceneObject : ArrayOfSceneObjects) {
        SceneObject->StaticMeshComponent->SetSimulatePhysics(true);
    }   
}

// Disable physics for all scene objects
void ASceneConfiguration::DisablePhysicsSceneObjects() 
{
    for (ASceneObject* SceneObject : ArrayOfSceneObjects) {
        SceneObject->StaticMeshComponent->SetSimulatePhysics(false);
    }
}

// Update the spatial relationships of scene objects after swapping
void ASceneConfiguration::UpdateSpatialRelationshipSceneObjects(ASceneObject* pSceneObject1, ASceneObject* pSceneObject2) 
{
    int32 ID1 = pSceneObject1->GetSceneObjectID();
    int32 ID2 = pSceneObject2->GetSceneObjectID();

    TArray<TTuple<int32, FString, int32>> TempArrayOfSceneObjectRelationships;

    for (TTuple<int32, FString, int32> CurrentTuple : ArrayOfSceneObjectRelationships) 
    {
        int32 CurrentID1 = CurrentTuple.Get<0>();
        FString SpatialRelationship = CurrentTuple.Get<1>();
        int32 CurrentID2 = CurrentTuple.Get<2>();

        // Don't modify spatial relationship if IDs are in ArrayOfContainedSceneObjects
        bool IsContained = false;

        for (TTuple<int32, int32> InnerCurrentTuple : ArrayOfContainedSceneObjects) 
        {
            int32 InnerCurrentID1 = InnerCurrentTuple.Get<0>();
            int32 InnerCurrentID2 = InnerCurrentTuple.Get<1>();

            if (InnerCurrentID1 == CurrentID1 && InnerCurrentID2 == CurrentID2) 
            {
                IsContained = true;
            }
            if (InnerCurrentID1 == CurrentID2 && InnerCurrentID2 == CurrentID1) 
            {
                IsContained = true;
            }
        }

        if (IsContained) 
        {
            TempArrayOfSceneObjectRelationships.Add(CurrentTuple);
            continue;
        }

        // Check if spatial relationship contains IDs and modify them
        if (CurrentID1 == ID1 && CurrentID2 == ID2) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(ID2, SpatialRelationship, ID1));
        }

        else if (CurrentID1 == ID2 && CurrentID2 == ID1) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(ID1, SpatialRelationship, ID2));
        }

        else if (CurrentID1 == ID1) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(ID2, SpatialRelationship, CurrentID2));            
        }

        else if (CurrentID1 == ID2) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(ID1, SpatialRelationship, CurrentID2));
        }

        else if (CurrentID2 == ID1) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(CurrentID1, SpatialRelationship, ID2)); 
        }

        else if (CurrentID2 == ID2) 
        {
            TempArrayOfSceneObjectRelationships.Add(MakeTuple(CurrentID1, SpatialRelationship, ID1));
        }

        else 
        {
            TempArrayOfSceneObjectRelationships.Add(CurrentTuple);
        }
    }

    ArrayOfSceneObjectRelationships = TempArrayOfSceneObjectRelationships;
}

// Set bShowSpatialRelationships
void ASceneConfiguration::SetBShowSpatialRelationships(bool pBShowSpatialRelationships) 
{
    bShowSpatialRelationships = pBShowSpatialRelationships;
}

// Show spatial relationships of scene objects in Output Log
void ASceneConfiguration::ShowSpatialRelationships() 
{
    if (bShowSpatialRelationships && ArrayOfSceneObjectRelationships.Num() > 0) 
    {
        UE_LOG(LogTemp, Warning, TEXT("ArrayOfSceneObjectRelationships: "));

        for (TTuple<int32, FString, int32> SceneObjectRelationship : ArrayOfSceneObjectRelationships) 
        {
            int32 ID1 = SceneObjectRelationship.Get<0>();
            FString SpatialRelationship = SceneObjectRelationship.Get<1>();
            int32 ID2 = SceneObjectRelationship.Get<2>();

            UE_LOG(LogTemp, Warning, TEXT("%d %s %d"), ID1, *SpatialRelationship, ID2);
        }
    }
}

// Check if a scene object is contained by another scene object
bool ASceneConfiguration::CheckIsContained(ASceneObject* pSceneObject) 
{
    int32 pSceneObjectID = pSceneObject->GetSceneObjectID();

    for (int i = 0; i < ArrayOfContainedSceneObjects.Num(); ++i) 
    {
        int32 CurrentID2 = ArrayOfContainedSceneObjects[i].Get<1>();

        if (CurrentID2 == pSceneObjectID) 
        {
            return true;
        }
    }

    return false;
}

// Move objects contained by pSceneObject to the new location
void ASceneConfiguration::MoveContainedObjects(ASceneObject* pSceneObject, FVector pSceneObjectOldLocation) 
{
    int32 pSceneObjectID = pSceneObject->GetSceneObjectID();

    // Search for objects contained by pSceneObject
    for (TTuple<int32, FString, int32> CurrentTuple : ArrayOfSceneObjectRelationships) 
    {
        int32 CurrentID1 = CurrentTuple.Get<0>();
        FString SpatialRelationship = CurrentTuple.Get<1>();

        if (CurrentID1 == pSceneObjectID && SpatialRelationship == "contain") 
        {
            int32 CurrentID2 = CurrentTuple.Get<2>();
            ASceneObject* CurrentID2SceneObject;
            FVector CurrentID2Location;
            FRotator CurrentID2Rotation;

            // Find the contained object based on ID
            for (ASceneObject* SceneObject : ArrayOfSceneObjects) {
                if (SceneObject->GetSceneObjectID() == CurrentID2) 
                {
                    CurrentID2SceneObject = SceneObject;
                    CurrentID2Location = SceneObject->GetActorLocation();
                    CurrentID2Rotation = SceneObject->GetActorRotation();
                    break;
                }
            }

            // Compute new location and rotation and move contained object
            FVector LocationChange = pSceneObject->GetActorLocation() - pSceneObjectOldLocation;
            float NewX = CurrentID2Location.X + LocationChange.X;
            float NewY = CurrentID2Location.Y + LocationChange.Y;
            CurrentID2Location = FVector(NewX, NewY, CurrentID2Location.Z);
            CurrentID2Rotation = FRotator(CurrentID2Rotation.Pitch, pSceneObject->GetActorRotation().Yaw, CurrentID2Rotation.Roll);
            
            DisableSceneObject(CurrentID2SceneObject);
            CurrentID2SceneObject->SetActorLocationAndRotation(CurrentID2Location, CurrentID2Rotation);
            EnableSceneObject(CurrentID2SceneObject);
        }
    }
}