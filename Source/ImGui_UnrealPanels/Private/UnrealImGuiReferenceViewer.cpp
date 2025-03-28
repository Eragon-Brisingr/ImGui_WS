// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealImGuiReferenceViewer.h"

#include "ImGuiEx.h"
#include "UnrealImGuiAssetPicker.h"
#include "UnrealImGuiPropertyDetails.h"
#include "UObject/ReferenceChainSearch.h"

namespace ReferenceChainSearch
{
	void DumpChain(FReferenceChainSearch::FReferenceChain* Chain,
		TFunctionRef<bool(FReferenceChainSearch::FCallbackParams& Params)> ReferenceCallback,
		TMap<uint64, FString>& CallstackCache,
		FOutputDevice& Out,
		ELogVerbosity::Type InVerbosityForPrint)
	{
		if (Chain->Num())
		{
			bool bPostCallbackContinue = true;
			const int32 RootIndex = Chain->Num() - 1;
			const FReferenceChainSearch::FNodeReferenceInfo* ReferenceInfo = Chain->GetReferenceInfo(RootIndex);
			FGCObjectInfo* ReferencerObject = Chain->GetNode(RootIndex)->ObjectInfo;
			{
				FReferenceChainSearch::FCallbackParams Params;
				Params.Referencer = nullptr;
				Params.Object = ReferencerObject;
				Params.ReferenceInfo = nullptr;
				Params.Indent = FMath::Min<int32>(TCStringSpcHelper<TCHAR>::MAX_SPACES, Chain->Num() - RootIndex);
				Params.Out = &Out;

				Out.Logf(InVerbosityForPrint, TEXT("%s %s"),
					FCString::Spc(Params.Indent),
					*ReferencerObject->GetFullName());

				bPostCallbackContinue = ReferenceCallback(Params);
			}

			// Roots are at the end so iterate from the last to the first node
			for (int32 NodeIndex = RootIndex - 1; NodeIndex >= 0 && bPostCallbackContinue; --NodeIndex)
			{
				FGCObjectInfo* Object = Chain->GetNode(NodeIndex)->ObjectInfo;

				FReferenceChainSearch::FCallbackParams Params;
				Params.Referencer = ReferencerObject;
				Params.Object = Object;
				Params.ReferenceInfo = ReferenceInfo;
				Params.Indent = FMath::Min<int32>(TCStringSpcHelper<TCHAR>::MAX_SPACES, Chain->Num() - NodeIndex - 1);
				Params.Out = &Out;

				if (ReferenceInfo && ReferenceInfo->Type == FReferenceChainSearch::EReferenceType::Property)
				{
					FString ReferencingPropertyName;
					UClass* ReferencerClass = Cast<UClass>(ReferencerObject->GetClass()->TryResolveObject());
					TArray<FProperty*> ReferencingProperties;

					if (ReferencerClass && UE::GC::FPropertyStack::ConvertPathToProperties(ReferencerClass, ReferenceInfo->ReferencerName, ReferencingProperties))
					{
						FProperty* InnermostProperty = ReferencingProperties.Last();
						FProperty* OutermostProperty = ReferencingProperties[0];

						ReferencingPropertyName = FString::Printf(TEXT("%s %s%s::%s"),
							*InnermostProperty->GetCPPType(),
							OutermostProperty->GetOwnerClass()->GetPrefixCPP(),
							*OutermostProperty->GetOwnerClass()->GetName(),
							*ReferenceInfo->ReferencerName.ToString());
					}
					else
					{
						// Handle base UObject referencer info (it's only exposed to the GC token stream and not to the reflection system)
						static const FName ClassPropertyName(TEXT("Class"));
						static const FName OuterPropertyName(TEXT("Outer"));

						FString ClassName;
						if (ReferenceInfo->ReferencerName == ClassPropertyName || ReferenceInfo->ReferencerName == OuterPropertyName)
						{
							ClassName = TEXT("UObject");
						}
						else if (ReferencerClass)
						{
							// Use the native class name when possible
							ClassName = ReferencerClass->GetPrefixCPP();
							ClassName += ReferencerClass->GetName();
						}
						else
						{
							// Revert to the internal class name if not
							ClassName = ReferencerObject->GetClassName();
						}
						ReferencingPropertyName = FString::Printf(TEXT("UObject* %s::%s"), *ClassName, *ReferenceInfo->ReferencerName.ToString());
					}

					Out.Logf(InVerbosityForPrint, TEXT("%s-> %s = %s"),
						FCString::Spc(Params.Indent),
						*ReferencingPropertyName,
						*Object->GetFullName());
				}
				else if (ReferenceInfo && ReferenceInfo->Type == FReferenceChainSearch::EReferenceType::AddReferencedObjects)
				{
					FString UObjectOrGCObjectName;
					if (ReferenceInfo->ReferencerName.IsNone())
					{
						UClass* ReferencerClass = Cast<UClass>(ReferencerObject->GetClass()->TryResolveObject());
						if (ReferencerClass)
						{
							UObjectOrGCObjectName = ReferencerClass->GetPrefixCPP();
							UObjectOrGCObjectName += ReferencerClass->GetName();
						}
						else
						{
							UObjectOrGCObjectName += ReferencerObject->GetClassName();
						}
					}
					else
					{
						UObjectOrGCObjectName = ReferenceInfo->ReferencerName.ToString();
					}

					Out.Logf(InVerbosityForPrint, TEXT("%s-> %s::AddReferencedObjects(%s)"),
						FCString::Spc(Params.Indent),
						*UObjectOrGCObjectName,
						*Object->GetFullName());
				}
				else if (ReferenceInfo && ReferenceInfo->Type == FReferenceChainSearch::EReferenceType::OuterChain)
				{
					Out.Logf(InVerbosityForPrint, TEXT("%s-> %s = %s"),
						FCString::Spc(Params.Indent),
						TEXT("Outer Chain"),
						*Object->GetFullName());
				}
				else
				{
					Out.Logf(InVerbosityForPrint, TEXT("%s-> %s = %s"),
						FCString::Spc(Params.Indent),
						TEXT("UNKNOWN"),
						*Object->GetFullName());
				}

				bPostCallbackContinue = ReferenceCallback(Params);

				ReferencerObject = Object;
				ReferenceInfo = Chain->GetReferenceInfo(NodeIndex);
			}
			Out.Logf(InVerbosityForPrint, TEXT("  "));
		}
	}
}

namespace DependencySearch
{
	struct FDependencySearch : FArchiveUObject
	{
		FDependencySearch(bool bIgnoreSelfPackage, const TSet<UPackage*>& SelfPackages)
			: bIgnoreSelfPackage(bIgnoreSelfPackage)
			, SelfPackages(SelfPackages)
		{
			ArShouldSkipBulkData = true;
			ArNoDelta = false;
			ArPortFlags = PPF_None;
			ArIsObjectReferenceCollector = true;
		}

		FArchive& operator<<(UObject*& Value) override
		{
			if (Value == nullptr)
			{
				return *this;
			}
			
			bool bIsAlreadyInSet;
			VisitedObjects.Add(Value, &bIsAlreadyInSet);
			if (bIsAlreadyInSet)
			{
				return *this;
			}
			
			if (SelfPackages.Contains(Value->GetPackage()))
			{
				Value->Serialize(*this);
				if (!bIgnoreSelfPackage)
				{
					Dependencies.Add(Value);
				}
			}
			else
			{
				Dependencies.Add(Value);
			}

			return *this;
		}

		bool bIgnoreSelfPackage;
		const TSet<UPackage*>& SelfPackages;
		TSet<UObject*> VisitedObjects;

		TArray<UObject*> Dependencies;
	};
}

UUnrealImGuiReferenceViewer::UUnrealImGuiReferenceViewer()
{
	Title = TEXT("Reference Viewer");
	Categories = { TEXT("Tools") };
}

void UUnrealImGuiReferenceViewer::Draw(UObject* Owner, UUnrealImGuiPanelBuilder* Builder, float DeltaSeconds)
{
	Super::Draw(Owner, Builder, DeltaSeconds);

	const auto Width = ImGui::GetContentRegionAvail().x / 3;
	if (auto Child = ImGui::FChildWindow{ "References", ImVec2{ Width, 0.f }, ImGuiChildFlags_Borders })
	{
		ImGui::SeparatorText("References");

		for (const auto& Reference : References)
		{
			if (!Reference.Asset.IsValid())
			{
				continue;
			}
			if (ImGui::Button(TCHAR_TO_UTF8(*Reference.Asset->GetName()), { -1.f, 0.f }))
			{
				Objects = { Reference.Asset };
				Refresh();
				break;
			}
			if (ImGui::BeginItemTooltip())
			{
				for (const auto& Path : Reference.ReferencePaths)
				{
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*Path));
				}
				ImGui::EndTooltip();
			}
		}
	}
	ImGui::SameLine();
	if (auto Child = ImGui::FChildWindow{ "Objects", ImVec2{ Width, 0.f }, ImGuiChildFlags_Borders })
	{
		ImGui::SeparatorText("Object/Objects");

		if (ImGui::Checkbox("Ignore Self Package", &bIgnoreSelfPackage))
		{
			Refresh();
		}

		if (ImGui::BeginTable("ObjectsTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.7f);
			
			const TGuardValue GObjectPickerSettingsGuard{ UnrealImGui::GObjectPickerSettings, UnrealImGui::FObjectPickerSettings{ .bShowNonAssetRegistry = true } };
			const TGuardValue GEnableEditVisiblePropertyGuard{UnrealImGui::GlobalValue::GEnableEditVisibleProperty, true };

			static auto ObjectProperty = StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(ThisClass, Objects));
			UnrealImGui::DrawUnrealProperty(ObjectProperty, UnrealImGui::FPtrArray{ (uint8*)this }, ObjectProperty->GetOffset_ForInternal());

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(1);

			ImGui::Separator();
			if (ImGui::Button("Refresh"))
			{
				Refresh();
			}
			
			ImGui::EndTable();
		}
	}
	ImGui::SameLine();
	if (auto Child = ImGui::FChildWindow{ "Dependencies", ImVec2{ -1.f, 0.f }, ImGuiChildFlags_Borders })
	{
		ImGui::SeparatorText("Dependencies");

		for (const auto& Dependency : Dependencies)
		{
			if (!Dependency.Asset.IsValid())
			{
				continue;
			}
			if (ImGui::Button(TCHAR_TO_UTF8(*Dependency.Asset->GetName()), { -1.f, 0.f }))
			{
				Objects = { Dependency.Asset };
				Refresh();
				break;
			}
			if (ImGui::BeginItemTooltip())
			{
				for (const auto& Path : Dependency.ReferencePaths)
				{
					ImGui::TextUnformatted(TCHAR_TO_UTF8(*Path));
				}
				ImGui::EndTooltip();
			}
		}
	}
}

void UUnrealImGuiReferenceViewer::Refresh()
{
	References.Reset();
	Dependencies.Reset();
				
	TArray<UObject*> ToFindObjects;
	TSet<UPackage*> SelfPackages;
	ToFindObjects.Reserve(Objects.Num());
	for (auto& ObjPtr : Objects)
	{
		if (auto Obj = ObjPtr.Get())
		{
			ToFindObjects.Add(Obj);
			SelfPackages.Add(Obj->GetPackage());
		}
	}
	if (ToFindObjects.Num() == 0)
	{
		return;
	}
	
	FReferenceChainSearch ReferenceChainSearch{ ToFindObjects, EReferenceChainSearchMode::Shortest };
	for (auto ReferenceChain : ReferenceChainSearch.GetReferenceChains())
	{
		if (auto RootObject = ReferenceChain->GetRootNode()->ObjectInfo->TryResolveObject())
		{
			if (RootObject->GetClass()->GetName() == TEXT("GCObjectReferencer"))
			{
				continue;
			}
			if (bIgnoreSelfPackage && SelfPackages.Contains(RootObject->GetPackage()))
			{
				continue;
			}
			int32 Idx = References.IndexOfByPredicate([&](const FReferencer& E){ return E.Asset == RootObject; });
			if (Idx == INDEX_NONE)
			{
				Idx = References.Add({ RootObject });
			}
			FStringOutputDevice OutString;
			OutString.SetAutoEmitLineTerminator(true);
			TMap<uint64, FString> CallstackCache;
			ReferenceChainSearch::DumpChain(ReferenceChain, [](FReferenceChainSearch::FCallbackParams&){ return true; }, CallstackCache, OutString, ELogVerbosity::Log);
			References[Idx].ReferencePaths.Add(MoveTemp(OutString));
		}
	}
	References.Sort([](const FReferencer& LHS, const FReferencer& RHS)
	{
		return LHS.Asset->GetName() < RHS.Asset->GetName();
	});

	DependencySearch::FDependencySearch DependencySearch{ bIgnoreSelfPackage, SelfPackages };
	for (auto Obj : ToFindObjects)
	{
		DependencySearch << Obj;
	}
	for (UObject* DepObj : DependencySearch.Dependencies)
	{
		Dependencies.Add({ DepObj, { DepObj->GetPathName() } });
	}
	Dependencies.Sort([](const FDependency& LHS, const FDependency& RHS)
	{
		return LHS.Asset->GetName() < RHS.Asset->GetName();
	});
}
