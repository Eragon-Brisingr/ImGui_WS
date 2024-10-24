// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiAssetPicker.h"
#include "UnrealImGuiPropertyDetails.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UnrealImGuiLibrary.generated.h"

UCLASS()
class IMGUI_WIDGETS_API UUnrealImGuiLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboObjectPicker(FName Label, UClass* BaseClass, UObject*& ObjectPtr, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ObjectPtr, TCHAR_TO_UTF8(*FilterHint));
	}

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftObjectPicker(FName Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboSoftObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftObjectPtr, TCHAR_TO_UTF8(*FilterHint));
	}
	
	UFUNCTION(BlueprintCallable, Category = "ImGui", meta = (WorldContext = WorldContextObject))
	static bool ComboActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, AActor*& ActorPtr, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboActorPicker(WorldContextObject->GetWorld(), TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ActorPtr, TCHAR_TO_UTF8(*FilterHint));
	}

	UFUNCTION(BlueprintCallable, Category = "ImGui", meta = (WorldContext = WorldContextObject))
	static bool ComboSoftActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboSoftActorPicker(WorldContextObject->GetWorld(), TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftActorPtr, TCHAR_TO_UTF8(*FilterHint));
	}

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboClassPicker(FName Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, bool bAllowAbstract = false, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, TCHAR_TO_UTF8(*FilterHint));
	}

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftClassPicker(FName Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, bool bAllowAbstract = false, const FString& FilterHint = TEXT("Filter"))
	{
		return UnrealImGui::ComboSoftClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, TCHAR_TO_UTF8(*FilterHint));
	}
	
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static void DrawObjectDetailTable(FName Label, UObject* Object)
	{
		if (Object)
		{
			UnrealImGui::DrawDetailTable(TCHAR_TO_UTF8(*Label.ToString()), Object->GetClass(), { Object });
		}
	}
};
