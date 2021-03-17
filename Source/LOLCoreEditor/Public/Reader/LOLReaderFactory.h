#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Misc/FeedbackContext.h"

namespace LOLImporter
{
    class LOLCOREEDITOR_API FLOLReaderFactory
    {
    public:
        static bool ReadFile(FFeedbackContext* Logger, FLOLAsset& Asset, const FString& FilePath);
    };
}