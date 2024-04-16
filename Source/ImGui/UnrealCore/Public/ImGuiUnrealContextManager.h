// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "ImGuiUnrealContextManager.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnImGui_WS_Draw, float);

USTRUCT()
struct IMGUI_API FImGuiUnrealContext
{
	GENERATED_BODY()
public:
	FOnImGui_WS_Draw OnDraw;
};

USTRUCT()
struct IMGUI_API FImGuiUnrealEditorContext : public FImGuiUnrealContext
{
	GENERATED_BODY()
public:
	bool bAlwaysDrawDefaultLayout = false;
};

UCLASS()
class IMGUI_API UImGuiUnrealContextWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static UImGuiUnrealContextWorldSubsystem* Get(const UObject* WorldContextObject);

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override { return WorldType == EWorldType::Game || WorldType == EWorldType::PIE; }

	UPROPERTY()
	FImGuiUnrealContext Context;
};

UCLASS()
class IMGUI_API UImGuiUnrealContextManager : public UEngineSubsystem
{
	GENERATED_BODY()

	friend UImGuiUnrealContextWorldSubsystem;
public:
	static UImGuiUnrealContextManager* GetChecked();
	static FImGuiUnrealContext* GetImGuiContext(const UWorld* World);
	static FImGuiUnrealEditorContext* GetImGuiEditorContext();

	void DrawViewport(int32& ContextIndex, float DeltaSeconds);
	const TArray<UImGuiUnrealContextWorldSubsystem*>& GetWorldSubsystems() const { return WorldSubsystems; }
	static constexpr int32 EditorContextIndex = INDEX_NONE;

#if WITH_EDITOR
	TFunction<void(float)> EditorDrawer;
#endif
private:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FImGuiUnrealEditorContext EditorContext;
#endif

	UPROPERTY(Transient)
	TArray<UImGuiUnrealContextWorldSubsystem*> WorldSubsystems;
};
