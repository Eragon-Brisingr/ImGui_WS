// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImGuiEditorStyleSet.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

FName FImGuiEditorStyleSet::ImGuiIconName("ImGui.Icon");
FName FImGuiEditorStyleSet::StyleName("ImGuiEditorStyle");
TUniquePtr<FImGuiEditorStyleSet> FImGuiEditorStyleSet::Inst(nullptr);

const FImGuiEditorStyleSet& FImGuiEditorStyleSet::Get()
{
	ensure(Inst.IsValid());
	return *(Inst.Get());
}

void FImGuiEditorStyleSet::Initialize()
{
	if (!Inst.IsValid())
	{
		Inst = TUniquePtr<FImGuiEditorStyleSet>(new FImGuiEditorStyleSet);
	}
}

void FImGuiEditorStyleSet::Shutdown()
{
	if (Inst.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Inst.Get());
		Inst.Reset();
	}
}

FImGuiEditorStyleSet::FImGuiEditorStyleSet()
	: FSlateStyleSet(StyleName)
{
	SetParentStyleName(FAppStyle::GetAppStyleSetName());
	const FString Path = IPluginManager::Get().FindPlugin(TEXT("ImGui_WS"))->GetBaseDir() / TEXT("Resources");
	FSlateStyleSet::SetContentRoot(Path);

	Set(ImGuiIconName, new FSlateVectorImageBrush(FSlateStyleSet::RootToContentDir(TEXT("ImGui.svg")), CoreStyleConstants::Icon16x16));
	
	FSlateStyleRegistry::RegisterSlateStyle(*this);	
}

const FName& FImGuiEditorStyleSet::GetStyleSetName() const
{
	return StyleName;
}

