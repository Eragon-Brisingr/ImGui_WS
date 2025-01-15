// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiPropertyCustomization.h"

#include "EngineUtils.h"
#include "imgui.h"
#include "ImGuiEx.h"
#include "UnrealImGuiAssetPicker.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/TextProperty.h"

namespace UnrealImGui
{
	namespace InnerValue
	{
		extern FObjectArray GOuters;
		extern const FDetailsFilter* GFilter;
		extern IMGUI_WIDGETS_API FPostPropertyNameWidgetCreated GPostPropertyNameWidgetCreated;
	}
	
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

		bool bIsShowChildren = false;
		const bool bIsIdentical = IsAllPropertiesIdentical(Property, Containers, Offset);
		TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
		}

		if (InnerValue::IsVisible(Property, Containers, Offset, bIsIdentical, PropertyCustomization, Customization) == false)
		{
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		NameCustomizer(Property, Containers, Offset, bIsIdentical, bIsShowChildren);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(InnerValue::ContainerValueRightWidth - (Customization ? Customization->ValueAdditiveRightWidth : 0.f));
		if (Customization)
		{
			Customization->CreateValueWidget(Property, Containers, Offset, bIsIdentical);
		}
		else
		{
			PropertyCustomization->CreateValueWidget(Property, Containers, Offset, bIsIdentical);
		}
		ValueExtend(Property, Containers, Offset, bIsIdentical);

		if (bIsShowChildren)
		{
			if (Customization)
			{
				Customization->CreateChildrenWidget(Property, Containers, Offset, bIsIdentical);
			}
			else
			{
				PropertyCustomization->CreateChildrenWidget(Property, Containers, Offset, bIsIdentical);
			}
			ImGui::TreePop();
		}

		ImGui::NextColumn();
	}

	void FBoolPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FBoolProperty* BoolProperty = CastFieldChecked<FBoolProperty>(Property);
		bool FirstValue = BoolProperty->GetPropertyValue(FirstValuePtr);
		if (ImGui::Checkbox(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), &FirstValue))
		{
			for (uint8* Container : Containers)
			{
				BoolProperty->SetPropertyValue(Container + Offset, FirstValue);
			}
			NotifyPostPropertyValueChanged(Property);
		}
	}

	void FNumericPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FNumericProperty* NumericProperty = CastFieldChecked<FNumericProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString PropertyLabelName = GetPropertyValueLabel(Property, bIsIdentical);
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
						ImGui::CloseCurrentPopup();
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

	bool FObjectPropertyBaseCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, bIsIdentical))
		{
			return true;
		}
		const FObjectPropertyBase* ObjectProperty = CastFieldChecked<FObjectPropertyBase>(Property);
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
			const bool bVisible = PropertyCustomization && InnerValue::IsVisible(ChildProperty, reinterpret_cast<FPtrArray&>(Instances), ChildProperty->GetOffset_ForInternal(), bIsIdentical, PropertyCustomization);
			Visited.Remove(ChildProperty);
			if (bVisible)
			{
				return true;
			}
		}
		return false;
	}

	bool FObjectPropertyBaseCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FObjectPropertyBase* ObjectProperty = CastFieldChecked<FObjectPropertyBase>(Property);
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

	static FObjectPickerSettings ObjectPickerSettings;
	static FActorPickerSettings ActorPickerSettings;
	static FClassPickerSettings ClassPickerSettings;

	void FObjectPropertyBaseCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FObjectPropertyBase* ObjectProperty = CastFieldChecked<FObjectPropertyBase>(Property);
		
		const uint8* FirstValuePtr = Containers[0] + Offset;
		FName ObjectName = NAME_None;
		const UObject* FirstValue = bIsIdentical ? ObjectProperty->GetObjectPropertyValue(FirstValuePtr) : nullptr;
		if (bIsIdentical)
		{
			if (IsValid(FirstValue))
			{
				ObjectName = FirstValue->GetFName();
			}
		}
		else
		{
			static FName MultiValueName = TEXT("Multi Values");
			ObjectName = MultiValueName;
		}

		if (ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference))
		{
			static FClassPickerData ClassPickerData;
			ComboClassPicker(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)),
				TCHAR_TO_UTF8(*ObjectName.ToString()),
				ObjectProperty->PropertyClass,
				[&](const TSoftClassPtr<UObject>& Class)
				{
					return FirstValue == Class.Get();
				}, [&](const TSoftClassPtr<UObject>& ClassPtr)
				{
					UClass* Class = ClassPtr.LoadSynchronous();
					if (Class == nullptr)
					{
						return;
					}
					for (int32 ContainerIdx = 0; ContainerIdx < Containers.Num(); ++ContainerIdx)
					{
						uint8* Container = Containers[ContainerIdx];
						UObject* Outer = InnerValue::GOuters[ContainerIdx];
						check(Outer);
						ObjectProperty->SetObjectPropertyValue(Container + Offset, NewObject<UObject>(Outer, Class));
					}
					NotifyPostPropertyValueChanged(Property);
				}, [&]
				{
					for (uint8* Container : Containers)
					{
						ObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}, CLASS_None, &ClassPickerSettings, &ClassPickerData);
		}
		else
		{
			UClass* ObjectClass = ObjectProperty->PropertyClass;
			if (ObjectClass->IsChildOf(AActor::StaticClass()))
			{
				static FActorPickerData ActorPickerData;
				ImGui::SetNextWindowSize({ -1, 400 });
				UWorld* World = InnerValue::GOuters.Num() > 0 && InnerValue::GOuters[0] ? InnerValue::GOuters[0]->GetWorld() : GWorld;
				ComboActorPicker(World, TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), TCHAR_TO_UTF8(*ObjectName.ToString()), ObjectClass,
					[&](const AActor* Actor)
					{
						return FirstValue == Actor;
					}, [&](AActor* Actor)
					{
						for (uint8* Container : Containers)
						{
							ObjectProperty->SetObjectPropertyValue(Container + Offset, Actor);
						}
						NotifyPostPropertyValueChanged(Property);
					}, [&]
					{
						for (uint8* Container : Containers)
						{
							ObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
						}
						NotifyPostPropertyValueChanged(Property);
					}, &ActorPickerSettings, &ActorPickerData);
			}
			else
			{
				static FObjectPickerData ObjectPickerData;
				ImGui::SetNextWindowSize({ -1, 400 });
				ComboObjectPicker(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), TCHAR_TO_UTF8(*ObjectName.ToString()), ObjectClass,
					[Path = FSoftObjectPath{ FirstValue }](const FAssetData& Asset)
					{
						return Path == Asset.GetSoftObjectPath();
					}, [&](const FAssetData& Asset)
					{
						for (uint8* Container : Containers)
						{
							ObjectProperty->SetObjectPropertyValue(Container + Offset, Asset.GetAsset());
						}
						NotifyPostPropertyValueChanged(Property);
					}, [&]
					{
						for (uint8* Container : Containers)
						{
							ObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
						}
						NotifyPostPropertyValueChanged(Property);
					}, &ObjectPickerSettings, &ObjectPickerData);
			}
		}
		if (FirstValue)
		{
			if (auto Tooltip = ImGui::FItemTooltip())
			{
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue->GetPathName()));
			}
		}
	}

	void FObjectPropertyBaseCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FObjectPropertyBase* ObjectProperty = CastFieldChecked<FObjectPropertyBase>(Property);
		check(ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference));

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
			DrawClassDefaultDetails(TopClass, true, Instances, 0);
		}
	}

	void FSoftObjectPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FSoftObjectProperty* SoftObjectProperty = CastFieldChecked<FSoftObjectProperty>(Property);
		uint8* FirstValuePtr = Containers[0] + Offset;
		const FSoftObjectPtr& FirstValue = *SoftObjectProperty->GetPropertyValuePtr(FirstValuePtr);

		const FString ObjectName = [&]()-> FString
		{
			if (bIsIdentical)
			{
				if (FirstValue.IsNull())
				{
					return TEXT("None");
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
			static FActorPickerData ActorPickerData;
			ImGui::SetNextWindowSize({ -1, 400 });
			UWorld* World = InnerValue::GOuters.Num() > 0 && InnerValue::GOuters[0] ? InnerValue::GOuters[0]->GetWorld() : GWorld;
			ComboActorPicker(World, TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), TCHAR_TO_UTF8(*ObjectName), ObjectClass,
				[&](const AActor* Actor)
				{
					return FirstValue.Get() == Actor;
				}, [&](AActor* Actor)
				{
					const FSoftObjectPtr NewValue{Actor};
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetPropertyValue(Container + Offset, NewValue);
					}
					NotifyPostPropertyValueChanged(Property);
				}, [&]
				{
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}, &ActorPickerSettings, &ActorPickerData);
		}
		else
		{
			static FObjectPickerData ObjectPickerData;
			ImGui::SetNextWindowSize({ -1, 400 });
			ComboObjectPicker(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), TCHAR_TO_UTF8(*ObjectName), ObjectClass,
				[&](const FAssetData& Asset)
				{
					return FirstValue.GetUniqueID() == Asset.GetSoftObjectPath();
				}, [&](const FAssetData& Asset)
				{
					const FSoftObjectPtr NewValue{Asset.ToSoftObjectPath()};
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetPropertyValue(Container + Offset, NewValue);
					}
					NotifyPostPropertyValueChanged(Property);
				}, [&]
				{
					for (uint8* Container : Containers)
					{
						SoftObjectProperty->SetObjectPropertyValue(Container + Offset, nullptr);
					}
					NotifyPostPropertyValueChanged(Property);
				}, &ObjectPickerSettings, &ObjectPickerData);
		}
		if (bIsIdentical)
		{
			if (auto ToolTip = ImGui::FItemTooltip())
			{
				ImGui::TextUnformatted(FirstValue.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*FirstValue.ToString()));
			}
		}
	}

	void FClassPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FClassProperty* ClassProperty = CastFieldChecked<FClassProperty>(Property);
		const UObject* FirstValue = ClassProperty->GetPropertyValue(Containers[0] + Offset);

		static FClassPickerData ClassPickerData;
		ImGui::SetNextWindowSize({ -1, 400 });
		ComboClassPicker(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)),
			bIsIdentical ? (FirstValue ? TCHAR_TO_UTF8(*FirstValue->GetName()) : "None") : "Multi Values",
			ClassProperty->MetaClass,
			[&](const TSoftClassPtr<UObject>& Class)
			{
				return FirstValue == Class.Get();
			}, [&](const TSoftClassPtr<UObject>& Class)
			{
				for (uint8* Container : Containers)
				{
					ClassProperty->SetPropertyValue(Container + Offset, Class.LoadSynchronous());
				}
				NotifyPostPropertyValueChanged(Property);
			}, [&]
			{
				for (uint8* Container : Containers)
				{
					ClassProperty->SetPropertyValue(Container + Offset, nullptr);
				}
				NotifyPostPropertyValueChanged(Property);
			}, CLASS_None, &ClassPickerSettings, &ClassPickerData);
		if (bIsIdentical && FirstValue)
		{
			if (auto ToolTip = ImGui::FItemTooltip())
			{
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue->GetPathName()));
			}
		}
	}

	void FSoftClassPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FSoftClassProperty* SoftClassProperty = CastFieldChecked<FSoftClassProperty>(Property);
		const FSoftObjectPtr FirstValue = SoftClassProperty->GetPropertyValue(Containers[0] + Offset);

		static FClassPickerData ClassPickerData;
		ImGui::SetNextWindowSize({ -1, 400 });
		ComboClassPicker(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)),
			bIsIdentical ? (FirstValue.ToSoftObjectPath().IsNull() ? "None" : TCHAR_TO_UTF8(*FirstValue.GetUniqueID().GetAssetName())) : "Multi Values",
			SoftClassProperty->MetaClass,
			[FirstValue = TSoftClassPtr<UObject>{ FirstValue.ToSoftObjectPath() }](const TSoftClassPtr<UObject>& Class)
			{
				return FirstValue == Class;
			}, [&](const TSoftClassPtr<UObject>& Class)
			{
				const FSoftObjectPtr NewValue{ Class.ToSoftObjectPath() };
				for (uint8* Container : Containers)
				{
					SoftClassProperty->SetPropertyValue(Container + Offset, NewValue);
				}
				NotifyPostPropertyValueChanged(Property);
			}, [&]
			{
				for (uint8* Container : Containers)
				{
					SoftClassProperty->SetPropertyValue(Container + Offset, FSoftObjectPtr());
				}
				NotifyPostPropertyValueChanged(Property);
			}, CLASS_None, &ClassPickerSettings, &ClassPickerData);
		if (bIsIdentical && !FirstValue.ToSoftObjectPath().IsNull())
		{
			if (auto ToolTip = ImGui::FItemTooltip())
			{
				ImGui::TextUnformatted(TCHAR_TO_UTF8(*FirstValue.ToString()));
			}
		}
	}

	void FStringPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FStrProperty* StringProperty = CastFieldChecked<FStrProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString FirstValue = StringProperty->GetPropertyValue(FirstValuePtr);
		const auto StringPoint = FTCHARToUTF8(*FirstValue);
		char Buff[512];
		FMemory::Memcpy(&Buff, StringPoint.Get(), StringPoint.Length() + 1);
		ImGui::InputText(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), Buff, sizeof(Buff));
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

	void FNamePropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FNameProperty* NameProperty = CastFieldChecked<FNameProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FString PropertyLabelName = GetPropertyValueLabel(Property, bIsIdentical);
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

	void FTextPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FTextProperty* TextProperty = CastFieldChecked<FTextProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const FText FirstValue = TextProperty->GetPropertyValue(FirstValuePtr);
		const auto StringPoint = FTCHARToUTF8(*FirstValue.ToString());
		char Buff[512];
		FMemory::Memcpy(&Buff, StringPoint.Get(), StringPoint.Length() + 1);
		ImGui::InputText(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), Buff, sizeof(Buff));
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

	void FEnumPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FEnumProperty* EnumProperty = CastFieldChecked<FEnumProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const UEnum* EnumType = EnumProperty->GetEnum();
		const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
		const int64 EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(FirstValuePtr);

		const int32 ShortNameStartIdx = EnumType->GetName().Len() + 2;
		const auto PreviewEnumName = EnumType->GetNameByValue(EnumValue);
		if (ImGui::BeginCombo(TCHAR_TO_UTF8(*GetPropertyValueLabel(Property, bIsIdentical)), PreviewEnumName != NAME_None ? TCHAR_TO_UTF8(*PreviewEnumName.ToString().Mid(ShortNameStartIdx)) : "None", ImGuiComboFlags_PopupAlignLeft))
		{
			for (int32 Idx = 0; Idx < EnumType->NumEnums() - 1; ++Idx)
			{
				const int64 CurrentEnumValue = EnumType->GetValueByIndex(Idx);
				const bool IsSelected = CurrentEnumValue == EnumValue;
				if (ImGui::Selectable(TCHAR_TO_UTF8(*EnumType->GetNameByValue(Idx).ToString().Mid(ShortNameStartIdx)), IsSelected))
				{
					for (uint8* Container : Containers)
					{
						UnderlyingProperty->SetIntPropertyValue(Container + Offset, CurrentEnumValue);
					}
					NotifyPostPropertyValueChanged(Property);
					ImGui::CloseCurrentPopup();
				}
				if (IsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	bool FArrayPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, bIsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, bIsIdentical))
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

	bool FArrayPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptArrayHelper(ArrayProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (bIsIdentical)
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

	void FArrayPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (bIsIdentical)
		{
			const int32 ArrayCount = FScriptArrayHelper(ArrayProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", ArrayCount);
			ImGui::SameLine();
			if (ImGui::Button("+"))
			{
				for (const uint8* Container : Containers)
				{
					FScriptArrayHelper(ArrayProperty, Container + Offset).AddValue();
				}
				NotifyPostPropertyValueChanged(Property);
			}
			ImGui::SameLine();
			if (ImGui::Button("x"))
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
			if (ImGui::Button("x"))
			{
				for (const uint8* Container : Containers)
				{
					FScriptArrayHelper(ArrayProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FArrayPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
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
			ImGui::FIdScope IdScope{ ElemIdx };
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				ArrayRawPtr[Idx] = Helpers[Idx].GetRawPtr(ElemIdx);
			}

			AddUnrealContainerPropertyInner(ArrayProperty->Inner, ArrayRawPtr, 0,
				[ElemIdx, &PropertyCustomization](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical, bool& bIsShowChildren)
			{
				const FString ElementName = FString::FromInt(ElemIdx);
				CreateUnrealPropertyNameWidget(Property, Containers, Offset, bIsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, bIsIdentical), bIsShowChildren, &ElementName);
			}, [ElemIdx, &Helpers](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical)
			{
				ImGui::SameLine();

				const auto ArrayPopupName = "UnrealArrayPopup";
				if (ImGui::ArrowButton(">", ImGuiDir_Down))
				{
					ImGui::OpenPopup(ArrayPopupName);
				}
				if (ImGui::BeginPopup(ArrayPopupName))
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

	bool FSetPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, bIsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, bIsIdentical))
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

	bool FSetPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptSetHelper(SetProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (bIsIdentical)
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

	void FSetPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (bIsIdentical)
		{
			const int32 SetCount = FScriptSetHelper(SetProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", SetCount);
			ImGui::SameLine();
			if (ImGui::Button("+"))
			{
				TArray<uint8> AddElem;
				AddElem.SetNumUninitialized(SetProperty->ElementProp->GetElementSize());
				SetProperty->ElementProp->InitializeValue(AddElem.GetData());
				for (const uint8* Container : Containers)
				{
					FScriptSetHelper(SetProperty, Container + Offset).AddElement(AddElem.GetData());
				}
				NotifyPostPropertyValueChanged(Property);
			}
			ImGui::SameLine();
			if (ImGui::Button("x"))
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
			if (ImGui::Button("x"))
			{
				for (const uint8* Container : Containers)
				{
					FScriptSetHelper(SetProperty, Container + Offset).EmptyElements();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FSetPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FSetProperty* SetProperty = CastFieldChecked<FSetProperty>(Property);
		const TSharedPtr<IUnrealPropertyCustomization> PropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(SetProperty->ElementProp);
		if (!PropertyCustomization)
		{
			return;
		}

		TArray<FScriptSetHelper, TInlineAllocator<ArrayInlineAllocateSize>> Helpers;
		Helpers.Reserve(Containers.Num());
		TArray<FScriptSetHelper::FIterator, TInlineAllocator<ArrayInlineAllocateSize>> Iterators;
		Iterators.Reserve(Containers.Num());
		for (const uint8* Container : Containers)
		{
			Helpers.Emplace(FScriptSetHelper(SetProperty, Container + Offset));
			Iterators.Emplace(Helpers.Last().CreateIterator());
		}
		FPtrArray SetRawPtr;
		SetRawPtr.SetNum(Containers.Num());

		for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
		{
			ImGui::FIdScope IdScope{ ElemIdx };
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				SetRawPtr[Idx] = Helpers[Idx].GetElementPtr(Iterators[Idx]);
			}

			AddUnrealContainerPropertyInner(SetProperty->ElementProp, SetRawPtr, 0,
				[ElemIdx, &PropertyCustomization](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical, bool& bIsShowChildren)
			{
				const FString ElementName = FString::FromInt(ElemIdx);
				CreateUnrealPropertyNameWidget(Property, Containers, Offset, bIsIdentical, PropertyCustomization->HasChildProperties(Property, Containers, Offset, bIsIdentical), bIsShowChildren, &ElementName);
			}, [ElemIdx, &Helpers, &Iterators](const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical)
			{
				ImGui::SameLine();

				const auto SetPopupName = "UnrealSetPopup";
				if (ImGui::ArrowButton(">", ImGuiDir_Down))
				{
					ImGui::OpenPopup(SetPopupName);
				}
				if (ImGui::BeginPopup(SetPopupName))
				{
					if (ImGui::MenuItem("Delete"))
					{
						for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
						{
							Helpers[Idx].RemoveAt(Iterators[Idx].GetInternalIndex());
						}
						NotifyPostPropertyValueChanged(Property);
					}
					ImGui::EndPopup();
				}
			});

			for (auto& It : Iterators)
			{
				++It;
			}
		}
	}

	bool FMapPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, bIsIdentical))
		{
			return true;
		}
		if (HasChildPropertiesOverride(Property, Containers, Offset, bIsIdentical))
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

	bool FMapPropertyCustomization::HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		const int32 FirstCount = FScriptMapHelper(MapProperty, FirstValuePtr).Num();
		if (FirstCount == 0)
		{
			return false;
		}
		if (bIsIdentical)
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

	void FMapPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const uint8* FirstValuePtr = Containers[0] + Offset;
		if (bIsIdentical)
		{
			const int32 MapCount = FScriptMapHelper(MapProperty, FirstValuePtr).Num();
			ImGui::Text("%d Elements", MapCount);
			ImGui::SameLine();
			if (ImGui::Button("+"))
			{
				TArray<uint8> KeyElem;
				KeyElem.SetNumUninitialized(MapProperty->KeyProp->GetElementSize());
				MapProperty->KeyProp->InitializeValue(KeyElem.GetData());
				TArray<uint8> ValueElem;
				ValueElem.SetNumUninitialized(MapProperty->ValueProp->GetElementSize());
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
			if (ImGui::Button("x"))
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
			if (ImGui::Button("x"))
			{
				for (const uint8* Container : Containers)
				{
					FScriptMapHelper(MapProperty, Container + Offset).EmptyValues();
				}
				NotifyPostPropertyValueChanged(Property);
			}
		}
	}

	void FMapPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
		const TSharedPtr<IUnrealPropertyCustomization> KeyPropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->KeyProp);
		const TSharedPtr<IUnrealPropertyCustomization> ValuePropertyCustomization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(MapProperty->ValueProp);
		if (!KeyPropertyCustomization || !ValuePropertyCustomization)
		{
			return;
		}

		TArray<FScriptMapHelper, TInlineAllocator<ArrayInlineAllocateSize>> Helpers;
		Helpers.Reserve(Containers.Num());
		TArray<FScriptMapHelper::FIterator, TInlineAllocator<ArrayInlineAllocateSize>> Iterators;
		Iterators.Reserve(Containers.Num());
		for (const uint8* Container : Containers)
		{
			Helpers.Emplace(FScriptMapHelper(MapProperty, Container + Offset));
			Iterators.Emplace(Helpers.Last().CreateIterator());
		}

		FPtrArray KeyRawPtr;
		KeyRawPtr.SetNum(Containers.Num());
		FPtrArray ValueRawPtr;
		ValueRawPtr.SetNum(Containers.Num());
		
		TSharedPtr<IUnrealStructCustomization> Customization = nullptr;
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(StructProperty->Struct);
		}
		else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			Customization = UnrealPropertyCustomizeFactory::FindPropertyCustomizer(ObjectProperty->PropertyClass);
		}

		for (int32 ElemIdx = 0; ElemIdx < Helpers[0].Num(); ++ElemIdx)
		{
			ImGui::FIdScope IdScope{ ElemIdx };
			for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
			{
				const auto& It = Iterators[Idx];
				KeyRawPtr[Idx] = Helpers[Idx].GetKeyPtr(It);
				ValueRawPtr[Idx] = Helpers[Idx].GetValuePtr(It);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			bool bIsShowChildren = false;
			const bool bKeyIsIdentical = IsAllPropertiesIdentical(MapProperty->KeyProp, KeyRawPtr, 0);
			const bool bValueIsIdentical = IsAllPropertiesIdentical(MapProperty->ValueProp, ValueRawPtr, 0);
			const bool bKeyHasChildProperties = KeyPropertyCustomization->HasChildProperties(MapProperty->KeyProp, KeyRawPtr, 0, bIsIdentical);
			const bool bValueHasChildProperties = ValuePropertyCustomization->HasChildProperties(MapProperty->ValueProp, ValueRawPtr, 0, bIsIdentical);
			{
				const FString Name = FString::FromInt(ElemIdx);
				if (bKeyHasChildProperties || bValueHasChildProperties)
				{
					bIsShowChildren = ImGui::TreeNode(TCHAR_TO_UTF8(*Name));
				}
				else
				{
					constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
					ImGui::TreeNodeEx(TCHAR_TO_UTF8(*Name), flags);
				}
				if (InnerValue::GPostPropertyNameWidgetCreated.IsBound())
				{
					InnerValue::GPostPropertyNameWidgetCreated.Execute(Property, Containers, Offset, bKeyIsIdentical && bValueIsIdentical, bKeyHasChildProperties);
				}
				
				ImGui::SameLine();
				if (Customization)
				{
					Customization->CreateValueWidget(MapProperty->KeyProp, KeyRawPtr, 0, bKeyIsIdentical);
				}
				else
				{
					KeyPropertyCustomization->CreateValueWidget(MapProperty->KeyProp, KeyRawPtr, 0, bKeyIsIdentical);
				}
				if (bKeyHasChildProperties && bIsShowChildren)
				{
					if (Customization)
					{
						Customization->CreateChildrenWidget(MapProperty->KeyProp, KeyRawPtr, 0, bKeyIsIdentical);
					}
					else
					{
						KeyPropertyCustomization->CreateChildrenWidget(MapProperty->KeyProp, KeyRawPtr, 0, bKeyIsIdentical);
					}
					ImGui::TreePop();
				}
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(InnerValue::ContainerValueRightWidth - (Customization ? Customization->ValueAdditiveRightWidth : 0.f));
			{
				if (Customization)
				{
					Customization->CreateValueWidget(MapProperty->ValueProp, ValueRawPtr, 0, bValueIsIdentical);
				}
				else
				{
					ValuePropertyCustomization->CreateValueWidget(MapProperty->ValueProp, ValueRawPtr, 0, bValueIsIdentical);
				}
				ImGui::SameLine();
				const auto MapPopupName = "UnrealMapPopup";
				if (ImGui::ArrowButton(">", ImGuiDir_Down))
				{
					ImGui::OpenPopup(MapPopupName);
				}
				if (ImGui::BeginPopup(MapPopupName))
				{
					if (ImGui::MenuItem("Delete"))
					{
						for (int32 Idx = 0; Idx < Containers.Num(); ++Idx)
						{
							Helpers[Idx].RemoveAt(Iterators[Idx].GetInternalIndex());
						}
						NotifyPostPropertyValueChanged(Property);
					}
					ImGui::EndPopup();
				}
				if (bValueHasChildProperties && bIsShowChildren)
				{
					if (Customization)
					{
						Customization->CreateChildrenWidget(MapProperty->ValueProp, ValueRawPtr, 0, bValueIsIdentical);
					}
					else
					{
						ValuePropertyCustomization->CreateChildrenWidget(MapProperty->ValueProp, ValueRawPtr, 0, bValueIsIdentical);
					}
					ImGui::TreePop();
				}
			}
			for (auto& It : Iterators)
			{
				++It;
			}

			ImGui::NextColumn();
		}
	}

	bool FStructPropertyCustomization::IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		if (IUnrealPropertyCustomization::IsVisible(Filter, Property, Containers, Offset, bIsIdentical))
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
			if (PropertyCustomization && InnerValue::IsVisible(ChildProperty, Containers, Offset + ChildProperty->GetOffset_ForInternal(), bIsIdentical, PropertyCustomization))
			{
				return true;
			}
		}
		return false;
	}

	void FStructPropertyCustomization::CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
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
		ImGui::Text("%d Elements %s", ElementCount, bIsIdentical ? "" : "*");
	}

	void FStructPropertyCustomization::CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool bIsIdentical) const
	{
		const FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Property);
		DrawStructDefaultDetails(StructProperty->Struct, Containers, Offset);
	}
}
