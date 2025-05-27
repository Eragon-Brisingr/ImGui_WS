// Fill out your copyright notice in the Description page of Project Settings.


#include "ImGuiEx.h"

namespace ImGui
{
	struct InputTextCallback_UserData
	{
	    FUtf8String&            Str;
	    ImGuiInputTextCallback  ChainCallback;
	    void*                   ChainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
	    InputTextCallback_UserData* user_data = static_cast<InputTextCallback_UserData*>(data->UserData);
	    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	    {
	        // Resize string callback
	        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
	        FUtf8String& str = user_data->Str;
	        IM_ASSERT(data->Buf == (char*)*str);
			auto& CharArray = str.GetCharArray();
	        CharArray.SetNum(data->BufTextLen + 1);
	        data->Buf = reinterpret_cast<char*>(CharArray.GetData());
	    }
	    else if (user_data->ChainCallback)
	    {
	        // Forward to user callback, if any
	        data->UserData = user_data->ChainCallbackUserData;
	        return user_data->ChainCallback(data);
	    }
	    return 0;
	}

	bool InputText(const char* label, FUtf8String& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

	    InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputText(label, (char*)*str, str.GetCharArray().Max(), flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextMultiline(const char* label, FUtf8String& str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputTextMultiline(label, (char*)*str, str.GetCharArray().Max(), size, flags, InputTextCallback, &cb_user_data);
	}

	bool InputTextWithHint(const char* label, const char* hint, FUtf8String& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
	    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	    flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data{ str, callback, user_data };
	    return ImGui::InputTextWithHint(label, hint, (char*)*str, str.GetCharArray().Max(), flags, InputTextCallback, &cb_user_data);
	}
}
