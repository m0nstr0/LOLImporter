// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace LOLImporter 
{
    namespace FLOLConverter 
    {
        inline FVector ConvertVector(const FVector& Vec)
        {
            return { Vec.X, Vec.Z, Vec.Y };
        }

        inline FQuat ConvertQuat(const FQuat& Quat)
        {
            return { -Quat.X, -Quat.Z, -Quat.Y, Quat.W };
        }

		uint32 ElfHash(FString StringToHash)
		{
			FString LowerStringToHash = StringToHash.ToLower();

			uint32 Hash = 0;
			uint32 High = 0;
			for (int32 i = 0; i < LowerStringToHash.Len(); i++) {
				Hash = (Hash << 4) + ((uint8)LowerStringToHash[i]);

				if ((High = Hash & 0xF0000000) != 0) {
					Hash ^= (High >> 24);
				}

				Hash &= ~High;
			}

			return Hash;
		}
	};
}