#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Type/LOLMesh.h"

namespace LOLImporter
{
    class LOLCOREEDITOR_API FLOLSKNReader : public FLOLReader
    {
    public:
        FLOLSKNReader(const TArray<uint8>& InData): FLOLReader::FLOLReader(InData) {};
        bool Read(FLOLMesh& Mesh);
        virtual ~FLOLSKNReader() {};
    };
}