// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLANMImportOptions.h"

void ULOLANMImportOptions::PostInitProperties()
{
	Super::PostInitProperties();
	SkeletonForAnimation = nullptr;
}


FText ULOLANMImportOptions::GetErrors()
{
	if (SkeletonForAnimation == nullptr || !SkeletonForAnimation->IsValidLowLevel()) 
	{
		return FText::FromString("Please, select an appropriate skeleton for this animation");
	}

	return FText::GetEmpty();
}