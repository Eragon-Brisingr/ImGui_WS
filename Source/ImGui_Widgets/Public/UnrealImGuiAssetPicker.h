// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiString.h"
#include "UObject/SoftObjectPtr.h"

struct FAssetData;

namespace UnrealImGui
{
	struct FObjectPickerSettings
	{
		const char* FilterHint = "Filter";
		bool bShowDeveloperContent = false;
		bool bShowEngineContent = false;
		bool bShowNonAssetRegistry = false;
		TFunction<bool(const FAssetData&)> CustomFilter;
	};
	struct FObjectPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedAssetClass;
		TArray<FAssetData> CachedAssetList;
	};
	IMGUI_WIDGETS_API bool ComboObjectPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const FAssetData&)>& IsSelectedFunc, const TFunctionRef<void(const FAssetData&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, FObjectPickerSettings* Settings, FObjectPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboObjectPicker(const char* Label, UClass* BaseClass, UObject*& ObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboObjectPicker(const char* Label, T*& ObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data = nullptr)
	{
		return ComboObjectPicker(Label, T::StaticClass(), reinterpret_cast<UObject*&>(ObjectPtr), Settings, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftObjectPicker(const char* Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboSoftObjectPicker(const char* Label, TSoftObjectPtr<T>& SoftObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data = nullptr)
	{
		return ComboSoftObjectPicker(Label, T::StaticClass(), reinterpret_cast<TSoftObjectPtr<UObject>&>(SoftObjectPtr), Settings, Data);
	}

	struct FActorPickerSettings
	{
		const char* FilterHint = "Filter";
		TFunction<bool(const AActor*)> CustomFilter;
	};
	IMGUI_WIDGETS_API extern FActorPickerSettings GActorPickerSettings;
	struct FActorPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedActorClass;
		TArray<TWeakObjectPtr<AActor>> CachedActorList;
	};
	IMGUI_WIDGETS_API bool ComboActorPicker(UWorld* World, const char* Label, const char* PreviewValue, const TSubclassOf<AActor>& BaseClass, const TFunctionRef<bool(AActor*)>& IsSelectedFunc, const TFunctionRef<void(AActor*)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, FActorPickerSettings* Settings = nullptr, FActorPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, AActor*& ActorPtr, FActorPickerSettings* Settings = nullptr, FActorPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<AActor, T>
	bool ComboActorPicker(UWorld* World, const char* Label, T*& ActorPtr, FActorPickerSettings* Settings = nullptr, FActorPickerData* Data = nullptr)
	{
		return ComboActorPicker(World, Label, T::StaticClass(), reinterpret_cast<AActor*&>(ActorPtr), Settings, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr, FActorPickerSettings* Settings = nullptr, FActorPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<AActor, T>
	bool ComboSoftActorPicker(UWorld* World, const char* Label, TSoftObjectPtr<T>& SoftActorPtr, FActorPickerSettings* Settings = nullptr, FActorPickerData* Data = nullptr)
	{
		return ComboSoftActorPicker(World, Label, T::StaticClass(), reinterpret_cast<TSoftObjectPtr<AActor>&>(SoftActorPtr), Settings, Data);
	}

	struct FClassPickerSettings
	{
		const char* FilterHint = "Filter";
		bool bShowDeveloperContent = false;
		bool bShowEngineContent = false;
		TFunction<bool(const UClass*)> CustomFilter;
		TFunction<bool(const FAssetData&)> CustomFilterUnloadBp;
	};
	IMGUI_WIDGETS_API extern FObjectPickerSettings GObjectPickerSettings;
	struct FClassPickerData
	{
		FUTF8String FilterString;
		TWeakObjectPtr<UClass> CachedClass;
		TArray<TSoftClassPtr<UObject>> CachedClassList;
	};
	IMGUI_WIDGETS_API bool ComboClassPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const TSoftClassPtr<UObject>&)>& IsSelectedFunc, const TFunctionRef<void(const TSoftClassPtr<UObject>&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, EClassFlags IgnoreClassFlags = CLASS_Abstract, FClassPickerSettings* Settings = nullptr, FClassPickerData* Data = nullptr);
	IMGUI_WIDGETS_API bool ComboClassPicker(const char* Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, FClassPickerSettings* Settings = nullptr, FClassPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboClassPicker(const char* Label, TSubclassOf<T>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, FClassPickerSettings* Settings = nullptr, FClassPickerData* Data = nullptr)
	{
		return ComboClassPicker(Label, T::StaticClass(), reinterpret_cast<TSubclassOf<UObject>&>(ClassPtr), IgnoreClassFlags, Settings, Data);
	}
	IMGUI_WIDGETS_API bool ComboSoftClassPicker(const char* Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, FClassPickerSettings* Settings = nullptr, FClassPickerData* Data = nullptr);
	template<typename T> requires std::is_base_of_v<UObject, T>
	bool ComboClassPicker(const char* Label, TSoftClassPtr<T>& ClassPtr, EClassFlags IgnoreClassFlags = CLASS_Abstract, FClassPickerSettings* Settings = nullptr, FClassPickerData* Data = nullptr)
	{
		return ComboClassPicker(Label, T::StaticClass(), reinterpret_cast<TSoftClassPtr<UObject>&>(ClassPtr), IgnoreClassFlags, Settings, Data);
	}
}
