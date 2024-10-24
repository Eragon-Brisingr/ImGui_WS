// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealImGuiPropertyDetails.h"
#include "AssetRegistry/AssetData.h"

class AActor;

namespace UnrealImGui
{
	struct IMGUI_WIDGETS_API FPropertyDisableScope
	{
		FPropertyDisableScope(const FProperty* Property);
		~FPropertyDisableScope();
	private:
		bool Disable = false;
	};

	struct IMGUI_WIDGETS_API FPropertyEnableScope
	{
		FPropertyEnableScope();
		~FPropertyEnableScope();
	private:
		bool Enable = false;
	};

	struct IMGUI_WIDGETS_API FBoolPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FNumericPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FObjectPropertyBaseCustomization : IUnrealPropertyCustomization
	{
		FObjectPropertyBaseCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		bool HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FObjectPropertyCustomization : FObjectPropertyBaseCustomization {};

	struct IMGUI_WIDGETS_API FWeakObjectPropertyCustomization : FObjectPropertyBaseCustomization {};

	struct IMGUI_WIDGETS_API FSoftObjectPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FClassPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FSoftClassPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FStringPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FNamePropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FTextPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FEnumPropertyCustomization : IUnrealPropertyCustomization
	{
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FArrayPropertyCustomization : IUnrealPropertyCustomization
	{
		FArrayPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		bool HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FSetPropertyCustomization : IUnrealPropertyCustomization
	{
		FSetPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		bool HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FMapPropertyCustomization : IUnrealPropertyCustomization
	{
		FMapPropertyCustomization()
		{
			bOverrideHasChildProperties = true;
		}

		bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		bool HasChildPropertiesOverride(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};

	struct IMGUI_WIDGETS_API FStructPropertyCustomization : IUnrealPropertyCustomization
	{
		FStructPropertyCustomization()
		{
			bHasChildProperties = true;
		}

		bool IsVisible(const FDetailsFilter& Filter, const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateValueWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
		void CreateChildrenWidget(const FProperty* Property, const FPtrArray& Containers, int32 Offset, bool IsIdentical) const override;
	};
}