#pragma once

#include "CoreMinimal.h"
#include "Types/LOLLogger.h"
#include "Types/LOLAsset.h"
#include "Serialization/MemoryReader.h"

namespace LOLImporter
{
    class FLOLReader : public FLOLLogger
    {
    private:
        FMemoryReader Reader;
    public:
        FLOLReader(const TArray<uint8>& InData) : Reader(InData) {}
        virtual ~FLOLReader() {}
        FMemoryReader& GetReader() { return Reader; }
        virtual bool Read(FLOLAsset& Asset) = 0;
    };
}