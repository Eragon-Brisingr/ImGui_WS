// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiString.h"
#include "UObject/SoftObjectPtr.h"

struct FAssetData;

namespace UnrealImGui
{
	struct FObjectPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedAssetClass;
		TArray<FAssetData> CachedAssetList;
	};
	IMGUI_WIDGETS_API bool ComboObjectPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const FAssetData&)>& IsSelectedFunc, const TFunctionRef<void(const FAssetData&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, const char* FilterHint = "Filter", FObjectPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboObjectPicker(const char* Label, UClass* BaseClass, UObject*& ObjectPtr, const char* FilterHint = "Filter", FObjectPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboObjectPicker(const char* Label, T*& ObjectPtr, const char* FilterHint = "Filter", FObjectPickerData* Data = nullptr)
	{
		return ComboObjectPicker(Label, T::StaticClass(), ObjectPtr, FilterHint, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftObjectPicker(const char* Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr, const char* FilterHint = "Filter", FObjectPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboSoftObjectPicker(const char* Label, TSoftObjectPtr<T>& SoftObjectPtr, const char* FilterHint = "Filter", FObjectPickerData* Data = nullptr)
	{
		return ComboSoftObjectPicker(Label, T::StaticClass(), reinterpret_cast<TSoftObjectPtr<UObject>&>(SoftObjectPtr), FilterHint, Data);
	}

	struct FActorPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedActorClass;
		TArray<TWeakObjectPtr<AActor>> CachedActorList;
	};
	IMGUI_WIDGETS_API bool ComboActorPicker(UWorld* World, const char* Label, const char* PreviewValue, const TSubclassOf<AActor>& BaseClass, const TFunctionRef<bool(AActor*)>& IsSelectedFunc, const TFunctionRef<void(AActor*)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, const char* FilterHint = "Filter", FActorPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, AActor*& ActorPtr, const char* FilterHint = "Filter", FActorPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<AActor, T>
	bool ComboActorPicker(UWorld* World, const char* Label, T*& ActorPtr, const char* FilterHint = "Filter", FActorPickerData* Data = nullptr)
	{
		return ComboActorPicker(World, Label, T::StaticClass(), ActorPtr, FilterHint, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr, const char* FilterHint = "Filter", FActorPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<AActor, T>
	bool ComboSoftActorPicker(UWorld* World, const char* Label, TSoftObjectPtr<T>& SoftActorPtr, const char* FilterHint = "Filter", FActorPickerData* Data = nullptr)
	{
		return ComboSoftActorPicker(World, Label, T::StaticClass(), reinterpret_cast<TSoftObjectPtr<AActor>&>(SoftActorPtr), FilterHint, Data);
	}

	struct FClassPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedClass;
		TArray<TSoftClassPtr<UObject>> CachedClassList;
	};
	IMGUI_WIDGETS_API bool ComboClassPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const TSoftClassPtr<UObject>&)>& IsSelectedFunc, const TFunctionRef<void(const TSoftClassPtr<UObject>&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, EClassFlags IgnoreClassFlags = CLASS_Abstract, const char* FilterHint = "Filter", FClassPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboClassPicker(const char* Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, const char* FilterHint = "Filter", FClassPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboClassPicker(const char* Label, TSubclassOf<T>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, const char* FilterHint = "Filter", FClassPickerData* Data = nullptr)
	{
		return ComboClassPicker(Label, T::StaticClass(), reinterpret_cast<TSubclassOf<UObject>&>(ClassPtr), IgnoreClassFlags, FilterHint, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftClassPicker(const char* Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, const char* FilterHint = "Filter", FClassPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboClassPicker(const char* Label, TSoftClassPtr<T>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, const char* FilterHint = "Filter", FClassPickerData* Data = nullptr)
	{
		return ComboClassPicker(Label, T::StaticClass(), reinterpret_cast<TSoftClassPtr<UObject>&>(ClassPtr), IgnoreClassFlags, FilterHint, Data);
	}
}
