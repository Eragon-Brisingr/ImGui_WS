// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImGui_Editor.h"

#include "Editor.h"
#include "ImGui_WS_Manager.h"
#include "Selection.h"
#include "ToolMenus.h"
#include "UnrealImGuiLibrary.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_EditorModule::StartupModule()
{
	if (GIsEditor == false || IsRunningCommandlet())
	{
		return;
	}

	const FMargin Padding{ 0.f, 0.f, 2.f, 0.f };
	static FButtonStyle ButtonStyle = FButtonStyle{ FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button") }
		.SetNormalPadding(Padding)
		.SetPressedPadding(Padding);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.StatusBar.ToolBar");
	FToolMenuSection& Section = Menu->AddSection(TEXT("ImGui"), LOCTEXT("ImGui", "ImGui"));
	Section.AddEntry(FToolMenuEntry::InitWidget(
		TEXT("LaunchUrlImGui_WS"),
		SNew(SButton)
		.ToolTipText(LOCTEXT("OpenImGui-WSTooltip", "Launch ImGui-WS Web page"))
		.ContentPadding(0.f)
		.ButtonStyle(&ButtonStyle)
		.OnClicked_Lambda([this]
		{
			if (UImGui_WS_Manager::GetChecked()->GetConnectionCount() > 0)
			{
				return FReply::Handled();
			}

			const UImGui_WS_Manager* Manager = UImGui_WS_Manager::GetChecked();
			FPlatformProcess::LaunchURL(*FString::Printf(TEXT("http://localhost:%d"), Manager->GetPort()), nullptr, nullptr);
			return FReply::Handled();
		})
		.Content()
		[
			SNew(SHorizontalBox)
			.Visibility(EVisibility::HitTestInvisible)
			+ SHorizontalBox::Slot()
			.Padding(4.f, 2.f, 0.f, 2.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]
				{
					if (UImGui_WS_Manager::GetChecked()->GetConnectionCount() > 0)
					{
						return ECheckBoxState::Checked;
					}
					return ECheckBoxState::Unchecked;
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2.f, 0.f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("IMGUI_Label", "IMGUI"))
			]
		], LOCTEXT("IMGUI_Label", "IMGUI")));

	static TWeakPtr<SNotificationItem> NotificationPtr;
	if (GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground)
	{
		const FProperty* PerformanceThrottlingProperty = FindFieldChecked<FProperty>(UEditorPerformanceSettings::StaticClass(), GET_MEMBER_NAME_CHECKED(UEditorPerformanceSettings, bThrottleCPUWhenNotForeground));
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("PropertyName"), PerformanceThrottlingProperty->GetDisplayNameText());
		FNotificationInfo Info(FText::Format(LOCTEXT("ImGui-WS PerformanceWarning", "ImGui-WS: The editor setting '{PropertyName}' is currently enabled. This will stop editor windows from updating in realtime while the editor is not in focus"), Arguments));

		// Add the buttons with text, tooltip and callback
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("ImGui-WS PerformanceWarningDisable", "Disable"),
			LOCTEXT("ImGui-WS PerformanceWarningDisableToolTip", "Disable ThrottleCPUWhenNotForeground"),
			FSimpleDelegate::CreateLambda([this]
			{
				UEditorPerformanceSettings* Settings = GetMutableDefault<UEditorPerformanceSettings>();
				Settings->bThrottleCPUWhenNotForeground = false;
				Settings->PostEditChange();
				Settings->SaveConfig();
				if (const TSharedPtr<SNotificationItem> Notification = NotificationPtr.Pin())
				{
					Notification->SetCompletionState(SNotificationItem::CS_None);
					Notification->ExpireAndFadeout();
				}
			}))
		);

		Info.bFireAndForget = false;
		Info.WidthOverride = 500.0f;
		Info.bUseLargeFont = false;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;

		// Launch notification
		const auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		Notification->SetCompletionState(SNotificationItem::CS_Pending);
		NotificationPtr = Notification;
	}

	for (TFieldIterator<UFunction> It{ UImGuiLibrary::StaticClass() }; It; ++It)
	{
		// Support script call internal function
		static const FName NAME_Function_ScriptCallable("ScriptCallable");
		It->SetMetaData(NAME_Function_ScriptCallable, TEXT(""));
	}
}

void FImGui_EditorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FImGui_EditorModule, ImGui_Editor)