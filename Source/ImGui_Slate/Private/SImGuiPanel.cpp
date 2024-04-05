// Fill out your copyright notice in the Description page of Project Settings.


#include "SImGuiPanel.h"

#include "imgui.h"
#include "ImGuiDelegates.h"
#include "ImGuiFontAtlas.h"
#include "imgui_internal.h"
#include "implot.h"
#include "UnrealImGuiKeyUtils.h"
#include "UnrealImGuiTexture.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include "HAL/PlatformApplicationMisc.h"

namespace ImGui
{
FColor ConvertColor(const uint32 Color)
{
	return FColor(
		(Color >> IM_COL32_R_SHIFT) & 0xFF,
		(Color >> IM_COL32_G_SHIFT) & 0xFF,
		(Color >> IM_COL32_B_SHIFT) & 0xFF,
		(Color >> IM_COL32_A_SHIFT) & 0xFF
	);
}

struct FScopedContext
{
	FScopedContext(ImGuiContext* Context)
		: PreContext(ImGui::GetCurrentContext())
	{
		ImGui::SetCurrentContext(Context);
	}
	~FScopedContext()
	{
		ImGui::SetCurrentContext(PreContext);
	}
private:
	ImGuiContext* PreContext;
};
}

struct FImGuiDrawList
{
	FImGuiDrawList() = default;
	explicit FImGuiDrawList(ImDrawList* Source);

	ImVector<ImDrawVert> VtxBuffer;
	ImVector<ImDrawIdx> IdxBuffer;
	ImVector<ImDrawCmd> CmdBuffer;
	ImDrawListFlags Flags = ImDrawListFlags_None;
};

FImGuiDrawList::FImGuiDrawList(ImDrawList* Source)
{
	VtxBuffer.swap(Source->VtxBuffer);
	IdxBuffer.swap(Source->IdxBuffer);
	CmdBuffer.swap(Source->CmdBuffer);
	Flags = Source->Flags;
}

SImGuiPanel::FImGuiDrawData::FImGuiDrawData() {}

SImGuiPanel::FImGuiDrawData::~FImGuiDrawData() {}

SImGuiPanel::FImGuiDrawData::FImGuiDrawData(const ImDrawData* Source)
{
	bValid = Source->Valid;

	TotalIdxCount = Source->TotalIdxCount;
	TotalVtxCount = Source->TotalVtxCount;

	DrawLists.SetNumUninitialized(Source->CmdListsCount);
	ConstructItems<FImGuiDrawList>(DrawLists.GetData(), Source->CmdLists.Data, Source->CmdListsCount);

	DisplayPos = FVector2f{ Source->DisplayPos };
	DisplaySize = FVector2f{ Source->DisplaySize };
	FrameBufferScale = FVector2f{ Source->FramebufferScale };
}

void SImGuiPanel::Construct(const FArguments& Args)
{
	ImGui::FScopedContext ScopedContext{ nullptr };
	Context = ImGui::CreateContext(&UnrealImGui::GetDefaultFontAtlas());
	PlotContext = ImPlot::CreateContext();
	OnImGuiTick = Args._OnImGuiTick;
	DesiredSize = Args._DesiredSize;
}

SImGuiPanel::~SImGuiPanel()
{
	DisableVirtualInput();
	UnrealImGui::OnImGuiContextDestroyed.Broadcast(Context);
	ImGui::DestroyContext(Context);
	ImPlot::DestroyContext(PlotContext);
}

void SImGuiPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	ImGui::FScopedContext ScopedContext{ Context };

	ImGuiIO& IO = ImGui::GetIO();
	IO.DisplaySize = ImVec2{ AllottedGeometry.GetAbsoluteSize() };
	IO.DeltaTime = InDeltaTime;

	FSlateApplication& SlateApp = FSlateApplication::Get();
	const bool bHasGamepad = (IO.BackendFlags & ImGuiBackendFlags_HasGamepad);
	if (bHasGamepad != SlateApp.IsGamepadAttached())
	{
		IO.BackendFlags ^= ImGuiBackendFlags_HasGamepad;
	}

	if (HasAnyUserFocus())
	{
		if (IO.WantSetMousePos)
		{
			SlateApp.SetCursorPos(FVector2D{ IO.MousePos });
		}

		if (IO.WantCaptureKeyboard && !HasKeyboardFocus())
		{
			// No HandleKeyCharEvent so punt focus to the widget for it to receive OnKeyChar events
			SlateApp.SetKeyboardFocus(AsShared());
		}

		if (IO.WantTextInput)
		{
			EnableVirtualInput();
		}
		else
		{
			DisableVirtualInput();
		}
	}

	ImPlotContext* OldPlotContent = ImPlot::GetCurrentContext();
	ON_SCOPE_EXIT
	{
		ImPlot::SetCurrentContext(OldPlotContent);
	};
	ImPlot::SetCurrentContext(PlotContext);

	ImGui::NewFrame();

	if (ensure(OnImGuiTick.IsBound()))
	{
		OnImGuiTick.Execute(InDeltaTime);
	}

	ImGui::Render();

	const ImDrawData* Data = ImGui::GetDrawData();
	DrawData = FImGuiDrawData{ Data };

	ImGui::EndFrame();
}

int32 SImGuiPanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!DrawData.bValid)
	{
		return LayerId;
	}

	const FSlateRenderTransform Transform(AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() - FVector2d(DrawData.DisplayPos));

	FSlateBrush TextureBrush;
	for (const FImGuiDrawList& DrawList : DrawData.DrawLists)
	{
		TArray<FSlateVertex> Vertices;
		Vertices.SetNumUninitialized(DrawList.VtxBuffer.Size);
		for (int32 BufferIdx = 0; BufferIdx < Vertices.Num(); ++BufferIdx)
		{
			const ImDrawVert& Vtx = DrawList.VtxBuffer.Data[BufferIdx];
			Vertices[BufferIdx] = FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, FVector2f{ Vtx.pos }, FVector2f{ Vtx.uv }, FVector2f::UnitVector, ImGui::ConvertColor(Vtx.col));
		}

		TArray<SlateIndex> Indices;
		Indices.SetNumUninitialized(DrawList.IdxBuffer.Size);
		for (int32 BufferIdx = 0; BufferIdx < Indices.Num(); ++BufferIdx)
		{
			Indices[BufferIdx] = DrawList.IdxBuffer.Data[BufferIdx];
		}

		for (const ImDrawCmd& DrawCmd : DrawList.CmdBuffer)
		{
			TArray VerticesSlice(Vertices.GetData() + DrawCmd.VtxOffset, Vertices.Num() - DrawCmd.VtxOffset);
			TArray IndicesSlice(Indices.GetData() + DrawCmd.IdxOffset, DrawCmd.ElemCount);

			const UTexture* Texture = UnrealImGui::FindTexture(DrawCmd.GetTexID());
			if (TextureBrush.GetResourceObject() != Texture)
			{
				TextureBrush.SetResourceObject(const_cast<UTexture*>(Texture));
				if (IsValid(Texture))
				{
					if (const UTexture2D* Texture2D = Cast<UTexture2D>(Texture))
					{
						TextureBrush.ImageSize.X = Texture2D->GetSizeX();
						TextureBrush.ImageSize.Y = Texture2D->GetSizeY();
					}
					else if (const UTextureRenderTarget2D* RT = Cast<UTextureRenderTarget2D>(Texture))
					{
						TextureBrush.ImageSize.X = RT->SizeX;
						TextureBrush.ImageSize.Y = RT->SizeY;
					}
					else
					{
						ensure(false);
					}
					TextureBrush.ImageType = ESlateBrushImageType::FullColor;
					TextureBrush.DrawAs = ESlateBrushDrawType::Image;
				}
				else
				{
					TextureBrush.ImageSize.X = 0;
					TextureBrush.ImageSize.Y = 0;
					TextureBrush.ImageType = ESlateBrushImageType::NoImage;
					TextureBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
				}
			}

			FSlateRect ClipRect(DrawCmd.ClipRect.x, DrawCmd.ClipRect.y, DrawCmd.ClipRect.z, DrawCmd.ClipRect.w);
			ClipRect = TransformRect(Transform, ClipRect);

			OutDrawElements.PushClip(FSlateClippingZone(ClipRect));
			FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, TextureBrush.GetRenderingResource(), VerticesSlice, IndicesSlice, nullptr, 0, 0);
			OutDrawElements.PopClip();
		}
	}

	return LayerId;
}

FVector2D SImGuiPanel::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return DesiredSize.Get();
}

FReply SImGuiPanel::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ImGui::FScopedContext ScopedContext{ Context };
	ImGuiIO& IO = ImGui::GetIO();
	const FVector2f Position = MouseEvent.GetScreenSpacePosition() - MyGeometry.GetAbsolutePosition();
	IO.AddMousePosEvent(Position.X, Position.Y);
	return IO.WantCaptureMouse ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ImGui::FScopedContext ScopedContext{ Context };
	ImGuiIO& IO = ImGui::GetIO();
	const FVector2f Position = MouseEvent.GetScreenSpacePosition() - MyGeometry.GetAbsolutePosition();
	IO.AddMousePosEvent(Position.X, Position.Y);
	const FKey Button = MouseEvent.GetEffectingButton();
	if (Button == EKeys::LeftMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
	}
	else if (Button == EKeys::RightMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Right, true);
	}
	else if (Button == EKeys::MiddleMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Middle, true);
	}
	return IO.WantCaptureMouse ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ImGui::FScopedContext ScopedContext(Context);
	ImGuiIO& IO = ImGui::GetIO();
	const FVector2f Position = MouseEvent.GetScreenSpacePosition() - MyGeometry.GetAbsolutePosition();
	IO.AddMousePosEvent(Position.X, Position.Y);
	const FKey Button = MouseEvent.GetEffectingButton();
	if (Button == EKeys::LeftMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
	}
	else if (Button == EKeys::RightMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Right, false);
	}
	else if (Button == EKeys::MiddleMouseButton)
	{
		IO.AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
	}
	return IO.WantCaptureMouse ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Treat as mouse down, ImGui handles double click internally
	return OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SImGuiPanel::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ImGui::FScopedContext ScopedContext(Context);
	ImGuiIO& IO = ImGui::GetIO();
	IO.AddMouseWheelEvent(0.0f, MouseEvent.GetWheelDelta());
	return IO.WantCaptureMouse ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent)
{
	ImGui::FScopedContext ScopedContext(Context);

	ImGuiIO& IO = ImGui::GetIO();
	const float Value = InAnalogInputEvent.GetAnalogValue();
	IO.AddKeyAnalogEvent(UnrealImGui::ConvertKey(InAnalogInputEvent.GetKey()), FMath::Abs(Value) > 0.1f, Value);
	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& Event)
{
	ImGui::FScopedContext ScopedContext{ Context };

	ImGuiIO& IO = ImGui::GetIO();
	IO.AddInputCharacter(CharCast<ANSICHAR>(Event.GetCharacter()));
	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	ImGui::FScopedContext ScopedContext(Context);

	ImGuiIO& IO = ImGui::GetIO();
	IO.AddKeyEvent(UnrealImGui::ConvertKey(InKeyEvent.GetKey()), true);
	const FModifierKeysState& ModifierKeys = InKeyEvent.GetModifierKeys();
	IO.AddKeyEvent(ImGuiMod_Ctrl, ModifierKeys.IsControlDown());
	IO.AddKeyEvent(ImGuiMod_Shift, ModifierKeys.IsShiftDown());
	IO.AddKeyEvent(ImGuiMod_Alt, ModifierKeys.IsAltDown());
	IO.AddKeyEvent(ImGuiMod_Super, ModifierKeys.IsCommandDown());
	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	ImGui::FScopedContext ScopedContext(Context);

	ImGuiIO& IO = ImGui::GetIO();
	IO.AddKeyEvent(UnrealImGui::ConvertKey(InKeyEvent.GetKey()), false);
	const FModifierKeysState& ModifierKeys = InKeyEvent.GetModifierKeys();
	IO.AddKeyEvent(ImGuiMod_Ctrl, ModifierKeys.IsControlDown());
	IO.AddKeyEvent(ImGuiMod_Shift, ModifierKeys.IsShiftDown());
	IO.AddKeyEvent(ImGuiMod_Alt, ModifierKeys.IsAltDown());
	IO.AddKeyEvent(ImGuiMod_Super, ModifierKeys.IsCommandDown());
	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiPanel::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	ImGui::FScopedContext ScopedContext(Context);
	ImGuiIO& IO = ImGui::GetIO();
	IO.AddFocusEvent(true);

	return Super::OnFocusReceived(MyGeometry, InFocusEvent);
}

void SImGuiPanel::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	ImGui::FScopedContext ScopedContext(Context);
	ImGuiIO& IO = ImGui::GetIO();
	IO.AddFocusEvent(false);
	ImGui::SetWindowFocus(nullptr);
	DisableVirtualInput();
}

FCursorReply SImGuiPanel::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	ImGui::FScopedContext ScopedContext(Context);
	const ImGuiIO& IO = ImGui::GetIO();
	if (IO.WantCaptureMouse && !(IO.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
	{
		const ImGuiMouseCursor CursorType = ImGui::GetMouseCursor();

		if (IO.MouseDrawCursor || CursorType == ImGuiMouseCursor_None)
		{
			return FCursorReply::Cursor(EMouseCursor::None);
		}
		else if (CursorType == ImGuiMouseCursor_Arrow)
		{
			return FCursorReply::Cursor(EMouseCursor::Default);
		}
		else if (CursorType == ImGuiMouseCursor_TextInput)
		{
			return FCursorReply::Cursor(EMouseCursor::TextEditBeam);
		}
		else if (CursorType == ImGuiMouseCursor_ResizeAll)
		{
			return FCursorReply::Cursor(EMouseCursor::CardinalCross);
		}
		else if (CursorType == ImGuiMouseCursor_ResizeNS)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeUpDown);
		}
		else if (CursorType == ImGuiMouseCursor_ResizeEW)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
		}
		else if (CursorType == ImGuiMouseCursor_ResizeNESW)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeSouthWest);
		}
		else if (CursorType == ImGuiMouseCursor_ResizeNWSE)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeSouthEast);
		}
		else if (CursorType == ImGuiMouseCursor_Hand)
		{
			return FCursorReply::Cursor(EMouseCursor::Hand);
		}
		else if (CursorType == ImGuiMouseCursor_NotAllowed)
		{
			return FCursorReply::Cursor(EMouseCursor::SlashedCircle);
		}
	}
	return Super::OnCursorQuery(MyGeometry, CursorEvent);
}

void SImGuiPanel::SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType)
{
	ImGuiIO& IO = Context->IO;
	ImGuiInputTextState& InputTextState = Context->InputTextState;
	InputTextState.ClearText();
	IO.AddInputCharactersUTF8(TCHAR_TO_UTF8(*InNewText.ToString()));
}

void SImGuiPanel::SetSelectionFromVirtualKeyboard(int InSelStart, int SelEnd)
{
	ImGuiInputTextState& InputTextState = Context->InputTextState;
	InputTextState.Stb.select_start = InSelStart;
	InputTextState.Stb.select_end = SelEnd;
	InputTextState.Stb.cursor = SelEnd;
	InputTextState.CursorClamp();
}

FText SImGuiPanel::GetText() const
{
	const ImGuiInputTextState& InputTextState = Context->InputTextState;
	TArray<ImWchar> WCharArray{ InputTextState.TextW.Data, InputTextState.TextW.size() };
	WCharArray.Add(0);
	const FString InputText{ WCharArray.GetData() };
	return FText::FromString(InputText);
}

bool SImGuiPanel::GetSelection(int& OutSelStart, int& OutSelEnd)
{
	const ImGuiInputTextState& InputTextState = Context->InputTextState;
	OutSelStart = InputTextState.GetSelectionStart();
	OutSelEnd = InputTextState.GetSelectionEnd();
	return true;
}

FText SImGuiPanel::GetHintText() const
{
	return FText::GetEmpty();
}

EKeyboardType SImGuiPanel::GetVirtualKeyboardType() const
{
	const ImGuiInputTextState& InputTextState = Context->InputTextState;
	if (InputTextState.Flags & ImGuiInputTextFlags_CharsDecimal)
	{
		return Keyboard_Number;
	}
	if (InputTextState.Flags & ImGuiInputTextFlags_Password)
	{
		return Keyboard_Password;
	}
	return Keyboard_Default;
}

FVirtualKeyboardOptions SImGuiPanel::GetVirtualKeyboardOptions() const
{
	return {};
}

bool SImGuiPanel::IsMultilineEntry() const
{
	const ImGuiInputTextState& InputTextState = Context->InputTextState;
	return InputTextState.Flags & ImGuiInputTextFlags_Multiline;
}

bool SImGuiPanel::IsComposing()
{
	return VirtualInput.bIsComposing;
}

bool SImGuiPanel::IsReadOnly()
{
	return !IsEnabled();
}

uint32 SImGuiPanel::GetTextLength()
{
	return VirtualInput.CompositionString.Len();
}

void SImGuiPanel::GetSelectionRange(uint32& OutBeginIndex, uint32& OutLength, ECaretPosition& OutCaretPosition)
{
	OutBeginIndex = VirtualInput.SelectionRangeBeginIndex;
	OutLength = VirtualInput.SelectionRangeLength;
	OutCaretPosition = VirtualInput.SelectionCaretPosition;
}

void SImGuiPanel::SetSelectionRange(const uint32 InBeginIndex, const uint32 InLength, const ECaretPosition InCaretPosition)
{
	VirtualInput.SelectionRangeBeginIndex = InBeginIndex;
	VirtualInput.SelectionRangeLength = InLength;
	VirtualInput.SelectionCaretPosition = InCaretPosition;
}

void SImGuiPanel::GetTextInRange(const uint32 InBeginIndex, const uint32 InLength, FString& OutString)
{
	OutString = VirtualInput.CompositionString.Mid(InBeginIndex, InLength);
}

void SImGuiPanel::SetTextInRange(const uint32 InBeginIndex, const uint32 InLength, const FString& InString)
{
	auto& CompositionString = VirtualInput.CompositionString;
	FString NewString;
	if (InBeginIndex > 0)
	{
		NewString = CompositionString.Mid(0, InBeginIndex);
	}

	NewString += InString;

	if ((int32)(InBeginIndex + InLength) < CompositionString.Len())
	{
		NewString += CompositionString.Mid(InBeginIndex + InLength, CompositionString.Len() - (InBeginIndex + InLength));
	}
	CompositionString = NewString;
}

int32 SImGuiPanel::GetCharacterIndexFromPoint(const FVector2D& InPoint)
{
	return INDEX_NONE;
}

bool SImGuiPanel::GetTextBounds(const uint32 InBeginIndex, const uint32 InLength, FVector2D& OutPosition, FVector2D& OutSize)
{
	// Let the IME editor follow the cursor
	const auto& CachedGeometry = GetTickSpaceGeometry();
	OutPosition = FVector2D(CachedGeometry.AbsolutePosition.X + Context->PlatformImeData.InputPos.x, CachedGeometry.AbsolutePosition.Y + Context->PlatformImeData.InputPos.y + 20);
	return true;
}

void SImGuiPanel::GetScreenBounds(FVector2D& OutPosition, FVector2D& OutSize)
{
	const auto& CachedGeometry = GetTickSpaceGeometry();
	OutPosition = CachedGeometry.GetAccumulatedRenderTransform().GetTranslation();
	OutSize = TransformVector(CachedGeometry.GetAccumulatedRenderTransform(), CachedGeometry.GetLocalSize());
}

TSharedPtr<FGenericWindow> SImGuiPanel::GetWindow()
{
	const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
	if (Window == nullptr)
	{
		return nullptr;
	}
	return Window->GetNativeWindow();
}

void SImGuiPanel::BeginComposition()
{
	VirtualInput.bIsComposing = true;
}

void SImGuiPanel::UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength)
{

}

void SImGuiPanel::EndComposition()
{
	VirtualInput.bIsComposing = false;

	ImGuiIO& IO = Context->IO;
	IO.AddInputCharactersUTF8(TCHAR_TO_UTF8(*VirtualInput.CompositionString));
	VirtualInput.CompositionString.Reset();
	VirtualInput.SelectionRangeBeginIndex = 0;
	VirtualInput.SelectionRangeLength = 0;
}

void SImGuiPanel::EnableVirtualInput()
{
	if (VirtualInput.bEnable)
	{
		return;
	}
	VirtualInput.bEnable = true;
	if (FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		FSlateApplication::Get().ShowVirtualKeyboard(true, 0, SharedThis(this));
	}
	else
	{
		if (ITextInputMethodSystem* TextInputMethodSystem = FSlateApplication::Get().GetTextInputMethodSystem())
		{
			VirtualInput.TextInputMethodChangeNotifier = TextInputMethodSystem->RegisterContext(SharedThis(this));
			if (VirtualInput.TextInputMethodChangeNotifier)
			{
				VirtualInput.TextInputMethodChangeNotifier->NotifyLayoutChanged(ITextInputMethodChangeNotifier::ELayoutChangeType::Created);
				TextInputMethodSystem->ActivateContext(SharedThis(this));
			}
		}
	}
}

void SImGuiPanel::DisableVirtualInput()
{
	if (VirtualInput.bEnable == false)
	{
		return;
	}

	VirtualInput.bEnable = false;
	if (FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		FSlateApplication::Get().ShowVirtualKeyboard(false, 0, SharedThis(this));
	}
	else
	{
		if (ITextInputMethodSystem* TextInputMethodSystem = FSlateApplication::Get().GetTextInputMethodSystem())
		{
			TextInputMethodSystem->DeactivateContext(SharedThis(this));
			TextInputMethodSystem->UnregisterContext(SharedThis(this));
		}
		VirtualInput.TextInputMethodChangeNotifier.Reset();
	}
}
