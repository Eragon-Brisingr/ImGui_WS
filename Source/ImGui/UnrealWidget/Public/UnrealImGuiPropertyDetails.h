// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

class FFieldClass;

namespace UnrealImGui
{
	struct IUnrealStructCustomization;
	struct IUnrealPropertyCustomization;

	namespace GlobalValue
	{
		IMGUI_API extern bool GEnableEditVisibleProperty;
		IMGUI_API extern bool GDisplayAllProperties;
	}
	
	constexpr int32 ArrayInlineAllocateSize = 1;
	using FPtrArray = TArray<uint8*, TInlineAllocator<ArrayInlineAllocateSize>>;
	template<typename T>
	struct TStructArray : TArray<T*, TInlineAllocator<ArrayInlineAllocateSize>>
	{
		TStructArray() = default;
		TStructArray(std::initializer_list<T*> InitList)
			: TArray<T*, TInlineAllocator<ArrayInlineAllocateSize>>{ InitList }
		{}
		operator const FPtrArray& () const
		{
			return reinterpret_cast<const FPtrArray&>(*this);
		}
	};
	using FObjectArray = TArray<UObject*, TInlineAllocator<ArrayInlineAllocateSize>>;
	template<typename T>
	struct TObjectArray : TArray<T*, TInlineAllocator<ArrayInlineAllocateSize>>
	{
		TObjectArray() = default;
		TObjectArray(std::initializer_list<T*> InitList)
			: TArray<T*, TInlineAllocator<ArrayInlineAllocateSize>>{ InitList }
		{}
		operator const FObjectArray& () const
		{
			return reinterpret_cast<const FObjectArray&>(*this);
		}
	};
	struct IMGUI_API FDetailsFilter
	{
		FString StringFilter;
		void Draw(const char* Label = "##DetailsFilter");
		bool IsFilterEnable() const { return StringFilter.IsEmpty() == false; }
	};
	namespace InnerValue
	{
		constexpr float ValueRightBaseWidth = -10.f;
		constexpr float ContainerValueRightWidth = ValueRightBaseWidth - 30.f;

		extern int32 GPropertyDepth;
		extern int32 GImGuiContainerIndex;
		extern FObjectArray GOuters;
		extern const FDetailsFilter* GFilter;

		struct FFilterCacheKey
		{
			const void* MemoryKey;
			const FProperty* Property;
			friend bool operator==(const FFilterCacheKey& LHS, const FFilterCacheKey& RHS) { return LHS.MemoryKey == RHS.MemoryKey && LHS.Property == RHS.Property; }
			friend uint32 GetTypeHash(const FFilterCacheKey& Key) { return PointerHash(Key.MemoryKey); }
		};
		extern TMap<FFilterCacheKey, bool> FilterCacheMap;
		bool IsVisible(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, const TSharedPtr<IUnrealPropertyCustomization>& PropertyCustomization, const TSharedPtr<IUnrealStructCustomization>& Customization);
		bool IsVisible(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, const TSharedPtr<IUnrealPropertyCustomization>& PropertyCustomization);
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

		virtual bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const;
		bool HasChildProperties(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const
		{
			return bOverrideHasChildProperties ? HasChildPropertiesOverride(Property, Containers, Offset, IsIdentical) : bHasChildProperties;
		}
		
		virtual bool HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const { return false; }
		virtual void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const = 0;
		virtual void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const {}

		virtual ~IUnrealPropertyCustomization() {}
	};

	struct IMGUI_API IUnrealStructCustomization : public TSharedFromThis<IUnrealStructCustomization>
	{
		// 值控件右侧剩余空间
		int32 ValueAdditiveRightWidth = 0;

		virtual bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const;
		virtual void CreateNameWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool& IsShowChildren) const;
		virtual void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const = 0;
		virtual void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const {}

		virtual ~IUnrealStructCustomization() {}
	};

	struct IMGUI_API IUnrealDetailsCustomization : public TSharedFromThis<IUnrealDetailsCustomization>
	{
		virtual void CreateClassDetails(const UClass* Class, const FObjectArray& Containers, int32 Offset) const;

		virtual ~IUnrealDetailsCustomization() {}
	};
	
	struct IMGUI_API UnrealPropertyCustomizeFactory
	{
	public:
		using PropertyCustomizeFunc = TFunction<void(const FProperty * Property, const FPtrArray & Containers, int32 Offset)>;

		static TSharedPtr<IUnrealPropertyCustomization> FindPropertyCustomizer(const FProperty* Property);
		static TSharedPtr<IUnrealStructCustomization> FindPropertyCustomizer(const UStruct* Struct);
		static TSharedPtr<IUnrealDetailsCustomization> FindDetailsCustomizer(const UClass* Class);

		static void AddPropertyCustomizer(FFieldClass* FieldClass, const TSharedRef<IUnrealPropertyCustomization>& Customization)
		{
			PropertyCustomizeMap.Add(FieldClass, Customization);
		}
		static void AddStructCustomizer(UStruct* Struct, const TSharedRef<IUnrealStructCustomization>& Customization)
		{
			check(Struct);
			StructCustomizeMap.Add(Struct, Customization);
		}
		static void AddClassCustomizer(UClass* Class, const TSharedRef<IUnrealDetailsCustomization>& Customization)
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
				if (const TSharedRef<IUnrealPropertyCustomization>* ExistedCustomization = PropertyCustomizeMap.Find(FieldClass))
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
				if (const TSharedRef<IUnrealStructCustomization>* ExistedCustomization = StructCustomizeMap.Find(Struct))
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
			TSharedPtr<IUnrealDetailsCustomization> Customization;

			FClassCustomizerScoped(UClass* Class, const TSharedRef<IUnrealDetailsCustomization>& InCustomization)
				: Class(Class)
			{
				if (const TSharedRef<IUnrealDetailsCustomization>* ExistedCustomization = ClassCustomizeMap.Find(Class))
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
		static void InitialDefaultCustomizer();
	private:
		static TMap<FFieldClass*, TSharedRef<IUnrealPropertyCustomization>> PropertyCustomizeMap;
		static TMap<UStruct*, TSharedRef<IUnrealStructCustomization>> StructCustomizeMap;
		static TMap<UClass*, TSharedRef<IUnrealDetailsCustomization>> ClassCustomizeMap;
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

	IMGUI_API void CreateUnrealPropertyNameWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical, bool HasChildProperties, bool& IsShowChildren, const FString* NameOverride = nullptr);
	
	IMGUI_API void AddUnrealProperty(const FProperty* Property, const FPtrArray& Containers, int32 Offset);

	IMGUI_API void DrawDefaultStructDetails(const UStruct* TopStruct, const FPtrArray& Instances, int32 Offset);
	IMGUI_API void DrawDefaultClassDetails(const UClass* TopClass, bool CollapseCategories, const FObjectArray& Instances, int32 Offset);

	IMGUI_API void DrawDetailTable(const char* str_id, const UStruct* TopStruct, const FPtrArray& Instances, const FDetailsFilter* Filter = nullptr, const FPostPropertyValueChanged& PostPropertyValueChanged = {});
	IMGUI_API void DrawDetailTable(const char* str_id, const UClass* TopClass, const FObjectArray& Instances, const FDetailsFilter* Filter = nullptr, const FPostPropertyValueChanged& PostPropertyValueChanged = {});

	IMGUI_API UClass* GetTopClass(const FObjectArray& Objects, const UClass* StopClass = UObject::StaticClass());
	IMGUI_API const UStruct* GetTopStruct(const TArrayView<const UStruct*> Structs, const UStruct* StopStruct = nullptr);
	
	IMGUI_API bool IsAllPropertiesIdentical(const FProperty* Property, const FPtrArray& Containers, int32 Offset);
	IMGUI_API FString GetPropertyDefaultLabel(const FProperty* Property, bool IsIdentical);
	IMGUI_API FString CreatePropertyLabel(const FProperty* Property, const FString& Name);
	IMGUI_API void NotifyPostPropertyValueChanged(const FProperty* Property);
}
