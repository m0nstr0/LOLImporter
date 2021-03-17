// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Reader/LOLReader.h"
#include "Types/LOLAsset.h"

namespace LOLImporter
{
	class LOLCOREEDITOR_API FLOLSKLReader : public FLOLReader
	{
	public:
		FLOLSKLReader(const TArray<uint8>& InData) : FLOLReader::FLOLReader(InData) {};
		~FLOLSKLReader() {};
		bool ReadNew(FLOLAsset& Asset);
		bool ReadLegacy(FLOLAsset& Asset, uint32 Version);
		bool Read(FLOLAsset& Asset) override;
	};
}
