/**
 * @file CameraTrajectory.cpp
 *
 * @brief The camera trajectory for AutoRGBDCamera
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 25.05.2021
 *
 */


#include "CameraTrajectory.h"

// Constructor for CameraTrajectory
ACameraTrajectory::ACameraTrajectory()
{
 	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = false;

    // Load the JSON file from the project's config directory
    LoadJsonFile();
    
    // Initialize the variables using the JSON file
    InitializeVariables();
}

// Called when the game starts or when spawned
void ACameraTrajectory::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACameraTrajectory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Load the JSON file from the project's config directory
void ACameraTrajectory::LoadJsonFile()
{
    // Use the project's config directory to store the JSON file
    FString FileName = FPaths::ProjectConfigDir();
    FileName.Append(TEXT("CameraTrajectory.json"));
    IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();
    
    if (FileManager.FileExists(*FileName)) {
        // Load the content of the JSON file as FString into FileContent
        if (FFileHelper::LoadFileToString(FileContent, *FileName, FFileHelper::EHashOptions::None)) {
            UE_LOG(LogTemp, Warning, TEXT("Text from CameraTrajectory.json: %s"), *FileContent);
        } else {
            UE_LOG(LogTemp, Warning, TEXT("Unable to load text from CameraTrajectory.json."));
        }
    }
    // Error and required directory and JSON file name in the Output Log
    else {
        UE_LOG(LogTemp, Warning, TEXT("ERROR: Can't read CameraTrajectory.json because it wasn't found."));
        UE_LOG(LogTemp, Warning, TEXT("Expected file location and name: %s"), *FileName);
    }
}

// Initialize the variables using the JSON file
void ACameraTrajectory::InitializeVariables()
{
    // Read FileContent and convert it to a JSON object
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
            // Initialize the Min, Max and Step values for X, Y, Z, Roll, Pitch and Yaw

			XMin = JsonObject->GetNumberField("XMin");
			XMax = JsonObject->GetNumberField("XMax");
			XStep = JsonObject->GetNumberField("XStep");

            YMin = JsonObject->GetNumberField("YMin");
			YMax = JsonObject->GetNumberField("YMax");
			YStep = JsonObject->GetNumberField("YStep");

            ZMin = JsonObject->GetNumberField("ZMin");
			ZMax = JsonObject->GetNumberField("ZMax");
			ZStep = JsonObject->GetNumberField("ZStep");

            RollMin = JsonObject->GetNumberField("RollMin");
			RollMax = JsonObject->GetNumberField("RollMax");
			RollStep = JsonObject->GetNumberField("RollStep");

            PitchMin = JsonObject->GetNumberField("PitchMin");
			PitchMax = JsonObject->GetNumberField("PitchMax");
			PitchStep = JsonObject->GetNumberField("PitchStep");

            YawMin = JsonObject->GetNumberField("YawMin");
			YawMax = JsonObject->GetNumberField("YawMax");
			YawStep = JsonObject->GetNumberField("YawStep");
		}
}