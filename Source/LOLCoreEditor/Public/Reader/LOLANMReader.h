#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Type/LOLAnimation.h"

namespace LOLImporter
{
    class LOLCOREEDITOR_API FLOLANMReader : public FLOLReader
    {
    private:
		FVector DecompressVector(FVector Min, FVector Max, uint8 CompressedData[6]);
		FQuat DecompressQuantizedQuaternion(uint8 CompressedTransform[6]);
		bool ReadCompressed(FLOLAnimation& Animation);
    public:
        FLOLANMReader(const TArray<uint8>& InData): FLOLReader::FLOLReader(InData) {};
        bool Read(FLOLAnimation& Animation);
        virtual ~FLOLANMReader() {};
    };
}