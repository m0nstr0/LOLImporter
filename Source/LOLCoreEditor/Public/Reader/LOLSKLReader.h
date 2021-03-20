// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Type/LOLSkeleton.h"

namespace LOLImporter
{
	class LOLCOREEDITOR_API FLOLSKLReader : public FLOLReader
	{
	private:
		bool ReadNew(FLOLSkeleton& Skeleton);
		bool ReadLegacy(FLOLSkeleton& Skeleton, uint32 Version);
	public:
		FLOLSKLReader(const TArray<uint8>& InData) : FLOLReader::FLOLReader(InData) {};
		~FLOLSKLReader() {};
		bool Read(FLOLSkeleton& Skeleton);
	};
}
