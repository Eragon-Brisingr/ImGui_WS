// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiNodeUtils.h"

#include "ImGuiLibrary.h"
#include "UObject/UObjectHash.h"

TArray<UClass*> ImGuiNodeUtils::GetImGuiLibraryClasses()
{
	TArray<UClass*> Classes;
	GetDerivedClasses(UImGuiLibrary::StaticClass(), Classes);
	return Classes;
}
