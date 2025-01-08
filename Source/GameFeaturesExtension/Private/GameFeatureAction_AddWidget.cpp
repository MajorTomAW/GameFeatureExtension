// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureAction_AddWidget.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/HUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddWidget)

#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddWidget::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWidget::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	for (const auto& Entry : Widgets)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.WidgetClass.ToSoftObjectPath().GetAssetPath());
	}
}
#endif
#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidget::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	{ // Validate Layouts
		int32 LayoutIndex = 0;
		for (const auto& Entry : Layouts)
		{
			if (Entry.LayoutClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutClassNull", "Layouts[{0}].LayoutClass must be set"), LayoutIndex));
			}

			if (!Entry.LayerTag.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayerTagInvalid", "Layouts[{0}].LayerTag must be set"), LayoutIndex));
			}

			++LayoutIndex;
		}
	}

	{ // Validate Widgets
		int32 WidgetIndex = 0;
		for (const auto& Entry : Widgets)
		{
			if (Entry.WidgetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("WidgetClassNull", "Widgets[{0}].WidgetClass must be set"), WidgetIndex));
			}

			if (!Entry.SlotTag.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("SlotTagInvalid", "Widgets[{0}].SlotTag must be set"), WidgetIndex));
			}

			++WidgetIndex;
		}
	}

	return Result;
}
#endif

void UGameFeatureAction_AddWidget::OnAddToWorld(
	const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = World->GetGameInstance();
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) &&
		(World != nullptr) &&
		(World->IsGameWorld()))
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			TSoftClassPtr<AActor> HUDClass = AHUD::StaticClass();
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler
			(
				HUDClass,
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext)
			);

			ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddWidget::Reset(FPerContextData& ActiveData)
{
	ActiveData.ComponentRequests.Empty();

	for (auto& Pair : ActiveData.ActorData)
	{
		for (FUIExtensionHandle& Handle : Pair.Value.ExtensionHandles)
		{
			Handle.Unregister();
		}
	}

	ActiveData.ActorData.Reset();
}

void UGameFeatureAction_AddWidget::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) ||
		(EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveWidgets(Actor, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) ||
		(EventName == UGameFrameworkComponentManager::NAME_GameActorReady))
	{
		AddWidgets(Actor, ActiveData);
	}
}

void UGameFeatureAction_AddWidget::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	const AHUD* HUD = CastChecked<AHUD>(Actor);

	if (!HUD->GetOwningPlayerController())
	{
		return;
	}

	if (ULocalPlayer* LP = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
	{
		FPerActorData& ActorData = ActiveData.ActorData.FindOrAdd(Actor);

		// Push Layers
		for (const auto& Entry : Layouts)
		{
			if (TSubclassOf<UCommonActivatableWidget> ConcreteWidgetClass = Entry.LayoutClass.Get())
			{
				ActorData.LayoutsAdded.Add(UCommonUIExtensions::PushContentToLayer_ForPlayer(LP, Entry.LayerTag, ConcreteWidgetClass));
			}
		}

		// Add Widgets
		UUIExtensionSubsystem* ExtensionSub = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
		for (const auto& Entry : Widgets)
		{
			ActorData.ExtensionHandles.Add(ExtensionSub->RegisterExtensionAsWidgetForContext(Entry.SlotTag, LP, Entry.WidgetClass.Get(), Entry.Priority));
		}
	}
}

void UGameFeatureAction_AddWidget::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	const AHUD* HUD = CastChecked<AHUD>(Actor);

	// Only unregister if this is the same HUD actor that was used to register, there could be multiple HUDs on one client
	FPerActorData* ActorData = ActiveData.ActorData.Find(HUD);

	if (ActorData == nullptr)
	{
		return;
	}

	for (TWeakObjectPtr<UCommonActivatableWidget>& Added : ActorData->LayoutsAdded)
	{
		if (Added.IsValid())
		{
			Added->DeactivateWidget();
		}
	}

	for (FUIExtensionHandle& Handle : ActorData->ExtensionHandles)
	{
		Handle.Unregister();
	}

	ActiveData.ActorData.Remove(HUD);
}

#undef LOCTEXT_NAMESPACE