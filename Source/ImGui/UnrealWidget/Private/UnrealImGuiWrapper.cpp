// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiWrapper.h"

#include "UnrealImGuiUtils.h"

namespace UnrealImGui
{
	struct InputTextCallback_UserData
	{
	    FUTF8String&            Str;
	    ImGuiInputTextCallback  ChainCallback;
	    void*                   ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
	    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	    {
	        // Resize string callback
	        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
	        FUTF8String& str = user_data->Str;
	        IM_ASSERT(data->Buf == str.GetData());
	        str.SetNum(data->BufTextLen + 1);
	        data->Buf = str.GetData();
	    }
	    else if (user_data->ChainCallback)
	    {
	        // Forward to user callback, if any
	        data->UserData = user_data->ChainCallbackUserData;
	        return user_data->ChainCallback(data);
	    }
	    return 0;
	}

	bool InputText(const char* label, FUTF8String& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

	    InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputText(label, str.GetData(), str.Max(), flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextMultiline(const char* label, FUTF8String& str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputTextMultiline(label, str.GetData(), str.Max(), size, flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextWithHint(const char* label, const char* hint, FUTF8String& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputTextWithHint(label, hint, str.GetData(), str.Max(), flags, InputTextCallback, &cb_user_data);
	}
}
