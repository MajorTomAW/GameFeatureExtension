// Copyright © 2024 Playton. All Rights Reserved.


#include "GameFeatureAction_AddSpawnedActors.h"

#include "AssetRegistry/AssetBundleData.h"
#include "CoreTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Templates/Casts.h"
#include "Templates/ChooseClass.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

struct FGameFeatureDeactivatingContext;

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWorldSystem

void UGameFeatureAction_AddSpawnedActors::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	Reset();
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddSpawnedActors::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsInitialized())
	{
		for (const FSpawningWorldActorsEntry& Entry : ActorsList)
		{
			for (const FSpawningActorEntry& ActorEntry : Entry.Actors)
			{
				AssetBundleData.AddBundleAssetTruncated(UGameFeaturesSubsystemSettings::LoadStateClient, ActorEntry.ActorType->GetPathName());
				AssetBundleData.AddBundleAssetTruncated(UGameFeaturesSubsystemSettings::LoadStateServer, ActorEntry.ActorType->GetPathName());
			}
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddSpawnedActors::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FSpawningWorldActorsEntry& Entry : ActorsList)
	{
		int32 ActorIndex = 0;
		for (const FSpawningActorEntry& ActorEntry : Entry.Actors)
		{
			if (ActorEntry.ActorType == nullptr)
			{
				Context.AddError(FText::Format(LOCTEXT("NullSpawnedActorType", "Null ActorType for actor #{0} at index {1} in ActorsList."), FText::AsNumber(ActorIndex), FText::AsNumber(EntryIndex)));
			}
			++ActorIndex;
		}
		++EntryIndex;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddSpawnedActors::OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();

	if ((World != nullptr) && World->IsGameWorld())
	{
		for (const FSpawningWorldActorsEntry& Entry : ActorsList)
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

			for (const FSpawningActorEntry& ActorEntry : Entry.Actors)
			{
				AActor* NewActor = World->SpawnActor<AActor>(ActorEntry.ActorType, ActorEntry.SpawnTransform);
				SpawnedActors.Add(NewActor);
			}
		}
	}
}

void UGameFeatureAction_AddSpawnedActors::Reset()
{
	for (TWeakObjectPtr<AActor>& ActorPtr : SpawnedActors)
	{
		if (ActorPtr.IsValid())
		{
			ActorPtr->Destroy();
		}
	}
}

#undef LOCTEXT_NAMESPACE