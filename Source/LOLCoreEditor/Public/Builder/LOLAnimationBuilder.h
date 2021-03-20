#pragma once

#include "CoreMinimal.h"
#include "Type/LOLAnimationAsset.h"

namespace LOLImporter
{
	class LOLCOREEDITOR_API FLOLAnimationBuilder
	{
	public:
		bool BuildAssets(const FLOLAnimationAsset& Asset, TArray<UObject*>& OutAssets);
	};
}