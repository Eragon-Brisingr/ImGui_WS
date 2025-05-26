// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ImGuiLibraryBase.h"
#include "UnrealImGuiComboEnum.h"
#include "UnrealImGuiPropertyDetails.h"
#include "AssetRegistry/AssetData.h"
#include "Templates/SubclassOf.h"
#include "UnrealImGuiLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FUnrealImGuiObjectPickerFilter, const FAssetData&, Asset);

USTRUCT(BlueprintType)
struct FUnrealImGuiObjectPickerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FString FilterHint = TEXT("Filter");

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FUnrealImGuiObjectPickerFilter Filter;
};

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FUnrealImGuiActorPickerFilter, const AActor*, Actor);

USTRUCT(BlueprintType)
struct FUnrealImGuiActorPickerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FString FilterHint = TEXT("Filter");

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FUnrealImGuiActorPickerFilter Filter;
};

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FUnrealImGuiClassPickerFilter, const UClass*, Class);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FUnrealImGuiClassPickerUnloadBpFilter, const FAssetData&, UnloadClass);

USTRUCT(BlueprintType)
struct FUnrealImGuiClassPickerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FString FilterHint = TEXT("Filter");
	
	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FUnrealImGuiClassPickerFilter Filter;

	UPROPERTY(BlueprintReadWrite, Category = ImGui)
	FUnrealImGuiClassPickerUnloadBpFilter FilterUnloadBp;
};

UCLASS()
class IMGUI_WIDGETS_API UUnrealImGuiLibrary : public UImGuiLibraryBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboObjectPicker(FName Label, UClass* BaseClass, UPARAM(Ref)UObject*& ObjectPtr);
	
	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftObjectPicker(FName Label, UClass* BaseClass, UPARAM(Ref)TSoftObjectPtr<UObject>& SoftObjectPtr);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboObjectPickerEx(FName Label, UClass* BaseClass, UPARAM(Ref)UObject*& ObjectPtr, const FUnrealImGuiObjectPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftObjectPickerEx(FName Label, UClass* BaseClass, UPARAM(Ref)TSoftObjectPtr<UObject>& SoftObjectPtr, const FUnrealImGuiObjectPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, UPARAM(Ref)AActor*& ActorPtr);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, UPARAM(Ref)TSoftObjectPtr<AActor>& SoftActorPtr);

	UFUNCTION(BlueprintCallable, Category = "ImGui", meta = (WorldContext = WorldContextObject))
	static bool ComboActorPickerEx(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, UPARAM(Ref)AActor*& ActorPtr, const FUnrealImGuiActorPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui", meta = (WorldContext = WorldContextObject))
	static bool ComboSoftActorPickerEx(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, UPARAM(Ref)TSoftObjectPtr<AActor>& SoftActorPtr, const FUnrealImGuiActorPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboClassPicker(FName Label, UClass* BaseClass, UPARAM(Ref)TSubclassOf<UObject>& ClassPtr, bool bAllowAbstract = false);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftClassPicker(FName Label, UClass* BaseClass, UPARAM(Ref)TSoftClassPtr<UObject>& SoftClassPtr, bool bAllowAbstract = false);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboClassPickerEx(FName Label, UClass* BaseClass, UPARAM(Ref)TSubclassOf<UObject>& ClassPtr, bool bAllowAbstract, const FUnrealImGuiClassPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboSoftClassPickerEx(FName Label, UClass* BaseClass, UPARAM(Ref)TSoftClassPtr<UObject>& SoftClassPtr, bool bAllowAbstract, const FUnrealImGuiClassPickerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool ComboEnum(FName Label, uint8& EnumValue, UEnum* EnumType, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ImGui.EImGuiComboFlags"))int32 Flags = 0);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static void DrawObjectDetailTable(FName Label, UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "ImGui")
	static bool DrawSingleProperty(FName PropertyName, UObject* Object);
};
