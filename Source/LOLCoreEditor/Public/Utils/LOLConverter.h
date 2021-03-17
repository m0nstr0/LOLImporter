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
	};
}