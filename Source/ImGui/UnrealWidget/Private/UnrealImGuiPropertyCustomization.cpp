// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPropertyCustomization.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiWrapper.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/TextProperty.h"

namespace UnrealImGui
{
	bool bPropertyDisable = false;
	int32 PropertyDisableCounter = 0;
	FPropertyDisableScope::FPropertyDisableScope(const FProperty* Property)
	{
		if (GlobalValue::GEnableEditVisibleProperty)
		{
			Disable = false;
		}
		else if (Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance))
		{
			Disable = true;
		}
		else if (Property->HasAllPropertyFlags(CPF_Edit) == false)
		{
			Disable = true;
		}
		if (Disable)
		{
			if (bPropertyDisable == false)
			{
				bPropertyDisable = true;
				ImGui::BeginDisabled(true);
			}
			PropertyDisableCounter += 1;
		}
	}

	FPropertyDisableScope::~FPropertyDisableScope()
	{
		if (Disable)
		{
			PropertyDisableCounter -= 1;
			if (bPropertyDisable)
			{
				bPropertyDisable = false;
				ImGui::EndDisabled();
			}
		}
	}

	FPropertyEnableScope::FPropertyEnableScope()
	{
		Enable = PropertyDisableCounter > 0;
		if (Enable)
		{
			PropertyDisableCounter -= 1;
			if (bPropertyDisable)
			{
				bPropertyDisable = false;
				ImGui::EndDisabled();
			}
		}
	}

	FPropertyEnableScope::~FPropertyEnableScope()
	{
		if (Enable)
		{
			if (bPropertyDisable == false)
			{
				bPropertyDisable = true;
				ImGui::BeginDisabled(true);
			}
			PropertyDisableCounter += 1;
		}
	}

	template<typename TNameCustomizer, typename TValueExtend>
	void AddUnrealContainerPropertyInner(const FProperty* Property, const FPtrArray& Containers, int32 Offset, const TNameCustomizer& NameCustomizer, const TValueExtend& ValueExtend)
	{
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(Property);
		if (!PropertyCustomization)
		{
			return;
		}

		bool IsShowChildren = false;
		const bool IsIdentical = IsAllPropertiesIdentical(Property, Containers, Offset);
		TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
		}

		if (InnerValue::IsVisible(Property, Containers, Offset, IsIdentical, PropertyCustomization, Customization) == false)
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		NameCustomizer(Property, Containers, Offset, IsIdentical, IsShowChildren);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ContainerValueRightWidth - (Customization ? Customization->ValueAdditiveRightWidth : 0.f));
		if (Customization)
		{
			Customization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}
		else
		{
			PropertyCustomization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}
		ValueExtend(Property, Containers, Offset, IsIdentical);

		if (IsShowChildren)
		{
			if (Customization)
			{
				Customization->CreateChildrenWidget(Property, Containers, Offset, IsIdentical);
			}
			else
			{
				PropertyCustomization->CreateChildrenWidget(Property, Containers, Offset, IsIdentical);
			}
			ImGui::TreePop();
		}

		ImGui::NextColumn();
	}

	void FBoolPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FBoolProperty* BoolProperty = CastFieldChecked<FBoolProperty>(Property);
		bool FirstValue = BoolProperty->GetPropertyValue(FirstValuePtr);
		if (ImGui::Checkbox(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), &FirstValue))
		{
			for (uint8* Container : Containers)
			{
				BoolProperty->SetPropertyValue(Container + Offset, FirstValue);
			}
			NotifyPostPropertyValueChanged(Property);
		}
	}

	void FNumericPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FNumericProperty* NumericProperty = CastFieldChecked<FNumericProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString PropertyLabelName = GetPropertyDefaultLabel(Property, IsIdentical);
		if (const UEnum* EnumDef = NumericProperty->GetIntPropertyEnum())
		{
			const int64 EnumValue = NumericProperty->GetSignedIntPropertyValue(FirstValuePtr);

			FString EnumName = EnumDef->GetNameByValue(EnumValue).ToString();
			EnumName.Split(TEXT("::"), nullptr, &EnumName);
			if (ImGui::BeginCombo(TCHAR_TO_UTF8(*PropertyLabelName), TCHAR_TO_UTF8(*EnumName), ImGuiComboFlags_PopupAlignLeft))
			{
				for (int32 Idx = 0; Idx < EnumDef->NumEnums() - 1; ++Idx)
				{
					const int64 CurrentEnumValue = EnumDef->GetValueByIndex(Idx);
					const bool IsSelected = CurrentEnumValue == EnumValue;
					EnumName = EnumDef->GetNameByIndex(Idx).ToString();
					EnumName.Split(TEXT("::"), nullptr, &EnumName);
					if (ImGui::Selectable(TCHAR_TO_UTF8(*EnumName), IsSelected))
					{
						for (uint8* Container : Containers)
						{
							NumericProperty->SetIntPropertyValue(Container + Offset, CurrentEnumValue);
						}
						NotifyPostPropertyValueChanged(Property);
					}
					if (IsSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		else if (NumericProperty->IsInteger())
		{
			int64 Number = NumericProperty->GetSignedIntPropertyValue(FirstValuePtr);
			ImGui::InputScalar(TCHAR_TO_UTF8(*PropertyLabelName), ImGuiDataType_S64, &Number);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				for (uint8* Container : Containers)
				{
					NumericProperty->SetIntPropertyValue(Container + Offset, Number);
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
		else if (NumericProperty->IsFloatingPoint())
		{
			double Number = NumericProperty->GetFloatingPointPropertyValue(FirstValuePtr);
			ImGui::InputScalar(TCHAR_TO_UTF8(*PropertyLabelName), ImGuiDataType_Double, &Number);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				for (uint8* Container : Containers)
				{
					NumericProperty->SetFloatingPointPropertyValue(Container + Offset, Number);
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	bool FObjectPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, IsIdentical))
		{
			return true;
		}
		const FObjectProperty* ObjectProperty = CastFieldChecked<FObjectProperty>(Property);
		if (ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference) == false)
		{
			return false;
		}
		FObjectArray Instances;
		for (const uint8* Container : Containers)
		{
			UObject* Object = ObjectProperty->GetObjectPropertyValue(Container + Offset);
			if (Object == nullptr)
			{
				return false;
			}
			Instances.Add(Object);
		}
		const UClass* TopClass = GetTopClass(Instances);
		static TSet<const FProperty*> Visited;
		for (const FProperty* ChildProperty = TopClass->PropertyLink; ChildProperty; ChildProperty = ChildProperty->PropertyLinkNext)
		{
			if (IsPropertyShow(ChildProperty) == false)
			{
				continue;
			}
			bool bIsAlreadyInSet;
			Visited.Add(ChildProperty, &bIsAlreadyInSet);
			if (bIsAlreadyInSet)
			{
				continue;
			}
			const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ChildProperty);
			const bool bVisible = PropertyCustomization && InnerValue::IsVisible(ChildProperty, reinterpret_cast<FPtrArray&>(Instances), ChildProperty->GetOffset_ForInternal(), IsIdentical, PropertyCustomization);
			Visited.Remove(ChildProperty);
			if (bVisible)
			{
				return true;
			}
		}
		return false;
	}

	bool FObjectPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FObjectProperty* ObjectProperty = CastFieldChecked<FObjectProperty>(Property);
		if (ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference) == false)
		{
			return false;
		}
		for (const uint8* Container : Containers)
		{
			if (ObjectProperty->GetObjectPropertyValue(Container + Offset) == nullptr)
			{
				return false;
			}
		}
		return true;
	}

	void FObjectPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FObjectProperty* ObjectProperty = CastFieldChecked<FObjectProperty>(Property);
		if (ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference))
		{
			const UObject* FirstValue = ObjectProperty->GetPropertyValue(Containers[0] + Offset);

			static FUTF8String FilterString;
			ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
			if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(FirstValue ? *FirstValue->GetName() : TEXT("Null"))))
			{
				UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
				const bool IsFiltered = ImGui::IsItemEdited();
				ImGui::Separator();
				if (ImGui::Selectable("Clear"))
				{
					for (uint8* Container : Containers)
					{
						ObjectProperty->SetPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}
				if (CachedInstancedClass != ObjectProperty->PropertyClass || IsFiltered)
				{
					CachedInstancedClass = ObjectProperty->PropertyClass;
					CachedClassList.Reset();
					TArray<UClass*> AllClasses;
					GetDerivedClasses(ObjectProperty->PropertyClass, AllClasses);
					AllClasses.Add(ObjectProperty->PropertyClass);
					const FString Filter = FilterString.ToString();
					for (UClass* Class : AllClasses)
					{
						if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
						{
							continue;
						}
						// Skip SKEL and REINST classes.
						if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
						{
							continue;
						}
						if (Filter.Len() > 0 && Class->GetName().Contains(Filter) == false)
						{
							continue;
						}
						if (Class->HasAllClassFlags(CLASS_EditInlineNew))
						{
							CachedClassList.Add(Class);
						}
					}
					CachedClassList.Sort([](const TWeakObjectPtr<UClass>& LHS, const TWeakObjectPtr<UClass>& RHS)
					{
						return LHS->GetFName().FastLess(RHS->GetFName());
					});
				}
				if (ImGui::BeginListBox("##Content"))
				{
					const UClass* FirstValueClass = FirstValue ? FirstValue->GetClass() : nullptr;
					ImGuiListClipper ListClipper{};
					ListClipper.Begin(CachedClassList.Num());
					while (ListClipper.Step())
					{
						for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
						{
							if (UClass* Class = CachedClassList[Idx].Get())
							{
								const bool IsSelected = Class == FirstValueClass;
								if (ImGui::Selectable(TCHAR_TO_UTF8(*Class->GetName()), IsSelected))
								{
									for (int32 ContainerIdx = 0; ContainerIdx < Containers.Num(); ++ContainerIdx)
									{
										uint8* Container = Containers[ContainerIdx];
										UObject* Outer = InnerValue::GOuters[ContainerIdx];
										check(Outer);
										ObjectProperty->SetPropertyValue(Container + Offset, NewObject<UObject>(Outer, Class));
									}
									NotifyPostPropertyValueChanged(Property);
								}
								if (IsSelected)
								{
									ImGui::SetItemDefaultFocus();
								}

								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class->GetFullName()));
									ImGui::EndTooltip();
								}
							}
						}
					}
					ListClipper.End();
					ImGui::EndListBox();
				}
				ImGui::EndCombo();
			}
			else if (FilterString.IsEmpty() == false)
			{
				FilterString.Reset();
				CachedInstancedClass = nullptr;
				CachedClassList.Empty();
			}
		}
		else
		{
			const uint8* FirstValuePtr = Containers[0] + Offset;
			FName ObjectName = NAME_None;
			const UObject* FirstValue = IsIdentical ? ObjectProperty->GetPropertyValue(FirstValuePtr) : nullptr;
			if (IsIdentical)
			{
				if (IsValid(FirstValue))
				{
					ObjectName = FirstValue->GetFName();
				}
				else
				{
					static FName NullName = TEXT("Null");
					ObjectName = NullName;
				}
			}
			else
			{
				static FName MultiValueName = TEXT("Multi Values");
				ObjectName = MultiValueName;
			}
			static FUTF8String FilterString;
			ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
			if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(*ObjectName.ToString())))
			{
				UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
				const bool IsFiltered = ImGui::IsItemEdited();
				ImGui::Separator();
				if (ImGui::Selectable("Clear"))
				{
					for (uint8* Container : Containers)
					{
						ObjectProperty->SetPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}
				UClass* ObjectClass = ObjectProperty->PropertyClass;
				if (CachedAssetClass != ObjectClass || IsFiltered)
				{
					CachedAssetClass = ObjectClass;
					CachedAssetList.Reset();
					static IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
					FARFilter ARFilter;
					ARFilter.bRecursivePaths = true;
					ARFilter.bRecursiveClasses = true;
					ARFilter.ClassPaths.Add(FTopLevelAssetPath{ ObjectClass });
					AssetRegistry.GetAssets(ARFilter, CachedAssetList);
					const FString Filter = FilterString.ToString();
					if (Filter.IsEmpty() == false)
					{
						CachedAssetList.RemoveAllSwap([&](const FAssetData& E) { return E.AssetName.ToString().Contains(Filter) == false; });
					}
					CachedAssetList.Sort([](const FAssetData& LHS, const FAssetData& RHS)
					{
						return LHS.AssetName.FastLess(RHS.AssetName);
					});
				}
				if (ImGui::BeginListBox("##Content"))
				{
					ImGuiListClipper ListClipper{};
					ListClipper.Begin(CachedAssetList.Num());
					while (ListClipper.Step())
					{
						for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
						{
							const FAssetData& Asset = CachedAssetList[Idx];
							const bool IsSelected = Asset.AssetName == ObjectName;
							if (ImGui::Selectable(TCHAR_TO_UTF8(*Asset.AssetName.ToString()), IsSelected))
							{
								for (uint8* Container : Containers)
								{
									ObjectProperty->SetPropertyValue(Container + Offset, Asset.GetAsset());
								}
								NotifyPostPropertyValueChanged(Property);
							}
							if (IsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*Asset.GetSoftObjectPath().ToString()));
								ImGui::EndTooltip();
							}
						}
					}
					ListClipper.End();
					ImGui::EndListBox();
				}
				ImGui::EndCombo();
			}
			else if (FilterString.IsEmpty() == false)
			{
				FilterString.Reset();
				CachedAssetClass = nullptr;
				CachedAssetList.Empty();
			}
			if (FirstValue)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue->GetPathName()));
					ImGui::EndTooltip();
				}
			}
		}
	}

	void FObjectPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FObjectProperty* ObjectProperty = CastFieldChecked<FObjectProperty>(Property);
		check(ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference));

		TGuardValue<int32> DepthGuard(InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1);

		FObjectArray Instances;
		for (const uint8* Container : Containers)
		{
			Instances.Add(ObjectProperty->GetObjectPropertyValue(Container + Offset));
		}
		const UClass* TopClass = GetTopClass(Instances);
		TGuardValue<FObjectArray> GOutersGuard{InnerValue::GOuters, Instances};

		const TSharedPtr<IUnrealDetailsCustomization> Customizer = UnrealPropertyCustomizeFactory::FindDetailsCustomizer(TopClass);
		if (Customizer)
		{
			Customizer->CreateClassDetails(TopClass, Instances, 0);
		}
		else
		{
			DrawDefaultClassDetails(TopClass, true, Instances, 0);
		}
	}

	void FSoftObjectPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FSoftObjectProperty* SoftObjectProperty = CastFieldChecked<FSoftObjectProperty>(Property);
		uint8* FirstValuePtr = Containers[0] + Offset;
		const FSoftObjectPtr& FirstValue = *SoftObjectProperty->GetPropertyValuePtr(FirstValuePtr);

		const FString ObjectName = [&]()-> FString
		{
			if (IsIdentical)
			{
				if (FirstValue.IsNull())
				{
					return TEXT("Null");
				}
				else if (UObject* Object = FirstValue.Get())
				{
					return Object->GetName();
				}
				else
				{
					return FirstValue.GetAssetName();
				}
			}
			return TEXT("Multi Values");
		}();
		UClass* ObjectClass = SoftObjectProperty->PropertyClass;

		if (ObjectClass->IsChildOf(AActor::StaticClass()))
		{
			static FUTF8String FilterString;
			ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
			if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(*ObjectName)))
			{
				UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
				const bool IsFiltered = ImGui::IsItemEdited();
				ImGui::Separator();
				if (ImGui::Selectable("Clear"))
				{
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}

				if (CachedActorClass != SoftObjectProperty->PropertyClass || IsFiltered)
				{
					CachedActorClass = SoftObjectProperty->PropertyClass;
					CachedActorList.Reset();
					const UObject* Outer = InnerValue::GOuters[0];
					const FString Filter = FilterString.ToString();
					for (TActorIterator<AActor> It(Outer ? Outer->GetWorld() : GWorld, SoftObjectProperty->PropertyClass); It; ++It)
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
						CachedActorList.Add(Actor);
					}
				}
				if (ImGui::BeginListBox("##Content"))
				{
					ImGuiListClipper ListClipper{};
					ListClipper.Begin(CachedActorList.Num());
					while (ListClipper.Step())
					{
						for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
						{
							if (AActor* Actor = CachedActorList[Idx].Get())
							{
								const bool IsSelected = FirstValue.Get() == Actor;
								if (ImGui::Selectable(TCHAR_TO_UTF8(*Actor->GetName()), IsSelected))
								{
									const FSoftObjectPtr NewValue{Actor};
									for (uint8* Container : Containers)
									{
										SoftObjectProperty->SetPropertyValue(Container + Offset, NewValue);
									}
									NotifyPostPropertyValueChanged(Property);
								}
								if (IsSelected)
								{
									ImGui::SetItemDefaultFocus();
								}

								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::TextUnformatted(TCHAR_TO_UTF8(*Actor->GetFullName()));
									ImGui::EndTooltip();
								}
							}
							else
							{
								ImGui::BeginDisabled(true);
								ImGui::Text("Deleted");
								ImGui::EndDisabled();
							}
						}
					}
					ListClipper.End();
					ImGui::EndListBox();
				}
				ImGui::EndCombo();
			}
			else
			{
				FilterString.Reset();
				CachedActorClass = nullptr;
				CachedActorList.Empty();
			}
		}
		else
		{
			static FUTF8String FilterString;
			ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
			if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(*ObjectName)))
			{
				UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
				const bool IsFiltered = ImGui::IsItemEdited();
				ImGui::Separator();
				if (ImGui::Selectable("Clear"))
				{
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}

				if (CachedAssetClass != ObjectClass || IsFiltered)
				{
					CachedAssetClass = ObjectClass;
					CachedAssetList.Reset();
					static IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
					FARFilter ARFilter;
					ARFilter.bRecursivePaths = true;
					ARFilter.bRecursiveClasses = true;
					ARFilter.ClassPaths.Add(FTopLevelAssetPath{ ObjectClass });
					AssetRegistry.GetAssets(ARFilter, CachedAssetList);
					const FString Filter = FilterString.ToString();
					if (Filter.IsEmpty() == false)
					{
						CachedAssetList.RemoveAllSwap([&](const FAssetData& E) { return E.AssetName.ToString().Contains(Filter) == false; });
					}
					CachedAssetList.Sort([](const FAssetData& LHS, const FAssetData& RHS)
					{
						return LHS.AssetName.FastLess(RHS.AssetName);
					});
				}
				if (ImGui::BeginListBox("##Content"))
				{
					ImGuiListClipper ListClipper{};
					ListClipper.Begin(CachedAssetList.Num());
					while (ListClipper.Step())
					{
						for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
						{
							const FAssetData& Asset = CachedAssetList[Idx];
							const bool IsSelected = FirstValue.GetUniqueID() == Asset.GetSoftObjectPath();
							if (ImGui::Selectable(TCHAR_TO_UTF8(*Asset.AssetName.ToString()), IsSelected))
							{
								const FSoftObjectPtr NewValue{Asset.ToSoftObjectPath()};
								for (uint8* Container : Containers)
								{
									SoftObjectProperty->SetPropertyValue(Container + Offset, NewValue);
								}
								NotifyPostPropertyValueChanged(Property);
							}
							if (IsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*Asset.GetSoftObjectPath().ToString()));
								ImGui::EndTooltip();
							}
						}
					}
					ListClipper.End();
					ImGui::EndListBox();
				}
				ImGui::EndCombo();
			}
			else if (FilterString.IsEmpty() == false)
			{
				FilterString.Reset();
				CachedAssetClass = nullptr;
				CachedAssetList.Empty();
			}
		}
		if (IsIdentical && FirstValue.IsNull() == false)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue.ToString()));
				ImGui::EndTooltip();
			}
		}
	}

	void FClassPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FClassProperty* ClassProperty = CastFieldChecked<FClassProperty>(Property);
		const UObject* FirstValue = ClassProperty->GetPropertyValue(Containers[0] + Offset);

		static FUTF8String FilterString;
		ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
		if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(FirstValue ? *FirstValue->GetName() : TEXT("Null"))))
		{
			UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
			const bool IsFiltered = ImGui::IsItemEdited();
			ImGui::Separator();
			if (ImGui::Selectable("Clear"))
			{
				for (uint8* Container : Containers)
				{
					ClassProperty->SetPropertyValue(Container + Offset, nullptr);
				}
				NotifyPostPropertyValueChanged(Property);
			}
			if (CachedClass != ClassProperty->MetaClass || IsFiltered)
			{
				CachedClass = ClassProperty->MetaClass;
				CachedClassList.Reset();
				// TODO：搜索未加载的蓝图类型
				TArray<UClass*> AllClasses;
				GetDerivedClasses(ClassProperty->MetaClass, AllClasses);
				AllClasses.Add(ClassProperty->MetaClass);
				const FString Filter = FilterString.ToString();
				for (UClass* Class : AllClasses)
				{
					if (Class->HasAnyClassFlags(CLASS_Deprecated))
					{
						continue;
					}
					// Skip SKEL and REINST classes.
					if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
					{
						continue;
					}
					if (Filter.Len() > 0 && Class->GetName().Contains(Filter) == false)
					{
						continue;
					}
					CachedClassList.Add(Class);
				}
				CachedClassList.Sort([](const TWeakObjectPtr<UClass>& LHS, const TWeakObjectPtr<UClass>& RHS)
				{
					return LHS->GetFName().FastLess(RHS->GetFName());
				});
			}
			if (ImGui::BeginListBox("##Content"))
			{
				ImGuiListClipper ListClipper{};
				ListClipper.Begin(CachedClassList.Num());
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						if (UClass* Class = CachedClassList[Idx].Get())
						{
							const bool IsSelected = Class == ClassProperty->MetaClass;
							if (ImGui::Selectable(TCHAR_TO_UTF8(*Class->GetName()), IsSelected))
							{
								for (uint8* Container : Containers)
								{
									ClassProperty->SetPropertyValue(Container + Offset, Class);
								}
								NotifyPostPropertyValueChanged(Property);
							}
							if (IsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class->GetFullName()));
								ImGui::EndTooltip();
							}
						}
					}
				}
				ListClipper.End();
				ImGui::EndListBox();
			}
			ImGui::EndCombo();
		}
		else if (FilterString.IsEmpty() == false)
		{
			FilterString.Reset();
			CachedClass = nullptr;
			CachedClassList.Empty();
		}
		if (FirstValue)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue->GetPathName()));
				ImGui::EndTooltip();
			}
		}
	}

	void FSoftClassPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FSoftClassProperty* SoftClassProperty = CastFieldChecked<FSoftClassProperty>(Property);
		const FSoftObjectPtr FirstValue = SoftClassProperty->GetPropertyValue(Containers[0] + Offset);
	
		static FUTF8String FilterString;
		ImGui::SetNextWindowSizeConstraints({ -1.f, -1.f }, { -1.f, -1.f });
		if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(FirstValue.IsNull() ? TEXT("Null") : *FirstValue.GetUniqueID().GetAssetName())))
		{
			UnrealImGui::InputTextWithHint("##Filter", "Filter", FilterString, ImGuiInputTextFlags_EnterReturnsTrue);
			const bool IsFiltered = ImGui::IsItemEdited();
			ImGui::Separator();
			if (ImGui::Selectable("Clear"))
			{
				for (uint8* Container : Containers)
				{
					SoftClassProperty->SetPropertyValue(Container + Offset, FSoftObjectPtr());
				}
				NotifyPostPropertyValueChanged(Property);
			}
			if (CachedClass != SoftClassProperty->MetaClass || IsFiltered)
			{
				CachedClass = SoftClassProperty->MetaClass;
				CachedClassList.Reset();
				// TODO：搜索未加载的蓝图类型
				TArray<UClass*> AllClasses;
				GetDerivedClasses(SoftClassProperty->MetaClass, AllClasses);
				AllClasses.Add(SoftClassProperty->MetaClass);
				const FString Filter = FilterString.ToString();
				for (UClass* Class : AllClasses)
				{
					if (Class->HasAnyClassFlags(CLASS_Deprecated))
					{
						continue;
					}
					// Skip SKEL and REINST classes.
					if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
					{
						continue;
					}
					if (Filter.Len() > 0 && Class->GetName().Contains(Filter) == false)
					{
						continue;
					}
					CachedClassList.Add(Class);
				}
				CachedClassList.Sort([](const TWeakObjectPtr<UClass>& LHS, const TWeakObjectPtr<UClass>& RHS)
				{
					return LHS->GetFName().FastLess(RHS->GetFName());
				});
			}
			if (ImGui::BeginListBox("##Content"))
			{
				ImGuiListClipper ListClipper{};
				ListClipper.Begin(CachedClassList.Num());
				while (ListClipper.Step())
				{
					for (int32 Idx = ListClipper.DisplayStart; Idx < ListClipper.DisplayEnd; ++Idx)
					{
						if (UClass* Class = CachedClassList[Idx].Get())
						{
							const bool IsSelected = Class == SoftClassProperty->MetaClass;
							if (ImGui::Selectable(TCHAR_TO_UTF8(*Class->GetName()), IsSelected))
							{
								const FSoftObjectPtr NewValue{Class};
								for (uint8* Container : Containers)
								{
									SoftClassProperty->SetPropertyValue(Container + Offset, NewValue);
								}
								NotifyPostPropertyValueChanged(Property);
							}
							if (IsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::TextUnformatted(TCHAR_TO_UTF8(*Class->GetFullName()));
								ImGui::EndTooltip();
							}
						}
					}
				}
				ListClipper.End();
				ImGui::EndListBox();
			}
			ImGui::EndCombo();
		}
		else if (FilterString.IsEmpty() == false)
		{
			FilterString.Reset();
			CachedClass = nullptr;
			CachedClassList.Empty();
		}
		if (IsIdentical && FirstValue.IsNull() == false)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue.ToString()));
				ImGui::EndTooltip();
			}
		}
	}

	void FStringPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FStrProperty* StringProperty = CastFieldChecked<FStrProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString FirstValue = StringProperty->GetPropertyValue(FirstValuePtr);
		const auto StringPoint = FTCHARToUTF8(*FirstValue);
		char Buff[512];
		FMemory::Memcpy(&Buff, StringPoint.Get(), StringPoint.Length() + 1);
		ImGui::InputText(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), Buff, sizeof(Buff));
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			const FString ChangedValue = UTF8_TO_TCHAR(Buff);
			for (uint8* Container : Containers)
			{
				StringProperty->SetPropertyValue(Container + Offset, ChangedValue);
			}
			NotifyPostPropertyValueChanged(Property);
		}
	}

	void FNamePropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FNameProperty* NameProperty = CastFieldChecked<FNameProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString PropertyLabelName = GetPropertyDefaultLabel(Property, IsIdentical);
		const FName FirstValue = NameProperty->GetPropertyValue(FirstValuePtr);
		const auto StringPoint = FTCHARToUTF8(*FirstValue.ToString());
		char Buff[512];
		FMemory::Memcpy(&Buff, StringPoint.Get(), StringPoint.Length() + 1);
		ImGui::InputText(TCHAR_TO_UTF8(*PropertyLabelName), Buff, sizeof(Buff));
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			const FName ChangedValue = UTF8_TO_TCHAR(Buff);
			for (uint8* Container : Containers)
			{
				NameProperty->SetPropertyValue(Container + Offset, ChangedValue);
			}
			NotifyPostPropertyValueChanged(Property);
		}
	}

	void FTextPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FTextProperty* TextProperty = CastFieldChecked<FTextProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FText FirstValue = TextProperty->GetPropertyValue(FirstValuePtr);
		const auto StringPoint = FTCHARToUTF8(*FirstValue.ToString());
		char Buff[512];
		FMemory::Memcpy(&Buff, StringPoint.Get(), StringPoint.Length() + 1);
		ImGui::InputText(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), Buff, sizeof(Buff));
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			const FText ChangedValue = FText::FromString(UTF8_TO_TCHAR(Buff));
			for (uint8* Container : Containers)
			{
				TextProperty->SetPropertyValue(Container + Offset, ChangedValue);
			}
			NotifyPostPropertyValueChanged(Property);
		}
	}

	void FEnumPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FEnumProperty* EnumProperty = CastFieldChecked<FEnumProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const UEnum* EnumDef = EnumProperty->GetEnum();
		const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
		const int64 EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(FirstValuePtr);

		FString EnumName = EnumDef->GetNameByValue(EnumValue).ToString();
		EnumName.Split(TEXT("::"), nullptr, &EnumName);
		if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyDefaultLabel(Property, IsIdentical)), TCHAR_TO_UTF8(*EnumName), ImGuiComboFlags_PopupAlignLeft))
		{
			for (int32 Idx = 0; Idx < EnumDef->NumEnums() - 1; ++Idx)
			{
				const int64 CurrentEnumValue = EnumDef->GetValueByIndex(Idx);
				const bool IsSelected = CurrentEnumValue == EnumValue;
				EnumName = EnumDef->GetNameByIndex(Idx).ToString();
				EnumName.Split(TEXT("::"), nullptr, &EnumName);
				if (ImGui::Selectable(TCHAR_TO_UTF8(*EnumName), IsSelected))
				{
					for (uint8* Container : Containers)
					{
						UnderlyingProperty->SetIntPropertyValue(Container + Offset, CurrentEnumValue);
					}
					NotifyPostPropertyValueChanged(Property);
				}
				if (IsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	bool FArrayPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, IsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, IsIdentical))
		{
			const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
			const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ArrayProperty->Inner);
			if (PropertyCustomization == nullptr)
			{
				return false;
			}
			TArray<FScriptArrayHelper> Helpers;
			for (const uint8* Container : Containers)
			{
				Helpers.Emplace(FScriptArrayHelper(ArrayProperty, Container + Offset));
			}
			FPtrArray ArrayRawPtr;
			ArrayRawPtr.SetNum(Containers.Num());
			for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
			{
				for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
				{
					ArrayRawPtr[Idx] = Helpers[Idx].GetRawPtr(ElemIdx);
				}
				if (InnerValue::IsVisible(ArrayProperty->Inner, ArrayRawPtr, 0, false, PropertyCustomization))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool FArrayPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptArrayHelper(ArrayProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (IsIdentical)
		{
			return true;
		}
		for (int32 Idx = 1; Idx < Containers.Num(); ++Idx)
		{
			if (FScriptArrayHelper(ArrayProperty, Containers[Idx] + Offset).Num() != FirstCount)
			{
				return false;
			}
		}
		return true;
	}

	void FArrayPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (IsIdentical)
		{
			const int32 ArrayCount = FScriptArrayHelper(ArrayProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", ArrayCount);
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("+")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptArrayHelper(ArrayProperty, Container + Offset).AddValue();
				}
				NotifyPostPropertyValueChanged(Property);
			}
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptArrayHelper(ArrayProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
		else
		{
			ImGui::Text("Different Elements *");
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptArrayHelper(ArrayProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FArrayPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ArrayProperty->Inner);
		if (!PropertyCustomization)
		{
			return;
		}

		TArray<FScriptArrayHelper> Helpers;
		for (const uint8* Container : Containers)
		{
			Helpers.Emplace(FScriptArrayHelper(ArrayProperty, Container + Offset));
		}
		FPtrArray ArrayRawPtr;
		ArrayRawPtr.SetNum(Containers.Num());
		for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
		{
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				ArrayRawPtr[Idx] = Helpers[Idx].GetRawPtr(ElemIdx);
			}
			TGuardValue<int32> GImGuiContainerIndexGuard(InnerValue::GImGuiContainerIndex, ElemIdx);

			AddUnrealContainerPropertyInner(ArrayProperty->Inner, ArrayRawPtr, 0,
				[ElemIdx, &PropertyCustomization](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren)
			{
				const FString ElementName = FString::Printf(TEXT("%d##%s%d"), ElemIdx, *Property->GetName(), InnerValue::GPropertyDepth);
				CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, IsIdentical), IsShowChildren, &ElementName);
			}, [ElemIdx, &Helpers](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical)
			{
				ImGui::SameLine();

				const FString ArrayPopupID =CreatePropertyLabel(Property, TEXT("unreal_array_popup"));
				if (ImGui::ArrowButton(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT(">"))), ImGuiDir_Down))
				{
					ImGui::OpenPopup(TCHAR_TO_UTF8(*ArrayPopupID));
				}

				ImGui::SameLine();
				if (ImGui::BeginPopup(TCHAR_TO_UTF8(*ArrayPopupID)))
				{
					if (ImGui::Selectable("Insert"))
					{
						for (FScriptArrayHelper& Helper : Helpers)
						{
							Helper.InsertValues(ElemIdx);
						}
						NotifyPostPropertyValueChanged(Property);
					}
					if (ImGui::Selectable("Delete"))
					{
						for (FScriptArrayHelper& Helper : Helpers)
						{
							Helper.RemoveValues(ElemIdx);
						}
						NotifyPostPropertyValueChanged(Property);
					}
					ImGui::EndPopup();
				}
			});
		}
	}

	bool FSetPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, IsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, IsIdentical))
		{
			const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
			const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(SetProperty->ElementProp);
			if (PropertyCustomization == nullptr)
			{
				return false;
			}
			TArray<FScriptSetHelper> Helpers;
			for (const uint8* Container : Containers)
			{
				Helpers.Emplace(FScriptSetHelper(SetProperty, Container + Offset));
			}
			FPtrArray SetRawPtr;
			SetRawPtr.SetNum(Containers.Num());
			for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
			{
				for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
				{
					SetRawPtr[Idx] = Helpers[Idx].GetElementPtr(ElemIdx);
				}
				if (InnerValue::IsVisible(SetProperty->ElementProp, SetRawPtr, 0, false, PropertyCustomization))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool FSetPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptSetHelper(SetProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (IsIdentical)
		{
			return true;
		}
		for (int32 Idx = 1; Idx < Containers.Num(); ++Idx)
		{
			if (FScriptSetHelper(SetProperty, Containers[Idx] + Offset).Num() != FirstCount)
			{
				return false;
			}
		}
		return true;
	}

	void FSetPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (IsIdentical)
		{
			const int32 SetCount = FScriptSetHelper(SetProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", SetCount);
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("+")))))
			{
				TArray<uint8> AddElem;
				AddElem.SetNumUninitialized(SetProperty->ElementProp->ElementSize);
				SetProperty->ElementProp->InitializeValue(AddElem.GetData());
				for (const uint8* Container : Containers)
				{
					FScriptSetHelper(SetProperty, Container + Offset).AddElement(AddElem.GetData());
				}
				NotifyPostPropertyValueChanged(Property);
			}
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptSetHelper(SetProperty, Container + Offset).EmptyElements();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
		else
		{
			ImGui::Text("Different Elements *");
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptSetHelper(SetProperty, Container + Offset).EmptyElements();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FSetPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(SetProperty->ElementProp);
		if (!PropertyCustomization)
		{
			return;
		}

		TArray<FScriptSetHelper> Helpers;
		for (const uint8* Container : Containers)
		{
			Helpers.Emplace(FScriptSetHelper(SetProperty, Container + Offset));
		}
		FPtrArray SetRawPtr;
		SetRawPtr.SetNum(Containers.Num());
		for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
		{
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				SetRawPtr[Idx] = Helpers[Idx].GetElementPtr(ElemIdx);
			}
			TGuardValue<int32> GImGuiContainerIndexGuard(InnerValue::GImGuiContainerIndex, ElemIdx);

			AddUnrealContainerPropertyInner(SetProperty->ElementProp, SetRawPtr, 0,
				[ElemIdx, &PropertyCustomization](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren)
			{
				const FString ElementName = FString::Printf(TEXT("%d##%s%d"), ElemIdx, *Property->GetName(), InnerValue::GPropertyDepth);
				CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, IsIdentical), IsShowChildren, &ElementName);
			}, [ElemIdx, &Helpers](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical)
			{
				ImGui::SameLine();

				const FString SetPopupID = CreatePropertyLabel(Property, TEXT("unreal_set_popup"));
				if (ImGui::ArrowButton(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT(">"))), ImGuiDir_Down))
				{
					ImGui::OpenPopup(TCHAR_TO_UTF8(*SetPopupID));
				}
				ImGui::SameLine();
				if (ImGui::BeginPopup(TCHAR_TO_UTF8(*SetPopupID)))
				{
					if (ImGui::MenuItem("Delete"))
					{
						for (FScriptSetHelper& Helper : Helpers)
						{
							Helper.RemoveAt(ElemIdx);
						}
						NotifyPostPropertyValueChanged(Property);
					}
					ImGui::EndPopup();
				}
			});
		}
	}

	bool FMapPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, IsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, IsIdentical))
		{
			const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
			const TSharedPtr<IUnrealPropertyCustomization> KeyPropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->KeyProp);
			const TSharedPtr<IUnrealPropertyCustomization> ValuePropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->ValueProp);
			if (KeyPropertyCustomization == nullptr || ValuePropertyCustomization == nullptr)
			{
				return false;
			}
			TArray<FScriptMapHelper> Helpers;
			for (const uint8* Container : Containers)
			{
				Helpers.Emplace(FScriptMapHelper(MapProperty, Container + Offset));
			}

			FPtrArray KeyRawPtr;
			KeyRawPtr.SetNum(Containers.Num());
			FPtrArray ValueRawPtr;
			ValueRawPtr.SetNum(Containers.Num());
			for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
			{
				for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
				{
					KeyRawPtr[Idx] = Helpers[Idx].GetKeyPtr(ElemIdx);
					ValueRawPtr[Idx] = Helpers[Idx].GetValuePtr(ElemIdx);
				}
				if (InnerValue::IsVisible(MapProperty->KeyProp, KeyRawPtr, 0, false, KeyPropertyCustomization) || InnerValue::IsVisible(MapProperty->ValueProp, ValueRawPtr, 0, false, ValuePropertyCustomization))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool FMapPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptMapHelper(MapProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (IsIdentical)
		{
			return true;
		}
		for (int32 Idx = 1; Idx < Containers.Num(); ++Idx)
		{
			if (FScriptMapHelper(MapProperty, Containers[Idx] + Offset).Num() != FirstCount)
			{
				return false;
			}
		}
		return true;
	}

	void FMapPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (IsIdentical)
		{
			const int32 MapCount = FScriptMapHelper(MapProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", MapCount);
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("+")))))
			{
				TArray<uint8> KeyElem;
				KeyElem.SetNumUninitialized(MapProperty->KeyProp->ElementSize);
				MapProperty->KeyProp->InitializeValue(KeyElem.GetData());
				TArray<uint8> ValueElem;
				ValueElem.SetNumUninitialized(MapProperty->ValueProp->ElementSize);
				MapProperty->ValueProp->InitializeValue(ValueElem.GetData());
				for (const uint8* Container : Containers)
				{
					if (FScriptMapHelper(MapProperty, Container + Offset).FindOrAdd(KeyElem.GetData()) == nullptr)
					{
						FScriptMapHelper(MapProperty, Container + Offset).AddPair(
							KeyElem.GetData(), ValueElem.GetData());
					}
				}
				NotifyPostPropertyValueChanged(Property);
			}
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptMapHelper(MapProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
		else
		{
			ImGui::Text("Different Elements *");
			ImGui::SameLine();
			if (ImGui::Button(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT("x")))))
			{
				for (const uint8* Container : Containers)
				{
					FScriptMapHelper(MapProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FMapPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const TSharedPtr<IUnrealPropertyCustomization> KeyPropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->KeyProp);
		const TSharedPtr<IUnrealPropertyCustomization> ValuePropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->ValueProp);
		if (!KeyPropertyCustomization || !ValuePropertyCustomization)
		{
			return;
		}

		TArray<FScriptMapHelper> Helpers;
		for (const uint8* Container : Containers)
		{
			Helpers.Emplace(FScriptMapHelper(MapProperty, Container + Offset));
		}

		FPtrArray KeyRawPtr;
		KeyRawPtr.SetNum(Containers.Num());
		FPtrArray ValueRawPtr;
		ValueRawPtr.SetNum(Containers.Num());
		for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
		{
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				KeyRawPtr[Idx] = Helpers[Idx].GetKeyPtr(ElemIdx);
				ValueRawPtr[Idx] = Helpers[Idx].GetValuePtr(ElemIdx);
			}
			TGuardValue<int32> GImGuiContainerIndexGuard(InnerValue::GImGuiContainerIndex, ElemIdx);

			TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
			}
			else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
			{
				Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			bool IsShowChildren = false;
			const bool KeyIsIdentical = IsAllPropertiesIdentical(MapProperty->KeyProp, KeyRawPtr, 0);
			const bool ValueIsIdentical = IsAllPropertiesIdentical(MapProperty->ValueProp, ValueRawPtr, 0);
			const bool KeyHasChildProperties = KeyPropertyCustomization->HasChildProperties(MapProperty->KeyProp, KeyRawPtr, 0, IsIdentical);
			const bool ValueHasChildProperties = ValuePropertyCustomization->HasChildProperties(MapProperty->ValueProp, ValueRawPtr, 0, IsIdentical);
			{
				const FString Name = FString::Printf(TEXT("%d##K%s%d"), ElemIdx, *Property->GetName(), InnerValue::GPropertyDepth);
				if (KeyHasChildProperties || ValueHasChildProperties)
				{
					IsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*Name));
				}
				else
				{
					constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
					ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Name), flags);
				}
				ImGui::SameLine();
				if (Customization)
				{
					Customization->CreateValueWidget(MapProperty->KeyProp, KeyRawPtr, 0, KeyIsIdentical);
				}
				else
				{
					KeyPropertyCustomization->CreateValueWidget(MapProperty->KeyProp, KeyRawPtr, 0, KeyIsIdentical);
				}
				if (KeyHasChildProperties && IsShowChildren)
				{
					if (Customization)
					{
						Customization->CreateChildrenWidget(MapProperty->KeyProp, KeyRawPtr, 0, KeyIsIdentical);
					}
					else
					{
						KeyPropertyCustomization->CreateChildrenWidget(MapProperty->KeyProp, KeyRawPtr, 0, KeyIsIdentical);
					}
					ImGui::TreePop();
				}
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(InnerValue::ContainerValueRightWidth - (Customization ? Customization->ValueAdditiveRightWidth : 0.f));
			{
				const FString Name = FString::Printf(TEXT("%d##V%s%d"), ElemIdx, *Property->GetName(), InnerValue::GPropertyDepth);
				if (Customization)
				{
					Customization->CreateValueWidget(MapProperty->ValueProp, ValueRawPtr, 0, ValueIsIdentical);
				}
				else
				{
					ValuePropertyCustomization->CreateValueWidget(MapProperty->ValueProp, ValueRawPtr, 0, ValueIsIdentical);
				}
				ImGui::SameLine();
				const FString MapPopupID = CreatePropertyLabel(Property, TEXT("unreal_map_popup"));
				if (ImGui::ArrowButton(TCHAR_TO_UTF8(*CreatePropertyLabel(Property, TEXT(">"))), ImGuiDir_Down))
				{
					ImGui::OpenPopup(TCHAR_TO_UTF8(*MapPopupID));
				}
				ImGui::SameLine();
				if (ImGui::BeginPopup(TCHAR_TO_UTF8(*MapPopupID)))
				{
					if (ImGui::MenuItem("Delete"))
					{
						for (FScriptMapHelper& Helper : Helpers)
						{
							Helper.RemoveAt(ElemIdx);
						}
						NotifyPostPropertyValueChanged(Property);
					}
					ImGui::EndPopup();
				}
				if (ValueHasChildProperties && IsShowChildren)
				{
					if (Customization)
					{
						Customization->CreateChildrenWidget(MapProperty->ValueProp, ValueRawPtr, 0, ValueIsIdentical);
					}
					else
					{
						ValuePropertyCustomization->CreateChildrenWidget(MapProperty->ValueProp, ValueRawPtr, 0, ValueIsIdentical);
					}
					ImGui::TreePop();
				}
			}

			ImGui::NextColumn();
		}
	}

	bool FStructPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, IsIdentical))
		{
			return true;
		}
		const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Property);
		for (const FProperty* ChildProperty = StructProperty->Struct->PropertyLink; ChildProperty; ChildProperty = ChildProperty->PropertyLinkNext)
		{
			if (IsPropertyShow(ChildProperty) == false)
			{
				continue;
			}
			const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ChildProperty);
			if (PropertyCustomization && InnerValue::IsVisible(ChildProperty, Containers, Offset + ChildProperty->GetOffset_ForInternal(), IsIdentical, PropertyCustomization))
			{
				return true;
			}
		}
		return false;
	}

	void FStructPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Property);
		int32 ElementCount = 0;
		for (const FProperty* ChildProperty = StructProperty->Struct->PropertyLink; ChildProperty; ChildProperty = ChildProperty->PropertyLinkNext)
		{
			if (IsPropertyShow(ChildProperty))
			{
				ElementCount += 1;
			}
		}
		ImGui::Text("%d Elements %s", ElementCount, IsIdentical ? "" : "*");
	}

	void FStructPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Property);
		TGuardValue<int32> DepthGuard(InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1);
		DrawDefaultStructDetails(StructProperty->Struct, Containers, Offset);
	}
}
