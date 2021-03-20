#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Misc/FeedbackContext.h"
#include "Type/LOLAnimationAsset.h"

namespace LOLImporter
{
	namespace LOLAnimationReaderFactory
	{
		bool Read(FFeedbackContext* Logger, FLOLAnimationAsset& Asset, const FString& FilePath);
	}
}