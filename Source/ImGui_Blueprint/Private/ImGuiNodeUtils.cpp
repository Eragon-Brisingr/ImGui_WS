// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiNodeUtils.h"

#include "ImGuiLibraryBase.h"
#include "UObject/UObjectHash.h"

TArray<UClass*> ImGuiNodeUtils::GetImGuiLibraryClasses()
{
	TArray<UClass*> Classes;
	GetDerivedClasses(UImGuiLibraryBase::StaticClass(), Classes);
	return Classes;
}
