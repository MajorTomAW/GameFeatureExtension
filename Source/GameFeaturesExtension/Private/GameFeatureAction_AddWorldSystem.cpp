// Copyright © 2025 MajorT. All Rights Reserved.


#include "GameFeatureAction_AddWorldSystem.h"

#include "AssetRegistry/AssetBundleData.h"
#include "Containers/IndirectArray.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeaturesSubsystemSettings.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Misc/AssertionMacros.h"
#include "Templates/ChooseClass.h"
#include "Templates/Tuple.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

struct FGameFeatureDeactivatingContext;

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////
/// UGameFeatureWorldSystem

void UGameFeatureWorldSystem::NativeInitialize(
	const FWorldContext& WorldContext, const FGameFeatureActivatingContext& FeatureContext)
{
	Initialize(WorldContext.World());
}

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWorldSystem

void UGameFeatureAction_AddWorldSystem::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	Reset();
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWorldSystem::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsInitialized())
	{
		for (const FGameFeatureWorldSystemEntry& Entry : WorldSystemsList)
		{
			if (Entry.SystemType)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, FTopLevelAssetPath(*Entry.SystemType));
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, FTopLevelAssetPath(*Entry.SystemType));
			}
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWorldSystem::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FGameFeatureWorldSystemEntry& Entry : WorldSystemsList)
	{
		if (Entry.SystemType == nullptr)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullWorldSystemType", "Null SystemType at index {0} in WorldSystemsList."), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif

UGameFeatureWorldSystem* UGameFeatureAction_AddWorldSystem::FindGameFeatureWorldSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType, UObject* WorldContextObject)
{
	UGameFeatureWorldSystem* WorldSystemInst = nullptr;
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameFeatureWorldSystemManager* SystemManager = World->GetSubsystem<UGameFeatureWorldSystemManager>())
			{
				for (auto& WorldSystem : SystemManager->SystemInstances)
				{
					if (WorldSystem.Key && WorldSystem.Key->GetClass() == SystemType)
					{
						WorldSystemInst = WorldSystem.Key;
						break;
					}
				}
			}
		}
	}
	return WorldSystemInst;
}

void UGameFeatureAction_AddWorldSystem::OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();

	if ((World != nullptr) && World->IsGameWorld())
	{
		UGameFeatureWorldSystemManager* SystemManager = World->GetSubsystem<UGameFeatureWorldSystemManager>();
		if (ensure(SystemManager))
		{
			for (const FGameFeatureWorldSystemEntry& Entry : WorldSystemsList)
			{
				if (!Entry.TargetWorld.IsNull())
				{
					UWorld* TargetWorld = Entry.TargetWorld.Get();
					if (TargetWorld != World)
					{
						// This system is intended for a specific world (not this one)
						continue;
					}
				}

				if (Entry.SystemType)
				{
					SystemManager->RequestSystemOfType(Entry.SystemType);
				}
			}
		}
	}
}

void UGameFeatureAction_AddWorldSystem::Reset()
{
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (World)
		{
			if (UGameFeatureWorldSystemManager* SystemManager = World->GetSubsystem<UGameFeatureWorldSystemManager>())
			{
				for (const FGameFeatureWorldSystemEntry& Entry : WorldSystemsList)
				{
					if (!Entry.TargetWorld.IsNull())
					{
						UWorld* TargetWorld = Entry.TargetWorld.Get();
						if (TargetWorld != World)
						{
							// This system is intended for a specific world (not this one)
							continue;
						}
					}

					if (Entry.SystemType)
					{
						SystemManager->ReleaseRequestForSystemOfType(Entry.SystemType);
					}
				}

			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// UGameFeatureWorldSystemManager

void UGameFeatureWorldSystemManager::PostInitialize()
{
	for (auto& PreExistingInstance : SystemInstances)
	{
		PreExistingInstance.Key->Initialize(GetWorld());
	}

	bIsInitialized = true;
}

bool UGameFeatureWorldSystemManager::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UGameFeatureWorldSystem* UGameFeatureWorldSystemManager::RequestSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType)
{
	for (auto& PreExistingInstance : SystemInstances)
	{
		if (PreExistingInstance.Key->GetClass() == SystemType)
		{
			PreExistingInstance.Value += 1;
			return PreExistingInstance.Key;
		}
	}

	UGameFeatureWorldSystem* NewWorldSystem = NewObject<UGameFeatureWorldSystem>(GetWorld(), SystemType);
	SystemInstances.Add(NewWorldSystem, 1);

	if (bIsInitialized)
	{
		NewWorldSystem->Initialize(GetWorld());
	}

	return NewWorldSystem;
}

void UGameFeatureWorldSystemManager::ReleaseRequestForSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType)
{
	for (auto& PreExistingInstance : SystemInstances)
	{
		if (PreExistingInstance.Key->GetClass() == SystemType)
		{
			PreExistingInstance.Value -= 1;

			if (PreExistingInstance.Value <= 0)
			{
				SystemInstances.Remove(PreExistingInstance.Key);
			}
			break;
		}
	}
}

#undef LOCTEXT_NAMESPACE