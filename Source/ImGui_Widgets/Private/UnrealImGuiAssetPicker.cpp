// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiAssetPicker.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Blueprint/BlueprintSupport.h"
#include "Misc/Paths.h"

namespace UnrealImGui
{
	FObjectPickerSettings GObjectPickerSettings;
	FActorPickerSettings GActorPickerSettings;
	FClassPickerSettings GClassPickerSettings;
	
	template<typename TActivatedFunc>
	struct FComboEx : FNoncopyable
	{
		[[nodiscard]]
		FORCEINLINE FComboEx(const char* label, const char* preview_value, TActivatedFunc&& ActivatedFunc)
			: State{ ImGui::BeginCombo(label, preview_value) }
		{
			if (ImGui::IsItemActivated())
			{
				ActivatedFunc();
			}
		}
		FORCEINLINE ~FComboEx() { if (State) { ImGui::EndCombo(); } }
		explicit operator bool() const noexcept { return State; }
	private:
		bool State;
	};
	
	bool FilterByPath(const FString& PackagePath, bool bShowDeveloperContent, bool bShowEngineContent)
	{
		if (!bShowDeveloperContent)
		{
			static const FString DeveloperPath = FPackageName::FilenameToLongPackageName(FPaths::GameDevelopersDir());
			if (PackagePath.StartsWith(DeveloperPath))
			{
				return true;
			}
		}
		if (!bShowEngineContent)
		{
			if (PackagePath.StartsWith(TEXT("/Engine")))
			{
				return true;
			}
		}
		return false;
	}
	
	bool ComboObjectPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const FAssetData&)>& IsSelectedFunc, const TFunctionRef<void(const FAssetData&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, FObjectPickerSettings* Settings, FObjectPickerData* Data)
	{
		if (Settings == nullptr)
		{
			Settings = &GObjectPickerSettings;
		}
		if (Data == nullptr)
		{
			static FObjectPickerData GlobalData;
			Data = &GlobalData;
		}
		auto& FilterString = Data->FilterString;
		auto& CachedAssetClass = Data->CachedAssetClass;
		auto& CachedAssetList = Data->CachedAssetList;
		bool bValueChanged = false;
		if (auto Combo = FComboEx(Label, PreviewValue, [&]
		{
			FilterString.Reset();
			CachedAssetClass = nullptr;
			CachedAssetList.Empty();
		}))
		{
			ImGui::FIdScope IdScope{ Label };
			ImGui::SetNextItemWidth(-ImGui::GetFontSize() * 7.f);
			ImGui::InputTextWithHint("##Filter", Settings->FilterHint, FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
			bool IsFiltered = ImGui::IsItemEdited();
			ImGui::SameLine();
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::Checkbox("Show Developer Content", &Settings->bShowDeveloperContent))
				{
					IsFiltered |= ImGui::IsItemEdited();
				}
				if (ImGui::Checkbox("Show Engine Content", &Settings->bShowEngineContent))
				{
					IsFiltered |= ImGui::IsItemEdited();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::Selectable("Clear"))
			{
				OnClearValue();
				bValueChanged = true;
			}

			if (CachedAssetClass != BaseClass || IsFiltered)
			{
				CachedAssetClass = BaseClass;
				CachedAssetList.Reset();
				static IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
				FARFilter ARFilter;
				ARFilter.bRecursivePaths = true;
				ARFilter.bRecursiveClasses = true;
				ARFilter.ClassPaths.Add(FTopLevelAssetPath{ BaseClass });
				AssetRegistry.GetAssets(ARFilter, CachedAssetList);
				const FString Filter = FilterString.ToString();
				CachedAssetList.RemoveAllSwap([&](const FAssetData& E)
				{
					if (Filter.IsEmpty() == false)
					{
						if (E.AssetName.ToString().Contains(Filter) == false)
						{
							return true;
						}
					}
					if (Settings->CustomFilter)
					{
						if (Settings->CustomFilter(E))
						{
							return true;
						}
					}
					if (FilterByPath(E.PackagePath.ToString(), Settings->bShowDeveloperContent, Settings->bShowEngineContent))
					{
						return true;
					}
					return false;
				});

				if (Settings->bShowNonAssetRegistry)
				{
					const TSet<FAssetData> ExistAssetList{ CachedAssetList };
					TArray<UObject*> Objects;
					GetObjectsOfClass(BaseClass, Objects, true, RF_ClassDefaultObject | RF_ArchetypeObject | RF_NewerVersionExists);
					for (UObject* Object : Objects)
					{
						const FAssetData Asset{ Object };
						if (ExistAssetList.Contains(Asset))
						{
							continue;
						}
						if (Filter.IsEmpty() == false && Asset.AssetName.ToString().Contains(Filter) == false)
						{
							continue;
						}
						if (Settings->CustomFilter && Settings->CustomFilter(Asset))
						{
							continue;
						}
						CachedAssetList.Add(Asset);
					}
				}

				CachedAssetList.Sort([](const FAssetData& LHS, const FAssetData& RHS)
				{
					return LHS.AssetName.FastLess(RHS.AssetName);
				});
			}
			if (auto ListBox = ImGui::FListBox("##Content", { -1, -1 }))
			{
				ImGuiListClipper ListClipper{};
				ListClipper.Begin(CachedAssetList.Num());
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						const FAssetData& Asset = CachedAssetList[Idx];
						const bool IsSelected = IsSelectedFunc(Asset);
						if (auto Obj = Asset.FastGetAsset())
						{
							ImGui::PushID(Obj);
						}
						else
						{
							ImGui::PushID(Asset.PackageName.GetComparisonIndex().ToUnstableInt());
						}
						if (ImGui::Selectable(TCHAR_TO_UTF8(*Asset.AssetName.ToString()), IsSelected))
						{
							OnSetValue(Asset);
							bValueChanged = true;
						}
						ImGui::PopID();
						if (IsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

						if (ImGui::BeginItemTooltip())
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Asset.GetSoftObjectPath().ToString()));
							ImGui::EndTooltip();
						}
					}
				}
				ListClipper.End();
			}
			if (bValueChanged)
			{
				ImGui::CloseCurrentPopup();
			}
		}
		return bValueChanged;
	}

	bool ComboObjectPicker(const char* Label, UClass* BaseClass, UObject*& ObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data)
	{
		const bool bValueChanged = ComboObjectPicker(Label, ObjectPtr ? TCHAR_TO_UTF8(*ObjectPtr->GetName()) : "None", BaseClass,
		[ObjectPath = FSoftObjectPath{ ObjectPtr }](const FAssetData& Asset)
		{
			return ObjectPath == Asset.GetSoftObjectPath();
		}, [&](const FAssetData& Asset)
		{
			ObjectPtr = Asset.GetAsset();
		}, [&]
		{
			ObjectPtr = nullptr;	
		}, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(ObjectPtr ? TCHAR_TO_UTF8(*ObjectPtr->GetPathName()) : "None");
		}
		return bValueChanged;
	}
	
	bool ComboSoftObjectPicker(const char* Label, UClass* BaseClass, TSoftObjectPtr<UObject>& SoftObjectPtr, FObjectPickerSettings* Settings, FObjectPickerData* Data)
	{
		const bool bValueChanged = ComboObjectPicker(Label, SoftObjectPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftObjectPtr.GetAssetName()), BaseClass,
			[&](const FAssetData& Asset)
			{
				return SoftObjectPtr.GetUniqueID() == Asset.GetSoftObjectPath();
			}, [&](const FAssetData& Asset)
			{
				SoftObjectPtr = Asset.GetSoftObjectPath();
			}, [&]
			{
				SoftObjectPtr.Reset();	
			}, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(SoftObjectPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftObjectPtr.ToString()));
		}
		return bValueChanged;
	}

	bool ComboActorPicker(UWorld* World, const char* Label, const char* PreviewValue, const TSubclassOf<AActor>& BaseClass, const TFunctionRef<bool(AActor*)>& IsSelectedFunc, const TFunctionRef<void(AActor*)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, FActorPickerSettings* Settings, FActorPickerData* Data)
	{
		if (Settings == nullptr)
		{
			Settings = &GActorPickerSettings;
		}
		if (Data == nullptr)
		{
			static FActorPickerData GlobalData;
			Data = &GlobalData;
		}
		auto& FilterString = Data->FilterString;
		auto& CachedActorClass = Data->CachedActorClass;
		auto& CachedActorList = Data->CachedActorList;
		bool bValueChanged = false;
		if (auto Combo = FComboEx(Label, PreviewValue, [&]
		{
			FilterString.Reset();
			CachedActorClass = nullptr;
			CachedActorList.Empty();
		}))
		{
			ImGui::FIdScope IdScope{ Label };
			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextWithHint("##Filter", Settings->FilterHint, FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
			const bool IsFiltered = ImGui::IsItemEdited();
			ImGui::Separator();
			if (ImGui::Selectable("Clear"))
			{
				OnClearValue();
				bValueChanged = true;
			}

			if (CachedActorClass != BaseClass.Get() || IsFiltered)
			{
				CachedActorClass = BaseClass;
				CachedActorList.Reset();
				const FString Filter = FilterString.ToString();
				for (TActorIterator<AActor> It(World, BaseClass); It; ++It)
				{
					AActor* Actor = *It;
					if (Actor == nullptr)
					{
						continue;
					}
					if (Filter.Len() > 0 && Actor->GetName().Contains(Filter) == false)
					{
						continue;
					}
					if (Settings->CustomFilter && Settings->CustomFilter(Actor))
					{
						continue;
					}
					CachedActorList.Add(Actor);
				}
			}
			if (auto ListBox = ImGui::FListBox("##Content", { -1, -1 }))
			{
				ImGuiListClipper ListClipper{};
				ListClipper.Begin(CachedActorList.Num());
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						if (AActor* Actor = CachedActorList[Idx].Get())
						{
							const bool IsSelected = IsSelectedFunc(Actor);
							ImGui::FIdScope ActorIdScope{ Actor };
							if (ImGui::Selectable(TCHAR_TO_UTF8(*Actor->GetName()), IsSelected))
							{
								OnSetValue(Actor);
								bValueChanged = true;
							}
							if (IsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}

							if (ImGui::BeginItemTooltip())
							{
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetFullName()));
								ImGui::EndTooltip();
							}
						}
						else
						{
							ImGui::BeginDisabled(true);
							ImGui::TextUnformatted("Deleted");
							ImGui::EndDisabled();
						}
					}
				}
				ListClipper.End();
			}
			if (bValueChanged)
			{
				ImGui::CloseCurrentPopup();
			}
		}
		return bValueChanged;
	}

	bool ComboActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, AActor*& ActorPtr, FActorPickerSettings* Settings, FActorPickerData* Data)
	{
		const bool bValueChanged = ComboActorPicker(World, Label, ActorPtr ? TCHAR_TO_UTF8(*ActorPtr->GetName()) : "None", BaseClass,
			[&](const AActor* Actor)
			{
				return ActorPtr == Actor;
			}, [&](AActor* Actor)
			{
				ActorPtr = Actor;
			}, [&]
			{
				ActorPtr = nullptr;	
			}, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(ActorPtr ? TCHAR_TO_UTF8(*ActorPtr->GetPathName()) : "None");
		}
		return bValueChanged;
	}

	bool ComboSoftActorPicker(UWorld* World, const char* Label, const TSubclassOf<AActor>& BaseClass, TSoftObjectPtr<AActor>& SoftActorPtr, FActorPickerSettings* Settings, FActorPickerData* Data)
	{
		const bool bValueChanged = ComboActorPicker(World, Label, SoftActorPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftActorPtr.GetAssetName()), BaseClass,
			[&](const AActor* Actor)
			{
				return SoftActorPtr == Actor;
			}, [&](AActor* Actor)
			{
				SoftActorPtr = Actor;
			}, [&]
			{
				SoftActorPtr = nullptr;	
			}, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(SoftActorPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftActorPtr.ToString()));
		}
		return bValueChanged;
	}

	// same as UAssetRegistryHelpers::GetBlueprintAssets, if UAssetRegistryHelpers::GetBlueprintAssets mark ASSETREGISTRY_API replace this
	void GetBlueprintAssets(const FTopLevelAssetPath& BaseClass, TArray<FAssetData>& OutAssetData)
	{
		IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();

		FARFilter Filter;
		Filter.bRecursiveClasses = true;
		Filter.ClassPaths.Add(BaseClass);
		// Expand list of classes to include derived classes
		TArray<FTopLevelAssetPath> BlueprintParentClassPathRoots = MoveTemp(Filter.ClassPaths);
		TSet<FTopLevelAssetPath> BlueprintParentClassPaths;
		if (Filter.bRecursiveClasses)
		{
			AssetRegistry.GetDerivedClassNames(BlueprintParentClassPathRoots, TSet<FTopLevelAssetPath>(), BlueprintParentClassPaths);
		}
		else
		{
			BlueprintParentClassPaths.Append(BlueprintParentClassPathRoots);
		}

		// Search for all blueprints and then check BlueprintParentClassPaths in the results
		Filter.ClassPaths.Reset(1);
		Filter.ClassPaths.Add(FTopLevelAssetPath(FName(TEXT("/Script/Engine")), FName(TEXT("BlueprintCore"))));
		Filter.bRecursiveClasses = true;

		auto FilterLambda = [&OutAssetData, &BlueprintParentClassPaths](const FAssetData& AssetData)
		{
			// Verify blueprint class
			if (BlueprintParentClassPaths.IsEmpty() || UAssetRegistryHelpers::IsAssetDataBlueprintOfClassSet(AssetData, BlueprintParentClassPaths))
			{
				OutAssetData.Add(AssetData);
			}
			return true;
		};
		AssetRegistry.EnumerateAssets(Filter, FilterLambda);
	}

	bool ComboClassPicker(const char* Label, const char* PreviewValue, UClass* BaseClass, const TFunctionRef<bool(const TSoftClassPtr<UObject>&)>& IsSelectedFunc, const TFunctionRef<void(const TSoftClassPtr<UObject>&)>& OnSetValue, const TFunctionRef<void()>& OnClearValue, EClassFlags IgnoreClassFlags, FClassPickerSettings* Settings, FClassPickerData* Data)
	{
		if (Settings == nullptr)
		{
			Settings = &GClassPickerSettings;
		}
		if (Data == nullptr)
		{
			static FClassPickerData GlobalData;
			Data = &GlobalData;
		}
		IgnoreClassFlags |= CLASS_Deprecated | CLASS_NewerVersionExists;
		auto& FilterString = Data->FilterString;
		auto& CachedClass = Data->CachedClass;
		auto& CachedClassList = Data->CachedClassList;
		bool bValueChanged = false;
		if (auto Combo = FComboEx(Label, PreviewValue, [&]
		{
			FilterString.Reset();
			CachedClass = nullptr;
			CachedClassList.Empty();
		}))
		{
			ImGui::FIdScope IdScope{ Label };
			ImGui::SetNextItemWidth(-ImGui::GetFontSize() * 7.f);
			ImGui::InputTextWithHint("##Filter", Settings->FilterHint, FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
			bool IsFiltered = ImGui::IsItemEdited();
			ImGui::SameLine();
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::Checkbox("Show Developer Content", &Settings->bShowDeveloperContent))
				{
					IsFiltered |= ImGui::IsItemEdited();
				}
				if (ImGui::Checkbox("Show Engine Content", &Settings->bShowEngineContent))
				{
					IsFiltered |= ImGui::IsItemEdited();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::Selectable("Clear"))
			{
				OnClearValue();
				bValueChanged = true;
			}
			if (CachedClass != BaseClass || IsFiltered)
			{
				CachedClass = BaseClass;
				CachedClassList.Reset();
				
				TSet<TSoftClassPtr<UObject>> AllClasses;
				TArray<UClass*> LoadedClasses;
				GetDerivedClasses(BaseClass, LoadedClasses);
				LoadedClasses.Add(BaseClass);
				const FString Filter = FilterString.ToString();
				for (UClass* Class : LoadedClasses)
				{
					if (Class->HasAnyClassFlags(IgnoreClassFlags))
					{
						continue;
					}
#if WITH_EDITOR
					if (Class->GetName().StartsWith(TEXT("SKEL_")))
					{
						continue;
					}
#endif
					if (Filter.Len() > 0 && Class->GetName().Contains(Filter) == false)
					{
						continue;
					}
					if (Settings->CustomFilter && Settings->CustomFilter(Class))
					{
						continue;
					}
					if (FilterByPath(Class->GetPackage()->GetName(), Settings->bShowDeveloperContent, Settings->bShowEngineContent))
					{
						continue;
					}
					AllClasses.Add(Class);
				}

				TArray<FAssetData> BlueprintClasses;
				GetBlueprintAssets(FTopLevelAssetPath{ BaseClass }, BlueprintClasses);
				for (const auto& Asset : BlueprintClasses)
				{
					uint32 ClassFlags;
					if (!Asset.GetTagValue(FBlueprintTags::ClassFlags, ClassFlags))
					{
						continue;
					}
					if (EnumHasAnyFlags((EClassFlags)ClassFlags, IgnoreClassFlags))
					{
						continue;
					}
					FString GeneratedClass;
					if (!Asset.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClass))
					{
						continue;
					}
					if (Filter.Len() > 0 && Asset.AssetName.ToString().Contains(Filter) == false)
					{
						continue;
					}
					if (Settings->CustomFilterUnloadBp && Settings->CustomFilterUnloadBp(Asset))
					{
						continue;
					}
					if (FilterByPath(Asset.PackagePath.ToString(), Settings->bShowDeveloperContent, Settings->bShowEngineContent))
					{
						continue;
					}
					AllClasses.Add(TSoftClassPtr<UObject>{ FSoftClassPath{ GeneratedClass } });
				}

				CachedClassList = AllClasses.Array();
				CachedClassList.Sort([](const TSoftClassPtr<UObject>& LHS, const TSoftClassPtr<UObject>& RHS)
				{
					return LHS.ToSoftObjectPath().FastLess(RHS.ToSoftObjectPath());
				});
			}
			if (auto ListBox = ImGui::FListBox("##Content", { -1, -1 }))
			{
				ImGuiListClipper ListClipper{};
				ListClipper.Begin(CachedClassList.Num());
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						auto& Class = CachedClassList[Idx];
						const bool IsSelected = IsSelectedFunc(Class);
						ImGui::FIdScope ClassIdScope{ (int32)Class.ToSoftObjectPath().GetAssetPath().GetPackageName().GetComparisonIndex().ToUnstableInt() };
						if (ImGui::Selectable(TCHAR_TO_UTF8(*Class.GetAssetName()), IsSelected))
						{
							OnSetValue(Class);
							bValueChanged = true;
						}
						if (IsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

						if (ImGui::BeginItemTooltip())
						{
							ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class.ToString()));
							ImGui::EndTooltip();
						}
					}
				}
				ListClipper.End();
			}
			if (bValueChanged)
			{
				ImGui::CloseCurrentPopup();
			}
		}
		return bValueChanged;
	}

	bool ComboClassPicker(const char* Label, UClass* BaseClass, TSubclassOf<UObject>& ClassPtr, EClassFlags IgnoreClassFlags, FClassPickerSettings* Settings, FClassPickerData* Data)
	{
		const bool bValueChanged = ComboClassPicker(Label, ClassPtr ? TCHAR_TO_UTF8(*ClassPtr->GetName()) : "None", BaseClass,
			[&](const TSoftClassPtr<UObject>& Class)
			{
				return ClassPtr == Class.Get();
			}, [&](const TSoftClassPtr<UObject>& Class)
			{
				ClassPtr = Class.LoadSynchronous();
			}, [&]
			{
				ClassPtr = nullptr;	
			}, IgnoreClassFlags, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(ClassPtr ? TCHAR_TO_UTF8(*ClassPtr->GetPathName()) : "None");
		}
		return bValueChanged;
	}

	bool ComboSoftClassPicker(const char* Label, UClass* BaseClass, TSoftClassPtr<UObject>& SoftClassPtr, EClassFlags IgnoreClassFlags, FClassPickerSettings* Settings, FClassPickerData* Data)
	{
		const bool bValueChanged = ComboClassPicker(Label, SoftClassPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftClassPtr.GetAssetName()), BaseClass,
			[&](const TSoftClassPtr<UObject>& Class)
			{
				return SoftClassPtr == Class;
			}, [&](const TSoftClassPtr<UObject>& Class)
			{
				SoftClassPtr = Class;
			}, [&]
			{
				SoftClassPtr = nullptr;	
			}, IgnoreClassFlags, Settings, Data);
		if (auto Tooltip = ImGui::FItemTooltip())
		{
			ImGui::TextUnformatted(SoftClassPtr.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*SoftClassPtr.ToString()));
		}
		return bValueChanged;
	}
}
