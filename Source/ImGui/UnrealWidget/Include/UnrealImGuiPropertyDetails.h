// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace UnrealImGui
{
	namespace GlobalValue
	{
		IMGUI_API extern bool GEnableEditVisibleProperty;
		IMGUI_API extern bool GDisplayAllProperties;
	}
	
	constexpr int32 ArrayInlineAllocateSize = 8;
	using FStructArray = TArray<uint8*, TInlineAllocator<ArrayInlineAllocateSize>>;
	using FObjectArray = TArray<UObject*, TInlineAllocator<ArrayInlineAllocateSize>>;
	template<typename T>
	struct TObjectArray : public TArray<T*, TInlineAllocator<ArrayInlineAllocateSize>>
	{
		operator const FObjectArray& () const
		{
			return reinterpret_cast<const FObjectArray&>(*this);
		}
	};
	namespace InnerValue
	{
		constexpr float ValueRightBaseWidth = -10.f;
		constexpr float ContainerValueRightWidth = ValueRightBaseWidth - 30.f;

		extern int32 GPropertyDepth;
		extern int32 GImGuiContainerIndex;
		extern FObjectArray GOuters;
	}
	
	struct IMGUI_API IUnrealPropertyCustomization : public TSharedFromThis<IUnrealPropertyCustomization>
	{
		IUnrealPropertyCustomization()
			: bHasChildProperties(false)
			, bOverrideHasChildProperties(false)
		{}
		
		// 值控件右侧剩余空间
		int32 ValueAdditiveRightWidth = 0;
		uint8 bHasChildProperties : 1;
		uint8 bOverrideHasChildProperties : 1;
		
		bool HasChildProperties(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const
		{
			return bOverrideHasChildProperties ? HasChildPropertiesOverride(Property, Containers, Offset, IsIdentical) : bHasChildProperties;
		}
		
		virtual bool HasChildPropertiesOverride(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const { return false; }
		virtual void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const = 0;
		virtual void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const {}

		virtual ~IUnrealPropertyCustomization() {}
	};

	struct IMGUI_API IUnrealStructCustomization : public TSharedFromThis<IUnrealStructCustomization>
	{
		// 值控件右侧剩余空间
		int32 ValueAdditiveRightWidth = 0;

		virtual void CreateNameWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren) const;
		virtual void CreateValueWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const = 0;
		virtual void CreateChildrenWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical) const {}

		virtual ~IUnrealStructCustomization() {}
	};

	struct IMGUI_API IUnrealClassCustomization : public TSharedFromThis<IUnrealClassCustomization>
	{
		virtual void CreateClassDetails(const UClass* Class, const FObjectArray& Containers, int32 Offset) const;

		virtual ~IUnrealClassCustomization() {}
	};
	
	struct IMGUI_API UnrealPropertyCustomizeFactory
	{
	public:
		using PropertyCustomizeFunc = TFunction<void(const FProperty * Property, const FStructArray & Containers, int32 Offset)>;

		static TSharedPtr<IUnrealPropertyCustomization> FindCustomizer(const FProperty* Property);
		static TSharedPtr<IUnrealStructCustomization> FindCustomizer(const UStruct* Struct);
		static TSharedPtr<IUnrealClassCustomization> FindCustomizer(const UClass* Class);

		static void AddPropertyCustomizer(FFieldClass* FieldClass, const TSharedRef<IUnrealPropertyCustomization>& Customization)
		{
			PropertyCustomizeMap.Add(FieldClass, Customization);
		}
		static void AddStructCustomizer(UStruct* Struct, const TSharedRef<IUnrealStructCustomization>& Customization)
		{
			check(Struct);
			StructCustomizeMap.Add(Struct, Customization);
		}
		static void AddClassCustomizer(UClass* Class, const TSharedRef<IUnrealClassCustomization>& Customization)
		{
			ClassCustomizeMap.Add(Class, Customization);
		}

		struct FPropertyCustomizerScoped
		{
			FFieldClass* FieldClass;
			TSharedPtr<IUnrealPropertyCustomization> Customization;

			FPropertyCustomizerScoped(FFieldClass* FieldClass, const TSharedRef<IUnrealPropertyCustomization>& InCustomization)
				: FieldClass(FieldClass)
			{
				if (TSharedRef<IUnrealPropertyCustomization>* ExistedCustomization = PropertyCustomizeMap.Find(FieldClass))
				{
					Customization = *ExistedCustomization;
				}
				PropertyCustomizeMap.Add(FieldClass, InCustomization);
			}
			~FPropertyCustomizerScoped()
			{
				if (Customization.IsValid())
				{
					PropertyCustomizeMap.FindOrAdd(FieldClass, Customization.ToSharedRef());
				}
				else
				{
					PropertyCustomizeMap.Remove(FieldClass);
				}
			}
		};

		struct FStructCustomizerScoped
		{
			UStruct* Struct;
			TSharedPtr<IUnrealStructCustomization> Customization;

			FStructCustomizerScoped(UStruct* Struct, const TSharedRef<IUnrealStructCustomization>& InCustomization)
				: Struct(Struct)
			{
				if (TSharedRef<IUnrealStructCustomization>* ExistedCustomization = StructCustomizeMap.Find(Struct))
				{
					Customization = *ExistedCustomization;
				}
				StructCustomizeMap.Add(Struct, InCustomization);
			}
			~FStructCustomizerScoped()
			{
				if (Customization.IsValid())
				{
					StructCustomizeMap.FindOrAdd(Struct, Customization.ToSharedRef());
				}
				else
				{
					StructCustomizeMap.Remove(Struct);
				}
			}
		};

		struct FClassCustomizerScoped
		{
			UClass* Class;
			TSharedPtr<IUnrealClassCustomization> Customization;

			FClassCustomizerScoped(UClass* Class, const TSharedRef<IUnrealClassCustomization>& InCustomization)
				: Class(Class)
			{
				if (TSharedRef<IUnrealClassCustomization>* ExistedCustomization = ClassCustomizeMap.Find(Class))
				{
					Customization = *ExistedCustomization;
				}
				ClassCustomizeMap.Add(Class, InCustomization);
			}
			~FClassCustomizerScoped()
			{
				if (Customization.IsValid())
				{
					ClassCustomizeMap.FindOrAdd(Class, Customization.ToSharedRef());
				}
				else
				{
					ClassCustomizeMap.Remove(Class);
				}
			}
		};
	private:
		friend class FImGuiModule;
		static void InitialDefaultCustomizer();

		static TMap<FFieldClass*, TSharedRef<IUnrealPropertyCustomization>> PropertyCustomizeMap;
		static TMap<UStruct*, TSharedRef<IUnrealStructCustomization>> StructCustomizeMap;
		static TMap<UClass*, TSharedRef<IUnrealClassCustomization>> ClassCustomizeMap;
	};

	DECLARE_DELEGATE_OneParam(FPostPropertyValueChanged, const FProperty*);
	
	inline bool IsPropertyShow(const FProperty* Property)
	{
		if (GlobalValue::GDisplayAllProperties)
		{
			return true;
		}
		return Property->HasAllPropertyFlags(CPF_Edit) && Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance) == false;
	}

	IMGUI_API void CreateUnrealPropertyNameWidget(const FProperty* Property, const FStructArray& Containers, int32 Offset, bool IsIdentical, bool HasChildProperties, bool& IsShowChildren, const FString* NameOverride = nullptr);
	
	IMGUI_API void AddUnrealProperty(const FProperty* Property, const FStructArray& Containers, int32 Offset = 0);

	IMGUI_API void DrawDefaultStructDetails(const UStruct* TopStruct, const FStructArray& Instances, int32 Offset);
	IMGUI_API void DrawDefaultClassDetails(const UClass* TopClass, bool CollapseCategories, const FObjectArray& Instances, int32 Offset);

	IMGUI_API void DrawDetailTable(const char* str_id, UStruct* TopStruct, const FStructArray& Instances, const FPostPropertyValueChanged& PostPropertyValueChanged = {});
	IMGUI_API void DrawDetailTable(const char* str_id, UClass* TopClass, const FObjectArray& Instances, const FPostPropertyValueChanged& PostPropertyValueChanged = {});

	IMGUI_API UClass* GetTopClass(const FObjectArray& Instances, const UClass* StopClass = UObject::StaticClass());
	
	IMGUI_API bool IsAllPropertiesIdentical(const FProperty* Property, const FStructArray& Containers, int32 Offset);
	IMGUI_API FString GetPropertyDefaultLabel(const FProperty* Property, bool IsIdentical);
	IMGUI_API FString CreatePropertyLabel(const FProperty* Property, const FString& Name);
	IMGUI_API void NotifyPostPropertyValueChanged(const FProperty* Property);
}
