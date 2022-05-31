// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiWS_Replay.h"

#include "font_awesome_5.h"
#include "imgui.h"

namespace ImGuiWS_Record
{
FImGuiWS_Replay::FImGuiWS_Replay(const char* FilePath)
{
	LoadedSession.load(FilePath);
}

void FImGuiWS_Replay::Draw(float DeltaTime, bool& CloseReplay)
{
	if (PlayState == EPlayState::Play)
	{
		FrameIndex = (FrameIndex + 1) % LoadedSession.nFrames();
	}
	
	const ImVec2 MousePos = ImGui::GetMousePos();
	
	constexpr float Height = 80.f;
	constexpr float VPadding = 40.f;
	constexpr float HPadding = 40.f;
	const ImGuiIO& IO = ImGui::GetIO();
	const bool bShowWindow = MousePos.y > IO.DisplaySize.y - Height - VPadding;
	if (bShowWindow == false && WindowHeightOffset == Height + VPadding)
	{
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f,  100.f / 255.f));

	WindowHeightOffset = FMath::FInterpTo(WindowHeightOffset, bShowWindow ? 0.f : Height + VPadding, DeltaTime, 20.f);
	const float WindowWidth = IO.DisplaySize.x - HPadding * 2.f;
	ImGui::SetNextWindowPos({ HPadding, IO.DisplaySize.y - Height - VPadding + WindowHeightOffset });
	ImGui::SetNextWindowSize({ WindowWidth, Height });
	ImGui::Begin("ReplayControl", nullptr, ImGuiWindowFlags_NoDecoration);

	ImGui::SetCursorPos({ 20.f, 10.f });
	ImGui::Text("ImGui-WS Replay");
	ImGui::SameLine(WindowWidth - 60.f);
	if (ImGui::Button("Quit"))
	{
		CloseReplay = true;
	}
	
	ImGui::SetCursorPos({ 30.f, 40.f });
	switch (PlayState)
	{
	case EPlayState::Pause:
		if (ImGui::Button(ICON_FA_PLAY))
		{
			PlayState = EPlayState::Play;
		}
		break;
	case EPlayState::Play:
		if (ImGui::Button(ICON_FA_PAUSE))
		{
			PlayState = EPlayState::Pause;
		}
		break;
	default: ;
	}
	ImGui::SameLine();

	ImGui::SetNextItemWidth(-30.f);
	ImGui::SliderInt("##PlayPosition", &FrameIndex, 0, LoadedSession.nFrames() - 1);

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

bool FImGuiWS_Replay::GetDrawData(FDrawData& DrawData)
{
	return LoadedSession.getFrame(FrameIndex, &DrawData.drawData, DrawData.drawLists, ImGui::GetDrawListSharedData());
}
}
