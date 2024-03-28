// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Input/IVirtualKeyboardEntry.h"

struct ImPlotContext;
struct ImGuiContext;
struct ImDrawData;
struct FImGuiDrawList;

class IMGUI_SLATE_API SImGuiPanel : public SLeafWidget, public IVirtualKeyboardEntry, public ITextInputMethodContext
{
	using Super = SLeafWidget;
public:
	DECLARE_DELEGATE_OneParam(FOnImGuiTick, float /*DeltaSeconds*/);

	SLATE_BEGIN_ARGS(SImGuiPanel)
	{}
	SLATE_EVENT(FOnImGuiTick, OnImGuiTick)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	~SImGuiPanel() override;

	ImGuiContext* GetContext() const { return Context; }

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D::ZeroVector; }
	bool SupportsKeyboardFocus() const override { return true; }
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) override;
	FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& Event) override;
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

	// IVirtualKeyboardEntry start
	void SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType) override;
	void SetSelectionFromVirtualKeyboard(int InSelStart, int SelEnd) override;
	FText GetText() const override;
	bool GetSelection(int& OutSelStart, int& OutSelEnd) override;
	FText GetHintText() const override;
	EKeyboardType GetVirtualKeyboardType() const override;
	FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
	bool IsMultilineEntry() const override;
	// IVirtualKeyboardEntry end

	// ITextInputMethodContext start
	bool IsComposing() override;
	bool IsReadOnly() override;
	uint32 GetTextLength() override;
	void GetSelectionRange(uint32& OutBeginIndex, uint32& OutLength, ECaretPosition& OutCaretPosition) override;
	void SetSelectionRange(const uint32 InBeginIndex, const uint32 InLength, const ECaretPosition InCaretPosition) override;
	void GetTextInRange(const uint32 InBeginIndex, const uint32 InLength, FString& OutString) override;
	void SetTextInRange(const uint32 InBeginIndex, const uint32 InLength, const FString& InString) override;
	int32 GetCharacterIndexFromPoint(const FVector2D& InPoint) override;
	bool GetTextBounds(const uint32 InBeginIndex, const uint32 InLength, FVector2D& OutPosition, FVector2D& OutSize) override;
	void GetScreenBounds(FVector2D& OutPosition, FVector2D& OutSize) override;
	TSharedPtr<FGenericWindow> GetWindow() override;
	void BeginComposition() override;
	void UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength) override;
	void EndComposition() override;
	// ITextInputMethodContext end
private:
	struct IMGUI_SLATE_API FImGuiDrawData
	{
		FImGuiDrawData();
		~FImGuiDrawData();
		explicit FImGuiDrawData(const ImDrawData* Source);

		bool bValid = false;

		int32 TotalIdxCount = 0;
		int32 TotalVtxCount = 0;

		TArray<FImGuiDrawList> DrawLists;

		FVector2f DisplayPos = FVector2f::ZeroVector;
		FVector2f DisplaySize = FVector2f::ZeroVector;
		FVector2f FrameBufferScale = FVector2f::ZeroVector;
	};
	FImGuiDrawData DrawData;
	ImGuiContext* Context = nullptr;
	ImPlotContext* PlotContext = nullptr;
	FOnImGuiTick OnImGuiTick;
	struct
	{
		uint8 bEnable : 1 { false };
		uint8 bIsComposing : 1 { false };

		uint32 SelectionRangeBeginIndex;
		uint32 SelectionRangeLength;
		ECaretPosition SelectionCaretPosition;
		FGeometry CachedGeometry;
		FString CompositionString;
		TSharedPtr<ITextInputMethodChangeNotifier> TextInputMethodChangeNotifier;
	} VirtualInput{};

	void EnableVirtualInput();
	void DisableVirtualInput();
};

