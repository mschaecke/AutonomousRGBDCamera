/**
 * @file SceneObject.cpp
 *
 * @brief A single object within the scene
 *
 * @author Marvin Alexander Schäcke
 *
 * @date 27.05.2021
 *
 */


#include "SceneObject.h"

// Constructor for SceneObject
ASceneObject::ASceneObject()
{
 	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create root component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create static mesh component
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(Root);

	// Initialize SceneObjectID
	SceneObjectID = -1;
}

// Called when the game starts or when spawned
void ASceneObject::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASceneObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Create mesh and material
	CreateMesh();
	CreateMaterial();

	// Disable Tick() once mesh and material are created
	this->SetActorTickEnabled(false);
}


// Get scene object ID
int32 ASceneObject::GetSceneObjectID() 
{
	return SceneObjectID;
}

// Set scene object ID
void ASceneObject::SetSceneObjectID(int32 pSceneObjectID) 
{
	SceneObjectID = pSceneObjectID;
}

// Set the mesh path
void ASceneObject::SetMeshPath(FString pMeshPath) 
{
	MeshPath = pMeshPath;
}

// Create the mesh using MeshPath and StaticMeshComponent
void ASceneObject::CreateMesh() 
{
	const TCHAR *ObjectToFind;
	const FString ObjectText = FString(TEXT("StaticMesh'")) + MeshPath + FString(TEXT("'"));
	ObjectToFind = *ObjectText;

	UStaticMesh *Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, ObjectToFind));
	if (Mesh && StaticMeshComponent)
	{
		StaticMeshComponent->SetStaticMesh(Mesh);
	}
}

// Set the material path
void ASceneObject::SetMaterialPath(FString pMaterialPath) 
{
	MaterialPath = pMaterialPath;
}

// Create the material using MaterialPath and StaticMeshComponent
void ASceneObject::CreateMaterial() 
{
	const TCHAR *ObjectToFind;
	const FString ObjectText = FString(TEXT("Material'")) + MaterialPath + FString(TEXT("'"));
	ObjectToFind = *ObjectText;

	UMaterial* Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), NULL, ObjectToFind));
	if (Material && StaticMeshComponent)
    {
		StaticMeshComponent->SetMaterial(0, Material);
	}
}