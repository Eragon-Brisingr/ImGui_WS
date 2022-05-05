// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPropertyDetails.h"

#include "imgui.h"
#include "UnrealImGuiPropertyCustomization.h"
#include "UnrealImGuiStat.h"

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
	}

	void AddUnrealPropertyInner(const FProperty* Property, const FStructArray& Containers, int32 Offset)
	{
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindCustomizer(Property);
		if (!PropertyCustomization)
		{
			return;
		}

		bool IsShowChildren = false;
		const bool IsIdentical = IsAllPropertiesIdentical(Property, Containers, Offset);

		TSharedPtr<IUnrealStructCustomization> StructCustomization = nullptr;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			StructCustomization = UnrealPropertyCustomizeFactory::FindCustomizer(StructProperty->Struct);
		}
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		if (StructCustomization)
		{
			StructCustomization->CreateNameWidget(Property, Containers, Offset, IsIdentical, IsShowChildren);
		}
		else
		{
			CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, IsIdentical), IsShowChildren);
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ValueRightBaseWidth - (StructCustomization ? StructCustomization->ValueAdditiveRightWidth : PropertyCustomization->ValueAdditiveRightWidth));
		if (StructCustomization)
		{
			StructCustomization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}
		else
		{
			PropertyCustomization->CreateValueWidget(Property, Containers, Offset, IsIdentical);
		}

		if (IsShowChildren)
		{
			if (StructCustomization)
			{
				StructCustomization->CreateChildrenWidget(Property, Containers, Offset, IsIdentical);
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
	TMap<UClass*, TSharedRef<IUnrealClassCustomization>> UnrealPropertyCustomizeFactory::ClassCustomizeMap;
	
	void IUnrealStructCustomization::CreateNameWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren) const
	{
		UnrealImGui::CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, false, IsShowChildren);
	}

	void IUnrealClassCustomization::CreateClassDetails(const UClass* Class, const FObjectArray& Containers, int32 Offset) const
	{
		DrawDefaultClassDetails(Class, false, Containers, Offset);
	}

	TSharedPtr<IUnrealPropertyCustomization> UnrealPropertyCustomizeFactory::FindCustomizer(const FProperty* Property)
	{
		for (FFieldClass* TestClass = Property->GetClass(); TestClass; TestClass = TestClass->GetSuperClass())
		{
			if (TSharedRef<IUnrealPropertyCustomization>* Customization = PropertyCustomizeMap.Find(TestClass))
			{
				return *Customization;
			}
		}
		return nullptr;
	}

	TSharedPtr<IUnrealStructCustomization> UnrealPropertyCustomizeFactory::FindCustomizer(const UStruct* Struct)
	{
		if (TSharedRef<IUnrealStructCustomization>* Customization = StructCustomizeMap.Find(Struct))
		{
			return *Customization;
		}
		return nullptr;
	}

	TSharedPtr<IUnrealClassCustomization> UnrealPropertyCustomizeFactory::FindCustomizer(const UClass* Class)
	{
		for (const UClass* TestClass = Class; TestClass; TestClass = TestClass->GetSuperClass())
		{
			if (TSharedRef<IUnrealClassCustomization>* Customization = ClassCustomizeMap.Find(TestClass))
			{
				return *Customization;
			}
		}
		return nullptr;
	}

	template<typename ComponentT, int32 Length>
	struct FComponentPropertyCustomization : public IUnrealStructCustomization
	{
		ImGuiDataType data_type;
		const char* fmt = nullptr;

		FComponentPropertyCustomization(ImGuiDataType data_type, const char* fmt = nullptr)
			: data_type(data_type), fmt(fmt)
		{}

		void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override
		{
			FPropertyDisableScope ImGuiDisableScope{ Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance) };
			
			ComponentT Component[Length];
			FMemory::Memcpy(&Component, Containers[0] + Offset, sizeof(ComponentT) * Length);
			ImGui::InputScalarN(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), data_type, Component, Length, nullptr, nullptr, fmt);
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
			static UScriptStruct* Get(FName Name)
			{
				static UPackage* CoreUObjectPkg = FindObjectChecked<UPackage>(nullptr, TEXT("/Script/CoreUObject"));

				UScriptStruct* Result = (UScriptStruct*)StaticFindObjectFast(UScriptStruct::StaticClass(), CoreUObjectPkg, Name, false, false, RF_NoFlags, EInternalObjectFlags::None);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				if (!Result)
				{
					UE_LOG(LogClass, Fatal, TEXT("Failed to find native struct '%s.%s'"), *CoreUObjectPkg->GetName(), *Name.ToString());
				}
#endif
				return Result;
			}
		};

		AddStructCustomizer(TBaseStructure<FVector>::Get(), MakeShared<FComponentPropertyCustomization<float, 3>>(ImGuiDataType_Float, "%.3f"));
		AddStructCustomizer(TBaseStructure<FRotator>::Get(), MakeShared<FComponentPropertyCustomization<float, 3>>(ImGuiDataType_Float, "%.3f"));
		AddStructCustomizer(TBaseStructure<FVector2D>::Get(), MakeShared<FComponentPropertyCustomization<float, 2>>(ImGuiDataType_Float, "%.3f"));
		AddStructCustomizer(TBaseStructure<FVector4>::Get(), MakeShared<FComponentPropertyCustomization<float, 4>>(ImGuiDataType_Float, "%.3f"));
		AddStructCustomizer(StaticGetBaseStructure::Get(TEXT("IntVector")), MakeShared<FComponentPropertyCustomization<int32, 3>>(ImGuiDataType_S32));

		struct FQuatCustomization : public IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FPropertyDisableScope ImGuiDisableScope{ Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance) };
				
				FRotator Rotation = reinterpret_cast<FQuat*>(Containers[0] + Offset)->Rotator();
				ImGui::InputFloat3(TCHAR_TO_UTF8(*UnrealImGui::GetPropertyDefaultLabel(Property, IsIdentical)), (float*)&Rotation);
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

		struct FLinearColorCustomization : public IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FPropertyDisableScope ImGuiDisableScope{ Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance) };
				
				FLinearColor Color = *reinterpret_cast<FLinearColor*>(Containers[0] + Offset);
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

		struct FColorCustomization : public IUnrealStructCustomization
		{
			void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const override
			{
				FPropertyDisableScope ImGuiDisableScope{ Property->HasAnyPropertyFlags(CPF_EditConst | CPF_DisableEditOnInstance) };

				FLinearColor Color = *reinterpret_cast<FColor*>(Containers[0] + Offset);
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

void UnrealImGui::CreateUnrealPropertyNameWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical, bool HasChildProperties, bool& IsShowChildren, const FString* NameOverride)
{
	const FString Name = FString::Printf(TEXT("%s##%d"), *(NameOverride ? *NameOverride : Property->GetName()), InnerValue::GPropertyDepth);
	if (HasChildProperties)
	{
		IsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*Name));
	}
	else
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
		ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Name), flags);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
#if WITH_EDITOR
		ImGui::Text(TCHAR_TO_UTF8(*Property->GetToolTipText().ToString()));
#else
		ImGui::Text(TCHAR_TO_UTF8(*Property->GetName()));
#endif
		ImGui::EndTooltip();
	}
}

void UnrealImGui::AddUnrealProperty(const FProperty* Property, const FStructArray& Containers, int32 Offset)
{
	if (Property->ArrayDim == 1)
	{
		AddUnrealPropertyInner(Property, Containers, Offset + Property->GetOffset_ForInternal());
	}
	else
	{
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
		bool IsShowChildren = false;
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		CreateUnrealPropertyNameWidget(Property, Containers, Offset, IsIdentical, true, IsShowChildren);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ValueRightBaseWidth);
		ImGui::Text(TCHAR_TO_UTF8(*FString::Printf(TEXT("%d Elements %s"), Property->ArrayDim, IsIdentical ? TEXT("") : TEXT("*"))));

		if (IsShowChildren)
		{
			const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindCustomizer(Property);
			if (!PropertyCustomization)
			{
				return;
			}
			
			for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ++ArrayIndex)
			{
				const int32 ElementOffset = Offset + Property->GetOffset_ForInternal() + Property->ElementSize * ArrayIndex;
				
				TGuardValue<int32> GImGuiContainerIndexGuard(InnerValue::GImGuiContainerIndex, ArrayIndex);
				
				bool IsElementShowChildren = false;
				const bool IsElementIdentical = IsAllPropertiesIdentical(Property, Containers, ElementOffset);
				TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
				if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
				{
					Customization = UnrealPropertyCustomizeFactory::FindCustomizer(StructProperty->Struct);
				}

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

void UnrealImGui::DrawDefaultStructDetails(const UStruct* TopStruct, const FStructArray& Instances, int32 Offset)
{
	for (FProperty* Property = TopStruct->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (UnrealImGui::IsPropertyShow(Property) == false)
		{
			continue;
		}
		UnrealImGui::AddUnrealProperty(Property, reinterpret_cast<const FStructArray&>(Instances), Offset);
	}
}

void UnrealImGui::DrawDefaultClassDetails(const UClass* TopClass, bool CollapseCategories, const FObjectArray& Instances, int32 Offset)
{
	CollapseCategories |= TopClass->HasAnyClassFlags(CLASS_CollapseCategories);
	for (const UClass* DetailClass = TopClass; DetailClass != UObject::StaticClass(); DetailClass = DetailClass->GetSuperClass())
	{
		if (DetailClass->PropertyLink->GetOwnerClass() == DetailClass)
		{
			if (CollapseCategories)
			{
				for (FProperty* Property = DetailClass->PropertyLink; Property && Property->GetOwnerClass() == DetailClass; Property = Property->PropertyLinkNext)
				{
					if (UnrealImGui::IsPropertyShow(Property) == false)
					{
						continue;
					}
					UnrealImGui::AddUnrealProperty(Property, reinterpret_cast<const FStructArray&>(Instances), Offset);
				}
			}
			else
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();

				const FString CategoryName = FString::Printf(TEXT("%s##%d%d"), *DetailClass->GetName(), UnrealImGui::InnerValue::GPropertyDepth, UnrealImGui::InnerValue::GImGuiContainerIndex);
				const bool IsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*CategoryName));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
#if WITH_EDITOR
					ImGui::Text(TCHAR_TO_UTF8(*DetailClass->GetToolTipText().ToString()));
#else
					ImGui::Text(TCHAR_TO_UTF8(*DetailClass->GetName()));
#endif
					ImGui::EndTooltip();
				}

				if (IsShowChildren)
				{
					for (FProperty* Property = DetailClass->PropertyLink; Property && Property->GetOwnerClass() == DetailClass; Property = Property->PropertyLinkNext)
					{
						if (UnrealImGui::IsPropertyShow(Property) == false)
						{
							continue;
						}
						UnrealImGui::AddUnrealProperty(Property, reinterpret_cast<const FStructArray&>(Instances), Offset);
					}
					ImGui::TreePop();
				}
				ImGui::NextColumn();
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("ImGui_DrawDetailTable"), STAT_ImGui_DrawDetailTable, STATGROUP_ImGui);

void UnrealImGui::DrawDetailTable(const char* str_id, UStruct* TopStruct, const FStructArray& Instances, const FPostPropertyValueChanged& PostPropertyValueChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ImGui_DrawDetailTable);
	if (ImGui::BeginTable(str_id, 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		TGuardValue<int32> DepthGuard(InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1);
		TGuardValue<FPostPropertyValueChanged> GPostPropertyValueChangedGuard(GlobalValue::GPostPropertyValueChanged, PostPropertyValueChanged);

		DrawDefaultStructDetails(TopStruct, Instances, 0);

		ImGui::EndTable();
	}
}

void UnrealImGui::DrawDetailTable(const char* str_id, UClass* TopClass, const FObjectArray& Instances, const FPostPropertyValueChanged& PostPropertyValueChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ImGui_DrawDetailTable);
	if (ImGui::BeginTable(str_id, 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		TGuardValue<FObjectArray> GOutersGuard{ InnerValue::GOuters, reinterpret_cast<const FObjectArray&>(Instances) };
		TGuardValue<int32> DepthGuard(InnerValue::GPropertyDepth, InnerValue::GPropertyDepth + 1);
		TGuardValue<FPostPropertyValueChanged> GPostPropertyValueChangedGuard(GlobalValue::GPostPropertyValueChanged, PostPropertyValueChanged);

		const TSharedPtr<IUnrealClassCustomization> Customizer = UnrealPropertyCustomizeFactory::FindCustomizer(TopClass);
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
	for (UObject* Entity : Instances)
	{
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

bool UnrealImGui::IsAllPropertiesIdentical(const FProperty* Property, const FStructArray& Containers, int32 Offset)
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
