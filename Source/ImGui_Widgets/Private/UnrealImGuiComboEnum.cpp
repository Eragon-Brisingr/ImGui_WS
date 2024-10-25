// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiComboEnum.h"

#include "ImGuiEx.h"
#include "UObject/Class.h"

namespace UnrealImGui
{
	bool ComboEnum(const char* Label, int64& EnumValue, const UEnum* EnumType, ImGuiComboFlags Flags)
	{
		if (!ensure(EnumType))
		{
			return false;
		}
		bool bValueChanged = false;
		const int32 ShortNameStartIdx = EnumType->GetName().Len() + 2;
		const auto PreviewEnumName = EnumType->GetNameByValue(EnumValue);
		if (auto Combo = ImGui::FCombo{ Label, PreviewEnumName != NAME_None ? TCHAR_TO_UTF8(*PreviewEnumName.ToString().Mid(ShortNameStartIdx)) : "None", Flags })
		{
			for (int32 Idx = 0; Idx < EnumType->NumEnums() - 1; ++Idx)
			{
				ImGui::FIdScope IdScope{ Idx };
				const auto Name = EnumType->GetNameByIndex(Idx).ToString().Mid(ShortNameStartIdx);
				const auto Value = EnumType->GetValueByIndex(Idx);
				const bool bIsSelected = Value == EnumValue;
				if (ImGui::Selectable(TCHAR_TO_UTF8(*Name), bIsSelected) && !bIsSelected)
				{
					bValueChanged = true;
					EnumValue = Value;
					ImGui::CloseCurrentPopup();
				}
				if (bIsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		return bValueChanged;
	}
}
