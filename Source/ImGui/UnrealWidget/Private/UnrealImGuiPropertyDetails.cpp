// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPropertyDetails.h"

#include "imgui.h"
#include "UnrealImGuiPropertyCustomization.h"
#include "UnrealImGuiStat.h"
#include "UnrealImGuiString.h"
#include "UnrealImGuiWrapper.h"

namespace UnrealImGui
{
	namespace GlobalValue
	{
		bool GEnableEditVisibleProperty = false;
		bool GDisplayAllProperties = false;

		FPostPropertyValueChanged GPostPropertyValueChanged;
	}
	
	namespace InnerValue
	{
		int32 GPropertyDepth = -1;
		int32 GImGuiContainerIndex = INDEX_NONE;
		FObjectArray GOuters;
		const FDetailsFilter* GFilter = nullptr;

		TMap<FFilterCacheKey, bool> FilterCacheMap;
		bool IsVisible(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, const TSharedPtr<IUnrealPropertyCustomization>& PropertyCustomization, const TSharedPtr<IUnrealStructCustomization>& Customization)
		{
			if (InnerValue::GFilter == nullptr || InnerValue::GFilter->IsFilterEnable() == false)
			{
				return true;
			}
			const FFilterCacheKey Key{ Containers[0] + Offset, Property };
			const uint32 KeyHash = GetTypeHash(Key);
			if (const bool* bVisible = FilterCacheMap.FindByHash(KeyHash, Key))
			{
				return *bVisible;
			}
			if (Customization)
			{
				const bool bVisible = Customization->IsVisible(*InnerValue::GFilter, Property, Containers, Offset, IsIdentical);
				FilterCacheMap.AddByHash(KeyHash, Key, bVisible);
				return bVisible;
			}
			else
			{
				const bool bVisible = PropertyCustomization->IsVisible(*InnerValue::GFilter, Property, Containers, Offset, IsIdentical);
				FilterCacheMap.AddByHash(KeyHash, Key, bVisible);
				return bVisible;
			}
		}

		bool IsVisible(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, const TSharedPtr<IUnrealPropertyCustomization>& PropertyCustomization)
		{
			if (InnerValue::GFilter == nullptr || InnerValue::GFilter->IsFilterEnable() == false)
			{
				return true;
			}
			const FFilterCacheKey Key{ Containers[0] + Offset };
			const uint32 KeyHash = GetTypeHash(Key);
			if (const bool* bFilter = FilterCacheMap.FindByHash(KeyHash, Key))
			{
				return *bFilter;
			}
			const bool bVisible = PropertyCustomization->IsVisible(*InnerValue::GFilter, Property, Containers, Offset, IsIdentical);
			FilterCacheMap.AddByHash(KeyHash, Key, bVisible);
			return bVisible;
		}
	}

	void AddUnrealPropertyInner(const FProperty* Property, const FPtrArray& Containers, int32 Offset)
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
		if (Customization)
		{
			Customization->CreateNameWidget(Property, Containers, Offset, IsIdentical, IsShowChildren);
		}
		else
		{
			CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, IsIdentical), IsShowChildren);
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ValueRightBaseWidth - (Customization ? Customization->ValueAdditiveRightWidth : PropertyCustomization->ValueAdditiveRightWidth));
		if (Customization)
		{
			Customization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}
		else
		{
			PropertyCustomization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}

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
}

namespace UnrealImGui
{
	TMap<FFieldClass*, TSharedRef<IUnrealPropertyCustomization>> UnrealPropertyCustomizeFactory::PropertyCustomizeMap;
	TMap<UStruct*, TSharedRef<IUnrealStructCustomization>> UnrealPropertyCustomizeFactory::StructCustomizeMap;
	TMap<UClass*, TSharedRef<IUnrealDetailsCustomization>> UnrealPropertyCustomizeFactory::ClassCustomizeMap;

	void FDetailsFilter::Draw(const char* Label)
	{
		FUTF8String UTF8String{ *StringFilter };
		if (InputTextWithHint(Label, "Filter", UTF8String))
		{
			StringFilter = *UTF8String;
		}
		ImGui::SameLine();
		ImGui::PushID(Label);
		if (ImGui::Button("X"))
		{
			StringFilter.Empty();
		}
		ImGui::PopID();
	}

	bool IUnrealPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		return Property->GetName().ToLower().Contains(Filter.StringFilter.ToLower());
	}

	bool IUnrealStructCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
	{
		return Property->GetName().ToLower().Contains(Filter.StringFilter.ToLower());
	}

	void IUnrealStructCustomization::CreateNameWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren) const
	{
		CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, false, IsShowChildren);
	}

	void IUnrealDetailsCustomization::CreateClassDetails(const UClass* Class, const FObjectArray& Containers, int32 Offset) const
	{
		DrawDefaultClassDetails(Class, false, Containers, Offset);
	}

	TSharedPtr<IUnrealPropertyCustomization> UnrealPropertyCustomizeFactory::FindPropertyCustomizer(const FProperty* Property)
	{
		for (const FFieldClass* TestClass = Property->GetClass(); TestClass; TestClass = TestClass->GetSuperClass())
		{
			if (TSharedRef<IUnrealPropertyCustomization>* Customization = PropertyCustomizeMap.Find(TestClass))
			{
				return *Customization;
			}
		}
		return nullptr;
	}

	TSharedPtr<IUnrealStructCustomization> UnrealPropertyCustomizeFactory::FindPropertyCustomizer(const UStruct* Struct)
	{
		if (TSharedRef<IUnrealStructCustomization>* Customization = StructCustomizeMap.Find(Struct))
		{
			return *Customization;
		}
		return nullptr;
	}

	TSharedPtr<IUnrealDetailsCustomization> UnrealPropertyCustomizeFactory::FindDetailsCustomizer(const UClass* Class)
	{
		for (const UClass* TestClass = Class; TestClass; TestClass = TestClass->GetSuperClass())
		{
			if (TSharedRef<IUnrealDetailsCustomization>* Customization = ClassCustomizeMap.Find(TestClass))
			{
				return *Customization;
			}
		}
		return nullptr;
	}

	template<typename ComponentT, int32 Length>
	struct FComponentPropertyCustomization : IUnrealStructCustomization
	{
		using ImGuiDataType = int32;
		ImGuiDataType DataType;
		const char* Format = nullptr;

		FComponentPropertyCustomization(ImGuiDataType data_type, const char* fmt = nullptr)
			: DataType(data_type), Format(fmt)
		{}

		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override
		{
			ComponentT Component[Length];
			FMemory::Memcpy(&Component, Containers[0] + Offset, sizeof(ComponentT) * Length);
			ImGui::InputScalarN(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), DataType, Component, Length, nullptr, nullptr, Format);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				for (uint8* Container : Containers)
				{
					FMemory::Memcpy(Container + Offset, &Component, sizeof(ComponentT) * Length);
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	};

	void UnrealPropertyCustomizeFactory::InitialDefaultCustomizer()
	{
		// 注册默认的属性自定义显示实现
		AddPropertyCustomizer(FBoolProperty::StaticClass(), MakeShared<FBoolPropertyCustomization>());
		AddPropertyCustomizer(FNumericProperty::StaticClass(), MakeShared<FNumericPropertyCustomization>());
		AddPropertyCustomizer(FObjectProperty::StaticClass(), MakeShared<FObjectPropertyCustomization>());
		AddPropertyCustomizer(FSoftObjectProperty::StaticClass(), MakeShared<FSoftObjectPropertyCustomization>());
		AddPropertyCustomizer(FClassProperty::StaticClass(), MakeShared<FClassPropertyCustomization>());
		AddPropertyCustomizer(FSoftClassProperty::StaticClass(), MakeShared<FSoftClassPropertyCustomization>());
		AddPropertyCustomizer(FStrProperty::StaticClass(), MakeShared<FStringPropertyCustomization>());
		AddPropertyCustomizer(FNameProperty::StaticClass(), MakeShared<FNamePropertyCustomization>());
		AddPropertyCustomizer(FTextProperty::StaticClass(), MakeShared<FTextPropertyCustomization>());
		AddPropertyCustomizer(FEnumProperty::StaticClass(), MakeShared<FEnumPropertyCustomization>());
		AddPropertyCustomizer(FArrayProperty::StaticClass(), MakeShared<FArrayPropertyCustomization>());
		AddPropertyCustomizer(FSetProperty::StaticClass(), MakeShared<FSetPropertyCustomization>());
		AddPropertyCustomizer(FMapProperty::StaticClass(), MakeShared<FMapPropertyCustomization>());
		AddPropertyCustomizer(FStructProperty::StaticClass(), MakeShared<FStructPropertyCustomization>());

		// 注册默认的结构体自定义显示实现
		struct StaticGetBaseStructure
		{
			static UScriptStruct* Get(FStringView Name)
			{
				static UPackage* CoreUObjectPkg = FindObjectChecked<UPackage>(nullptr, TEXT("/Script/CoreUObject"));
				UScriptStruct* Result = FindObjectChecked<UScriptStruct>(CoreUObjectPkg, Name.GetData());

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				if (!Result)
				{
					UE_LOG(LogClass, Fatal, TEXT("Failed to find native struct '%s.%s'"), *CoreUObjectPkg->GetName(), Name.GetData());
				}
#endif
				return Result;
			}
		};

		static_assert(std::is_same<FVector::FReal, double>::value);
		AddStructCustomizer(TBaseStructure<FVector>::Get(), MakeShared<FComponentPropertyCustomization<FVector::FReal, 3>>(ImGuiDataType_Double, "%.3f"));
		AddStructCustomizer(TBaseStructure<FRotator>::Get(), MakeShared<FComponentPropertyCustomization<FRotator::FReal, 3>>(ImGuiDataType_Double, "%.3f"));
		AddStructCustomizer(TBaseStructure<FVector2D>::Get(), MakeShared<FComponentPropertyCustomization<FVector2D::FReal, 2>>(ImGuiDataType_Double, "%.3f"));
		AddStructCustomizer(TBaseStructure<FVector4>::Get(), MakeShared<FComponentPropertyCustomization<FVector4::FReal, 4>>(ImGuiDataType_Double, "%.3f"));
		AddStructCustomizer(StaticGetBaseStructure::Get(TEXT("IntVector")), MakeShared<FComponentPropertyCustomization<int32, 3>>(ImGuiDataType_S32));

		struct FQuatCustomization : IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FRotator Rotation = reinterpret_cast<FQuat*>(Containers[0] + Offset)->Rotator();
				static_assert(std::is_same<FVector::FReal, double>::value);
				ImGui::InputScalarN(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), ImGuiDataType_Double, (double*)&Rotation, 3);
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					const FQuat Quat = Rotation.Quaternion();
					for (uint8* Container : Containers)
					{
						*reinterpret_cast<FQuat*>(Container + Offset) = Quat;
					}
					NotifyPostPropertyValueChanged(Property);
				}
			}
		};
		AddStructCustomizer(TBaseStructure<FQuat>::Get(), MakeShared<FQuatCustomization>());

		struct FLinearColorCustomization : IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FLinearColor Color = *reinterpret_cast<FLinearColor*>(Containers[0] + Offset);
				static_assert(std::is_same<decltype(Color.A), float>::value);
				if (ImGui::ColorEdit4(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), (float*)&Color))
				{
					for (uint8* Container : Containers)
					{
						*reinterpret_cast<FLinearColor*>(Container + Offset) = Color;
					}
					NotifyPostPropertyValueChanged(Property);
				}
			}
		};
		AddStructCustomizer(TBaseStructure<FLinearColor>::Get(), MakeShared<FLinearColorCustomization>());

		struct FColorCustomization : IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FLinearColor Color = *reinterpret_cast<FColor*>(Containers[0] + Offset);
				static_assert(std::is_same<decltype(Color.A), float>::value);
				if (ImGui::ColorEdit4(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), (float*)&Color))
				{
					for (uint8* Container : Containers)
					{
						*reinterpret_cast<FColor*>(Container + Offset) = Color.ToFColor(true);
					}
					UnrealImGui::NotifyPostPropertyValueChanged(Property);
				}
			}
		};
		AddStructCustomizer(TBaseStructure<FColor>::Get(), MakeShared<FColorCustomization>());
	}
}

void UnrealImGui::CreateUnrealPropertyNameWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool HasChildProperties, bool& IsShowChildren, const FString* NameOverride)
{
	FPropertyEnableScope PropertyEnableScope;

	const FString Name = FString::Printf(TEXT("%s##%d"), *(NameOverride ? *NameOverride : Property->GetName()), InnerValue::GPropertyDepth);
	if (HasChildProperties)
	{
		IsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*Name));
	}
	else
	{
		constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
		ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Name), flags);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
#if WITH_EDITOR
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*Property->GetToolTipText().ToString()));
#else
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*Property->GetName()));
#endif
		ImGui::EndTooltip();
	}
}

void UnrealImGui::AddUnrealProperty(const FProperty* Property, const FPtrArray& Containers, int32 Offset)
{
	FPropertyDisableScope ImGuiDisableScope{ Property };
	if (Property->ArrayDim == 1)
	{
		AddUnrealPropertyInner(Property, Containers, Offset + Property->GetOffset_ForInternal());
	}
	else
	{
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(Property);
		if (!PropertyCustomization)
		{
			return;
		}
		TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
		}

		const bool IsIdentical = [&]
		{
			if (Containers.Num() == 1)
			{
				return true;
			}
			for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ++ArrayIndex)
			{
				const int32 ElementOffset = Offset + Property->GetOffset_ForInternal() + Property->ElementSize * ArrayIndex;
				if (IsAllPropertiesIdentical(Property, Containers, ElementOffset) == false)
				{
					return false;
				}
			}
			return true;
		}();
		if (InnerValue::IsVisible(Property, Containers, Offset, IsIdentical, PropertyCustomization, Customization) == false)
		{
			return;
		}

		bool IsShowChildren = false;

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, true, IsShowChildren);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ValueRightBaseWidth);
		ImGui::TextUnformatted(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d Elements %s"), Property->ArrayDim, IsIdentical ? TEXT("") : TEXT("*"))));

		if (IsShowChildren)
		{
			for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ++ArrayIndex)
			{
				const int32 ElementOffset = Offset + Property->GetOffset_ForInternal() + Property->ElementSize * ArrayIndex;
				
				TGuardValue<int32> GImGuiContainerIndexGuard(InnerValue::GImGuiContainerIndex, ArrayIndex);
				
				bool IsElementShowChildren = false;
				const bool IsElementIdentical = IsAllPropertiesIdentical(Property, Containers, ElementOffset);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				{
					const FString ElementName = FString::Printf(TEXT("%d##%s%d"), ArrayIndex, *Property->GetName(), InnerValue::GPropertyDepth);
					CreateUnrealPropertyNameWidget(Property, Containers, ElementOffset, IsElementIdentical, PropertyCustomization->HasChildProperties(Property, Containers, ElementOffset, IsIdentical), IsElementShowChildren, &ElementName);
				}
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(InnerValue::ValueRightBaseWidth - (Customization ? Customization->ValueAdditiveRightWidth : 0.f));
				if (Customization)
				{
					Customization->CreateValueWidget(Property, Containers, ElementOffset, IsElementIdentical);
				}
				else
				{
					PropertyCustomization->CreateValueWidget(Property, Containers, ElementOffset, IsElementIdentical);
				}

				if (IsElementShowChildren)
				{
					if (Customization)
					{
						Customization->CreateChildrenWidget(Property, Containers, ElementOffset, IsElementIdentical);
					}
					else
					{
						PropertyCustomization->CreateChildrenWidget(Property, Containers, ElementOffset, IsElementIdentical);
					}
					ImGui::TreePop();
				}

				ImGui::NextColumn();
			}
			ImGui::TreePop();
		}
		ImGui::NextColumn();
	}
}

void UnrealImGui::DrawDefaultStructDetails(const UStruct* TopStruct, const FPtrArray& Instances, int32 Offset)
{
	check(Instances.Num() > 0);

	for (const FProperty* Property = TopStruct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (IsPropertyShow(Property) == false)
		{
			continue;
		}
		AddUnrealProperty(Property, Instances, Offset);
	}
}

void UnrealImGui::DrawDefaultClassDetails(const UClass* TopClass, bool CollapseCategories, const FObjectArray& Instances, int32 Offset)
{
	check(Instances.Num() > 0);

	CollapseCategories |= TopClass->HasAnyClassFlags(CLASS_CollapseCategories);
	for (const UClass* DetailClass = TopClass; DetailClass != UObject::StaticClass(); DetailClass = DetailClass->GetSuperClass())
	{
		if (DetailClass->PropertyLink && DetailClass->PropertyLink->GetOwnerClass() == DetailClass)
		{
			if (CollapseCategories)
			{
				for (const FProperty* Property = DetailClass->PropertyLink; Property && Property->GetOwnerClass() == DetailClass; Property = Property->PropertyLinkNext)
				{
					if (IsPropertyShow(Property) == false)
					{
						continue;
					}
					AddUnrealProperty(Property, reinterpret_cast<const FPtrArray&>(Instances), Offset);
				}
			}
			else
			{
				if (InnerValue::GFilter && InnerValue::GFilter->IsFilterEnable())
				{
					bool bCategoryVisible = false;
					for (const FProperty* Property = DetailClass->PropertyLink; Property && Property->GetOwnerClass() == DetailClass; Property = Property->PropertyLinkNext)
					{
						if (IsPropertyShow(Property) == false)
						{
							continue;
						}
						const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(Property);
						if (!PropertyCustomization)
						{
							continue;
						}
						TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
						if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
						{
							Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
						}
						else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
						{
							Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
						}
						if (InnerValue::IsVisible(Property, reinterpret_cast<const FPtrArray&>(Instances), Property->GetOffset_ForInternal(), false, PropertyCustomization, Customization))
						{
							bCategoryVisible = true;
							break;
						}
					}
					if (bCategoryVisible == false)
					{
						continue;
					}
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();

				const FString CategoryName = FString::Printf(TEXT("%s##%d%d"), *DetailClass->GetName(), InnerValue::GPropertyDepth, InnerValue::GImGuiContainerIndex);
				const bool IsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*CategoryName));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
#if WITH_EDITOR
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*DetailClass->GetToolTipText().ToString()));
#else
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*DetailClass->GetName()));
#endif
					ImGui::EndTooltip();
				}

				if (IsShowChildren)
				{
					for (const FProperty* Property = DetailClass->PropertyLink; Property && Property->GetOwnerClass() == DetailClass; Property = Property->PropertyLinkNext)
					{
						if (IsPropertyShow(Property) == false)
						{
							continue;
						}
						AddUnrealProperty(Property, reinterpret_cast<const FPtrArray&>(Instances), Offset);
					}
					ImGui::TreePop();
				}
				ImGui::NextColumn();
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("ImGui_DrawDetailTable"), STAT_ImGui_DrawDetailTable, STATGROUP_ImGui);

void UnrealImGui::DrawDetailTable(const char* str_id, const UStruct* TopStruct, const FPtrArray& Instances, const FDetailsFilter* Filter, const FPostPropertyValueChanged& PostPropertyValueChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ImGui_DrawDetailTable);
	if (ImGui::BeginTable(str_id, 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		TGuardValue GFilterGuard{ InnerValue::GFilter, Filter };
		TGuardValue DepthGuard{InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1 };
		TGuardValue GPostPropertyValueChangedGuard{ GlobalValue::GPostPropertyValueChanged, PostPropertyValueChanged };
		TGuardValue GFilterCacheMapGuard{ InnerValue::FilterCacheMap, InnerValue::FilterCacheMap };

		DrawDefaultStructDetails(TopStruct, Instances, 0);

		ImGui::EndTable();
	}
}

void UnrealImGui::DrawDetailTable(const char* str_id, const UClass* TopClass, const FObjectArray& Instances, const FDetailsFilter* Filter, const FPostPropertyValueChanged& PostPropertyValueChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ImGui_DrawDetailTable);
	if (ImGui::BeginTable(str_id, 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		TGuardValue GFilterGuard{ InnerValue::GFilter, Filter };
		TGuardValue GOutersGuard{ InnerValue::GOuters, reinterpret_cast<const FObjectArray&>(Instances) };
		TGuardValue DepthGuard{ InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1 };
		TGuardValue GPostPropertyValueChangedGuard{ GlobalValue::GPostPropertyValueChanged, PostPropertyValueChanged };
		TGuardValue GFilterCacheMapGuard{ InnerValue::FilterCacheMap, InnerValue::FilterCacheMap };

		const TSharedPtr<IUnrealDetailsCustomization> Customizer = UnrealPropertyCustomizeFactory::FindDetailsCustomizer(TopClass);
		if (Customizer)
		{
			Customizer->CreateClassDetails(TopClass, Instances, 0);
		}
		else
		{
			DrawDefaultClassDetails(TopClass, false, Instances, 0);
		}
		
		ImGui::EndTable();
	}
}

UClass* UnrealImGui::GetTopClass(const FObjectArray& Instances, const UClass* StopClass)
{
	check(Instances.Num() > 0);
	UClass* TopClass = Instances[0]->GetClass();
	for (const UObject* Entity : Instances)
	{
		if (Entity == nullptr)
		{
			return nullptr;
		}
		for (; TopClass; TopClass = TopClass->GetSuperClass())
		{
			if (Entity->GetClass()->IsChildOf(TopClass))
			{
				break;
			}
		}
		if (TopClass == StopClass)
		{
			break;
		}
	}
	return TopClass;
}

bool UnrealImGui::IsAllPropertiesIdentical(const FProperty* Property, const FPtrArray& Containers, int32 Offset)
{
	for (int32 Idx = 1; Idx < Containers.Num(); ++Idx)
	{
		const uint8* LHS = Containers[Idx - 1];
		const uint8* RHS = Containers[Idx];
		if (Property->Identical(LHS + Offset, RHS + Offset) == false)
		{
			return false;
		}
	}
	return true;
}

FString UnrealImGui::GetPropertyDefaultLabel(const FProperty* Property, bool IsIdentical)
{
	return CreatePropertyLabel(Property, IsIdentical ? TEXT("") : TEXT("*"));
}

FString UnrealImGui::CreatePropertyLabel(const FProperty* Property, const FString& Name)
{
	// 利用FName的Index机制创建短ID
	const uint32 ComparisonIndex = Property->GetFName().GetComparisonIndex().ToUnstableInt();
	return FString::Printf(TEXT("%s##%u%d%d"), *Name, ComparisonIndex, InnerValue::GPropertyDepth + 1, InnerValue::GImGuiContainerIndex + 1);
}

void UnrealImGui::NotifyPostPropertyValueChanged(const FProperty* Property)
{
	GlobalValue::GPostPropertyValueChanged.ExecuteIfBound(Property);
}
