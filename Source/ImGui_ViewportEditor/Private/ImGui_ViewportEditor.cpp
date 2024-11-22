#include "ImGui_ViewportEditor.h"

#include "ImGui_Viewport.h"
#include "LevelEditor.h"
#include "SImGuiPanel.h"
#include "SLevelViewport.h"
#include "Slate/SGameLayerManager.h"

#define LOCTEXT_NAMESPACE "ImGui_WS"

void FImGui_ViewportEditorModule::StartupModule()
{
	if (FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		LevelEditor->OnLevelEditorCreated().AddRaw(this, &FImGui_ViewportEditorModule::WhenLevelEditorCreated);
	}
}

void FImGui_ViewportEditorModule::ShutdownModule()
{
	if (FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
	{
		LevelEditor->OnLevelEditorCreated().RemoveAll(this);
	}
}

namespace PrivateAccess
{
    template<typename Tag, typename Tag::type M>
    struct Rob
    { 
       FORCEINLINE friend typename Tag::type Access(Tag)
       {
          return M;
       }
    };

#define ROB_MEMBER(Class, Member, MemberType) \
struct Class##Member##Rob \
{ \
typedef MemberType Class::*type; \
friend type Access(Class##Member##Rob); \
};\
template struct Rob<Class##Member##Rob, &Class::Member>; \
FORCEINLINE MemberType& Class##_##Member(Class& Inst) { return Inst.*Access(Class##Member##Rob()); }

    ROB_MEMBER(SLevelViewport, GameLayerManager, TSharedPtr<SGameLayerManager>);

#undef ROB_MEMBER
}

class SImGuiEditorViewportOverlay : public SImGuiViewportOverlay
{
	using Super = SImGuiViewportOverlay;
public:
	void Construct(const FArguments& Args, const TSharedRef<ILevelEditor>& LevelEditor)
	{
		WeakLevelEditor = LevelEditor;
		Super::Construct(Args);
	}
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		auto LevelEditor = WeakLevelEditor.Pin();
		if (LevelEditor == nullptr)
		{
			return;
		}
		auto World = LevelEditor->GetWorld();
		if (World == nullptr)
		{
			return;
		}
		if (World->LastRenderTime == WorldLastRenderTime)
		{
			return;
		}
		WorldLastRenderTime = World->LastRenderTime;
		Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}
private:
	TWeakPtr<ILevelEditor> WeakLevelEditor;
	double WorldLastRenderTime = -1.f;
};

void FImGui_ViewportEditorModule::WhenLevelEditorCreated(TSharedPtr<ILevelEditor> LevelEditor)
{
	if (LevelEditor == nullptr)
	{
		return;
	}
	auto ActiveViewport = LevelEditor->GetActiveViewportInterface();
	if (!ActiveViewport)
	{
		return;
	}
	auto GameLayerManager = PrivateAccess::SLevelViewport_GameLayerManager(*ActiveViewport);
	if (!GameLayerManager)
	{
		return;
	}

	auto Content = SNew(SImGuiEditorViewportOverlay, LevelEditor.ToSharedRef())
		.Visibility_Lambda([]
		{
			if (GEditor->IsPlayingSessionInEditor())
			{
				return EVisibility::Collapsed;
			}
			return ImGui::Viewport::GetVisibility();
		})
		.OnImGuiTick_Lambda([WeakLevelEditor = TWeakPtr<ILevelEditor>(LevelEditor)](float DeltaSeconds)
		{
			if (auto LevelEditor = WeakLevelEditor.Pin())
			{
				if (auto World = LevelEditor->GetWorld())
				{
					ImGui::Viewport::OnImGuiTick.Broadcast(World, DeltaSeconds);
				}
			}
		});

	GameLayerManager->AddGameLayer(Content, 1);
	FImGui_ViewportModule::EditorViewport = Content;
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FImGui_ViewportEditorModule, ImGui_ViewportEditor)