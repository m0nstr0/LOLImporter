#pragma once

#include "CoreMinimal.h"
#include "Type/LOLLogger.h"
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
    };
}