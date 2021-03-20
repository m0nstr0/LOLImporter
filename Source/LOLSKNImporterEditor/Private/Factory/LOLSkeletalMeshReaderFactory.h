#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Misc/FeedbackContext.h"
#include "Type/LOLSkeletalMeshAsset.h"

namespace LOLImporter
{
	namespace LOLSkeletalMeshReaderFactory
	{
		bool Read(FFeedbackContext* Logger, FLOLSkeletalMeshAsset& Asset, const FString& FilePath);
	}
}