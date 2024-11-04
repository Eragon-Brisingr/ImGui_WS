// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiLibrary.h"

#include "UnrealImGuiAssetPicker.h"

namespace UnrealImGuiLibrary
{
	UnrealImGui::FObjectPickerSettings ObjectPickerSettings;
	UnrealImGui::FActorPickerSettings ActorPickerSettings;
	UnrealImGui::FClassPickerSettings ClassPickerSettings;

	struct FObjectPickerSettingsScope
	{
		FObjectPickerSettingsScope(const FUnrealImGuiObjectPickerSettings& Settings)
			: FilterHint(ObjectPickerSettings.FilterHint)
			, CustomFilter(MoveTemp(ObjectPickerSettings.CustomFilter))
		{
			ObjectPickerSettings.FilterHint = TCHAR_TO_UTF8(*Settings.FilterHint);
			if (Settings.Filter.IsBound())
			{
				ObjectPickerSettings.CustomFilter = [Filter = Settings.Filter](const FAssetData& Asset)
				{
					return Filter.Execute(Asset);
				};
			}
			else
			{
				ObjectPickerSettings.CustomFilter.Reset();
			}
		}
		~FObjectPickerSettingsScope()
		{
			ObjectPickerSettings.FilterHint = FilterHint;
			ObjectPickerSettings.CustomFilter = MoveTemp(CustomFilter);
		}
		
		const char* FilterHint;
		TFunction<bool(const FAssetData&)> CustomFilter;
	};

	struct FActorPickerSettingsScope
	{
		FActorPickerSettingsScope(const FUnrealImGuiActorPickerSettings& Settings)
			: FilterHint(ActorPickerSettings.FilterHint)
			, CustomFilter(MoveTemp(ActorPickerSettings.CustomFilter))
		{
			ActorPickerSettings.FilterHint = TCHAR_TO_UTF8(*Settings.FilterHint);
			if (Settings.Filter.IsBound())
			{
				ActorPickerSettings.CustomFilter = [Filter = Settings.Filter](const AActor* Actor)
				{
					return Filter.Execute(Actor);
				};
			}
			else
			{
				ActorPickerSettings.CustomFilter.Reset();
			}
		}
		~FActorPickerSettingsScope()
		{
			ActorPickerSettings.FilterHint = FilterHint;
			ActorPickerSettings.CustomFilter = MoveTemp(CustomFilter);
		}
		
		const char* FilterHint;
		TFunction<bool(const AActor*)> CustomFilter;
	};

	struct FClassPickerSettingsScope
	{
		FClassPickerSettingsScope(const FUnrealImGuiClassPickerSettings& Settings)
			: FilterHint(ClassPickerSettings.FilterHint)
			, CustomFilter(MoveTemp(ClassPickerSettings.CustomFilter))
			, CustomFilterUnloadBp(MoveTemp(ClassPickerSettings.CustomFilterUnloadBp))
		{
			ClassPickerSettings.FilterHint = TCHAR_TO_UTF8(*Settings.FilterHint);
			if (Settings.Filter.IsBound())
			{
				ClassPickerSettings.CustomFilter = [Filter = Settings.Filter](const UClass* Class)
				{
					return Filter.Execute(Class);
				};
			}
			else
			{
				ClassPickerSettings.CustomFilter.Reset();
			}
		
			if (Settings.FilterUnloadBp.IsBound())
			{
				ClassPickerSettings.CustomFilterUnloadBp = [Filter = Settings.FilterUnloadBp](const FAssetData& Asset)
				{
					return Filter.Execute(Asset);
				};
			}
			else
			{
				ClassPickerSettings.CustomFilterUnloadBp.Reset();
			}
		}
		~FClassPickerSettingsScope()
		{
			ClassPickerSettings.FilterHint = FilterHint;
			ClassPickerSettings.CustomFilter = MoveTemp(CustomFilter);
			ClassPickerSettings.CustomFilterUnloadBp = MoveTemp(CustomFilterUnloadBp);
		}

		const char* FilterHint;
		TFunction<bool(const UClass*)> CustomFilter;
		TFunction<bool(const FAssetData&)> CustomFilterUnloadBp;
	};
}

bool UUnrealImGuiLibrary::ComboObjectPicker(FName Label, UClass* BaseClass, UObject*& ObjectPtr)
{
	return UnrealImGui::ComboObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ObjectPtr, &UnrealImGuiLibrary::ObjectPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftObjectPicker(FName Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr)
{
	return UnrealImGui::ComboSoftObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftObjectPtr, &UnrealImGuiLibrary::ObjectPickerSettings);
}

bool UUnrealImGuiLibrary::ComboObjectPickerEx(FName Label, UClass* BaseClass, UObject*& ObjectPtr, const FUnrealImGuiObjectPickerSettings& Settings)
{
	auto Scope = UnrealImGuiLibrary::FObjectPickerSettingsScope{ Settings };
	return UnrealImGui::ComboObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ObjectPtr, &UnrealImGuiLibrary::ObjectPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftObjectPickerEx(FName Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr, const FUnrealImGuiObjectPickerSettings& Settings)
{
	auto Scope = UnrealImGuiLibrary::FObjectPickerSettingsScope{ Settings };
	return UnrealImGui::ComboSoftObjectPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftObjectPtr, &UnrealImGuiLibrary::ObjectPickerSettings);
}

bool UUnrealImGuiLibrary::ComboActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, AActor*& ActorPtr)
{
	return UnrealImGui::ComboActorPicker(WorldContextObject ? WorldContextObject->GetWorld() : nullptr, TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ActorPtr, &UnrealImGuiLibrary::ActorPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftActorPicker(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr)
{
	return UnrealImGui::ComboSoftActorPicker(WorldContextObject ? WorldContextObject->GetWorld() : nullptr, TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftActorPtr, &UnrealImGuiLibrary::ActorPickerSettings);
}

bool UUnrealImGuiLibrary::ComboActorPickerEx(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, AActor*& ActorPtr, const FUnrealImGuiActorPickerSettings& Settings)
{
	auto Scope = UnrealImGuiLibrary::FActorPickerSettingsScope{ Settings };
	return UnrealImGui::ComboActorPicker(WorldContextObject ? WorldContextObject->GetWorld() : nullptr, TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ActorPtr, &UnrealImGuiLibrary::ActorPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftActorPickerEx(UObject* WorldContextObject, FName Label, TSubclassOf<AActor> BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr, const FUnrealImGuiActorPickerSettings& Settings)
{
	auto Scope = UnrealImGuiLibrary::FActorPickerSettingsScope{ Settings };
	return UnrealImGui::ComboSoftActorPicker(WorldContextObject ? WorldContextObject->GetWorld() : nullptr, TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftActorPtr, &UnrealImGuiLibrary::ActorPickerSettings);
}

bool UUnrealImGuiLibrary::ComboClassPicker(FName Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, bool bAllowAbstract)
{
	return UnrealImGui::ComboClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, &UnrealImGuiLibrary::ClassPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftClassPicker(FName Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, bool bAllowAbstract)
{
	return UnrealImGui::ComboSoftClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, &UnrealImGuiLibrary::ClassPickerSettings);
}

bool UUnrealImGuiLibrary::ComboClassPickerEx(FName Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, bool bAllowAbstract, const FUnrealImGuiClassPickerSettings& Settings)
{
	auto Scope = UnrealImGuiLibrary::FClassPickerSettingsScope{ Settings };
	return UnrealImGui::ComboClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, ClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, &UnrealImGuiLibrary::ClassPickerSettings);
}

bool UUnrealImGuiLibrary::ComboSoftClassPickerEx(FName Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, bool bAllowAbstract, const FUnrealImGuiClassPickerSettings& Settings)
{
	return UnrealImGui::ComboSoftClassPicker(TCHAR_TO_UTF8(*Label.ToString()), BaseClass, SoftClassPtr, bAllowAbstract ? CLASS_Abstract : CLASS_None, &UnrealImGuiLibrary::ClassPickerSettings);
}
