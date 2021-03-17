#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Types/LOLAsset.h"

namespace LOLImporter
{
    class LOLCOREEDITOR_API FLOLSKNReader : public FLOLReader
    {
    public:
        FLOLSKNReader(const TArray<uint8>& InData): FLOLReader::FLOLReader(InData) {};
        bool Read(FLOLAsset& Asset) override;
        virtual ~FLOLSKNReader() {};
    };
}