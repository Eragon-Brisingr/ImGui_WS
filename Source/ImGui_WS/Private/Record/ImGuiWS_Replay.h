// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "imgui-ws-record.h"

namespace ImGuiWS_Record
{
class FImGuiWS_Replay
{
public:
	FImGuiWS_Replay(const char* FilePath);

	void Draw(float DeltaTime, bool& CloseReplay);

	struct FDrawData
	{
		ImDrawData drawData;
		ImDrawData* operator->()
		{
			return &drawData;
		}
	private:
		friend class FImGuiWS_Replay;
		std::vector<ImDrawList> drawLists;
	};
	bool GetDrawData(FDrawData& DrawData);
private:
	Session LoadedSession;
	float WindowHeightOffset = 0.f;

	enum class EPlayState : uint8
	{
		Pause,
		Play,
	};
	EPlayState PlayState = EPlayState::Play;
	int32 FrameIndex = 0;
};
}
