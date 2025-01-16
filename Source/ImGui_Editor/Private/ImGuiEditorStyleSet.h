// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

class FImGuiEditorStyleSet : public FSlateStyleSet
{
public:
	static const FImGuiEditorStyleSet& Get();

	static void Initialize();

	static void Shutdown();

	static FName ImGuiIconName;
	static FName StyleName;
private:
	FImGuiEditorStyleSet();

	virtual const FName& GetStyleSetName() const override;

	static TUniquePtr<FImGuiEditorStyleSet> Inst;
};
