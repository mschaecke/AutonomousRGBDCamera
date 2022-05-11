// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include <mutex>
#include <vector>
#include <condition_variable>

/**
 * This is a double buffer, one for reading and one for writing. When writing is done they will be swapped.
 * It also acts as the connection between VisionCamera and Server. The StartReading method is blocking until
 * the DoneWriting method is called. So when the VisionActor is done writing, the StartReading methods returns
 * and the Server will start reading and sending the packet.
 */
class AUTONOMOUSRGBDCAMERA_API PacketBuffer
{
public:
  /**
   * packet format:
   * - PacketHeader
   * - Color image data (width * height * 3 Bytes (BGR))
   * - Depth image data (width * height * 2 Bytes (Float16))
   * - Object image data (width * height * 3 Bytes (BGR))
   * - List of map entries
   * - SceneGraph (annotations)
   */

  struct Vector
  {
    float X;
    float Y;
    float Z;
  };

  struct Quaternion
  {
    float X;
    float Y;
    float Z;
    float W;
  };

  struct PacketHeader
  {
    uint32_t Size; // Size of the complete packet
    uint32_t SizeHeader; // Size of the header
    uint32_t MapEntries; // Number of map entries at the end of the packet
    uint32_t Width; // Width of the images
    uint32_t Height; // Height of the images
    uint64_t TimestampCapture; // Timestamp from capture
    uint64_t TimestampSent; // Timestamp from sending
    float FieldOfViewX; // FOV in X direction
    float FieldOfViewY; // FOV in Y dircetion
    Vector Translation; // Translation of the camera for current frame
    Quaternion Rotation; // Rotation of the camera for current frame
    uint32_t numberOfObjects; // Maximum number of objects = 100
    uint32_t numberOfRelations; // Maximum number of relations = 10000
  };

  struct MapEntry
  {
    uint32_t Size; // Size of the complete map entry
    uint8_t R; // Red channel
    uint8_t G; // Green channel
    uint8_t B; // Blue channel
    char FirstChar; // Position of the first character, Size - 7 Bytes in total
  };

  // Scene object properties
	struct ObjectProperty 
  {
		uint32 ID;
		FString Mesh;
		FString Material;
		TArray<FFloat32> Location;
		TArray<FFloat32> Rotation;
	};

	// Description of objects in scene graph
	struct ObjectDescription 
  {
		TArray<ObjectProperty> Properties;
	};

	// Spatial relationship between two scene objects
	struct ObjectRelation 
  {
		uint32 ID1;
		FString SpatialRelationship;
		uint32 ID2;
	};

	// Scene graph for AutoRGBDCamera
	struct SceneGraph 
  {
		TArray<ObjectDescription> Objects;
		TArray<ObjectRelation> Relations;
	};


private:
  std::vector<uint8> ReadBuffer, WriteBuffer;
  bool IsDataReadable;
  std::mutex LockBuffer, LockRead;
  std::condition_variable CVWait;

public:
  // Sizes of the Header, the raw color and depth image data
  const uint32 SizeHeader, SizeRGB, SizeFloat;
  // Size of the scene graph
  uint32 SizeSceneGraph;
  // Offsets for the images and map entries in the packet buffer
  const uint32 OffsetColor, OffsetDepth, OffsetObject, OffsetMap;
  // Offset for SceneGraph in the packet buffer
  uint32 OffsetSceneGraph;
  // Size of the complete packet
  const uint32 Size;
  // Pointers to the beginning of the images, map and SceneGraph for writing and a pointer to the beginning of a completed packet for reading
  uint8 *Color, *Depth, *Object, *Map, *PointerSceneGraph, *Read;
  // Pointer to the packet headers
  PacketHeader *HeaderWrite, *HeaderRead;

  // Initializes the buffer, widht and height are not changeable afterwards
  PacketBuffer(const uint32 Width, const uint32 Height, const float FieldOfView);

  // Starts writing and copies the map entries and the scene graph to the end of the packet.
  void StartWriting(const TMap<FString, uint32> &ObjectToColor, const TArray<FColor> &ObjectColors, const struct SceneGraph &pSceneGraph);

  // Serialize a float array
  void SerializeFloatArray(uint8 *pBuffer, TArray<FFloat32> FloatArray);

  // Copy properties to buffer
  void CopyProperties(uint8*& pBuffer, ObjectProperty Properties);

  // Copy objects to buffer
  void CopyObjects(uint8*& pBuffer, TArray<ObjectDescription> Objects);

  // Copy relations to buffer
  void CopyRelations(uint8*& pBuffer, TArray<ObjectRelation> Relations);

  // Copy SceneGraph to buffer
  void CopySceneGraph(uint8*& pBuffer, SceneGraph pSceneGraph);

  // Swaps reading and writing buffer and unblocks the reading thread
  void DoneWriting();

  // Waits until the reading buffer is valid and locks it.
  void StartReading();

  // Unlocks the reading buffer
  void DoneReading();

  // Releases the lock so that StartReading will return, this is needed to stop the server in the end.
  void Release();
};