// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "PacketBuffer.h"


PacketBuffer::PacketBuffer(const uint32 Width, const uint32 Height, const float FieldOfView) :
  IsDataReadable(false), SizeHeader(sizeof(PacketHeader)), SizeRGB(Width *Height * 3 * sizeof(uint8)), SizeFloat(Width *Height *sizeof(FFloat16)), SizeSceneGraph(sizeof(SceneGraph)),
  OffsetColor(SizeHeader), OffsetDepth(OffsetColor + SizeRGB), OffsetObject(OffsetDepth + SizeFloat), OffsetMap(OffsetObject + SizeRGB), OffsetSceneGraph(OffsetMap + sizeof(MapEntry)),
  Size(SizeHeader + SizeRGB + SizeFloat + SizeRGB)
{
  ReadBuffer.resize(Size + 1024 * 1024);
  WriteBuffer.resize(Size + 1024 * 1024);

  // Create relative FOV for each axis
  const float FOVX = Height > Width ? FieldOfView * Width / Height : FieldOfView;
  const float FOVY = Width > Height ? FieldOfView * Height / Width : FieldOfView;

  // Setting header information that do not change
  HeaderRead = reinterpret_cast<PacketHeader *>(&ReadBuffer[0]);
  HeaderRead->Size = Size;
  HeaderRead->SizeHeader = SizeHeader;
  HeaderRead->Width = Width;
  HeaderRead->Height = Height;
  HeaderRead->FieldOfViewX = FOVX;
  HeaderRead->FieldOfViewY = FOVY;
  HeaderWrite = reinterpret_cast<PacketHeader *>(&WriteBuffer[0]);
  HeaderWrite->Size = Size;
  HeaderWrite->SizeHeader = SizeHeader;
  HeaderWrite->Width = Width;
  HeaderWrite->Height = Height;
  HeaderWrite->FieldOfViewX = FOVX;
  HeaderWrite->FieldOfViewY = FOVY;

  // Setting the pointers to the data
  Color = &WriteBuffer[OffsetColor];
  Depth = &WriteBuffer[OffsetDepth];
  Object = &WriteBuffer[OffsetObject];
  Map = &WriteBuffer[OffsetMap];
  PointerSceneGraph = &WriteBuffer[OffsetSceneGraph];
  Read = &ReadBuffer[0];

  IsDataReadable = false;
}

void PacketBuffer::StartWriting(const TMap<FString, uint32> &ObjectToColor, const TArray<FColor> &ObjectColors, const struct SceneGraph &pSceneGraph)
{
  uint32_t Count = 0;
  uint32_t MapSize = 0;
  uint8_t *It = Map;

  // Writing the object color map entries to the end of the packet
  for(auto &Elem : ObjectToColor)
  {
    const uint32_t NameSize = Elem.Key.Len();
    const uint32_t ElemSize = sizeof(uint32_t) + 3 * sizeof(uint8_t) + NameSize;
    const FColor &ObjectColor = ObjectColors[Elem.Value];

    // Resize the internal buffer if necessary
    if(Size + MapSize + ElemSize > WriteBuffer.size())
    {
      WriteBuffer.resize(WriteBuffer.size() + 1024 * 1024);
      // Update pointers
      Color = &WriteBuffer[OffsetColor];
      Depth = &WriteBuffer[OffsetDepth];
      Object = &WriteBuffer[OffsetObject];
      Map = &WriteBuffer[OffsetMap];
      OffsetSceneGraph = OffsetMap + MapSize;
      PointerSceneGraph = &WriteBuffer[OffsetSceneGraph];
      HeaderWrite = reinterpret_cast<PacketHeader *>(&WriteBuffer[0]);
    }

    MapEntry *Entry = reinterpret_cast<MapEntry*>(It);
    Entry->Size = ElemSize;

    Entry->R = ObjectColor.R;
    Entry->G = ObjectColor.G;
    Entry->B = ObjectColor.B;

    // Convert name to ANSI and copy it to the packet (no trailing '\0', length is indirectly given by the entry size)
    const char *Name = TCHAR_TO_ANSI(*Elem.Key);
    memcpy(&Entry->FirstChar, Name, NameSize);

    It += ElemSize;
    MapSize += ElemSize;
    ++Count;
  }

  HeaderWrite->MapEntries = Count;
  OffsetSceneGraph = OffsetMap + MapSize;
  PointerSceneGraph = &WriteBuffer[OffsetSceneGraph];

  // Write the SceneGraph data to the end of the packet
  HeaderWrite->numberOfObjects = pSceneGraph.Objects.Num();
  HeaderWrite->numberOfRelations = pSceneGraph.Relations.Num();

  SizeSceneGraph = 0;
  int Counter = 0;
  int Counter2 = 0;

  for (PacketBuffer::ObjectDescription Object : pSceneGraph.Objects)
  {
    for (PacketBuffer::ObjectProperty Property : Object.Properties)
    {
      SizeSceneGraph += sizeof(int32) + Property.Mesh.Len() + Property.Material.Len() + 3 * sizeof(Float32) + 3 * sizeof(Float32);
      ++Counter;
    }
  }

  for (PacketBuffer::ObjectRelation Relation : pSceneGraph.Relations) 
  {
    SizeSceneGraph += sizeof(int32) + Relation.SpatialRelationship.Len() + sizeof(int32);
    ++Counter2;
  }

  HeaderWrite->Size += (MapSize + SizeSceneGraph);

  // Copy SceneGraph to the packet
  CopySceneGraph(PointerSceneGraph, pSceneGraph);
}

// Serialize a float array
void PacketBuffer::SerializeFloatArray(uint8 *pBuffer, TArray<FFloat32> FloatArray)
{
  for (int i = 0; i < FloatArray.Num(); ++i)
  {
    memcpy(pBuffer + i * sizeof(FFloat32), (char *)&FloatArray[i], sizeof(FFloat32));
  }
}

// Copy properties to buffer
void PacketBuffer::CopyProperties(uint8*& pBuffer, PacketBuffer::ObjectProperty Properties)
{
  memcpy(pBuffer, &Properties.ID, sizeof(int32));
  pBuffer += sizeof(int32);
  
  const char *Mesh = TCHAR_TO_ANSI(*Properties.Mesh);
  int32 s = Properties.Mesh.Len();
  memcpy(pBuffer, &s, sizeof(int32));
  pBuffer += sizeof(uint32_t);
  memcpy(pBuffer, Mesh, Properties.Mesh.Len());
  pBuffer += Properties.Mesh.Len();

  const char *Material = TCHAR_TO_ANSI(*Properties.Material);
  *reinterpret_cast<uint32_t *>(pBuffer) = Properties.Material.Len();
  pBuffer += sizeof(uint32_t);
  memcpy(pBuffer, Material, Properties.Material.Len());
  pBuffer += Properties.Material.Len();

  SerializeFloatArray(pBuffer, Properties.Location);
  pBuffer += Properties.Location.Num() * sizeof(FFloat32);
  
  SerializeFloatArray(pBuffer, Properties.Rotation);
  pBuffer += Properties.Rotation.Num() * sizeof(FFloat32);
}

// Copy objects to buffer
void PacketBuffer::CopyObjects(uint8*& pBuffer, TArray<PacketBuffer::ObjectDescription> Objects) 
{
  for (PacketBuffer::ObjectDescription Description : Objects) {
    for (PacketBuffer::ObjectProperty Properties : Description.Properties) 
    {
      CopyProperties(pBuffer, Properties);
    }
  }
}

// Copy relations to buffer
void PacketBuffer::CopyRelations(uint8*& pBuffer, TArray<PacketBuffer::ObjectRelation> Relations) 
{
  for (PacketBuffer::ObjectRelation Relation : Relations) 
  {
    memcpy(pBuffer, (char *)&Relation.ID1, sizeof(int32));
    pBuffer += sizeof(int32);

    const char *SpatialRelationship = TCHAR_TO_ANSI(*Relation.SpatialRelationship);
    *reinterpret_cast<uint32_t *>(pBuffer) = Relation.SpatialRelationship.Len();
    pBuffer += sizeof(uint32_t);
    memcpy(pBuffer, SpatialRelationship, Relation.SpatialRelationship.Len());
    pBuffer += Relation.SpatialRelationship.Len();
    
    memcpy(pBuffer, (char *)&Relation.ID2, sizeof(int32));
    pBuffer += sizeof(int32);
  }
}

// Copy SceneGraph to buffer
void PacketBuffer::CopySceneGraph(uint8*& pBuffer, SceneGraph pSceneGraph) 
{
  CopyObjects(pBuffer, pSceneGraph.Objects);
  CopyRelations(pBuffer, pSceneGraph.Relations);
}

void PacketBuffer::DoneWriting()
{
  // Swapping buffers
  LockBuffer.lock();
  IsDataReadable = true;
  WriteBuffer.swap(ReadBuffer);
  Color = &WriteBuffer[OffsetColor];
  Depth = &WriteBuffer[OffsetDepth];
  Object = &WriteBuffer[OffsetObject];
  Map = &WriteBuffer[OffsetMap];
  PointerSceneGraph = &WriteBuffer[OffsetSceneGraph];
  Read = &ReadBuffer[0];
  HeaderRead = reinterpret_cast<PacketHeader *>(&ReadBuffer[0]);
  HeaderWrite = reinterpret_cast<PacketHeader *>(&WriteBuffer[0]);
  LockBuffer.unlock();
  CVWait.notify_one();
}

void PacketBuffer::StartReading()
{
  // Waits until writing is done
  std::unique_lock<std::mutex> WaitLock(LockRead);
  CVWait.wait(WaitLock, [this] {return IsDataReadable; });

  LockBuffer.lock();
}

void PacketBuffer::DoneReading()
{
  IsDataReadable = false;
  LockBuffer.unlock();
}

void PacketBuffer::Release()
{
  IsDataReadable = true;
  CVWait.notify_one();
}