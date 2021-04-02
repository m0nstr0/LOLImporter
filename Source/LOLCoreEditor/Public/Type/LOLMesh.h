#pragma once

#include "CoreMinimal.h"

namespace LOLImporter
{
    struct FLOLVertex
    {
		FVector Position;
        uint8 JointIndex[4];
        float Weight[4];
		FVector Normal;
		FVector2D TexCoord;
    };

    struct FLOLSubMesh 
    {
        FString Name;
        uint32 StartVertex;
        uint32 VertexCount;
        uint32 StartIndex;
        uint32 IndexCount;

        FLOLSubMesh(): Name(""), StartVertex(0), VertexCount(0), StartIndex(0), IndexCount(0) {}
    };

    struct FLOLMesh 
    {
        TArray<FLOLVertex> Vertices;
        TArray<uint16> Indices;
        TArray<FLOLSubMesh> SubMeshes;
    };
}