// Copyright © 2025 MajorT. All Rights Reserved.


#include "GameFeatureAction_AddLevelInstances.h"

#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Delegates/Delegate.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Trace/Detail/Channel.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "UObject/Package.h"
#include "UnrealEngine.h"
#endif

struct FGameFeatureDeactivatingContext;

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddLevelInstances

void UGameFeatureAction_AddLevelInstances::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &UGameFeatureAction_AddLevelInstances::OnWorldCleanup);

	if (!ensureAlways(AddedLevels.Num() == 0))
	{
		DestroyAddedLevels();
	}

	bIsActivated = true;
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddLevelInstances::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	DestroyAddedLevels();
	bIsActivated = false;

	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	Super::OnGameFeatureDeactivating(Context);
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddLevelInstances::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FGameFeatureLevelInstanceEntry& Entry : LevelInstanceList)
	{
		if (Entry.Level.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("LevelEntryNull", "Null level reference at index {0} in LevelInstanceList"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddLevelInstances::OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if (ensureAlways(bIsActivated) && (GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
#if WITH_EDITOR
		// Allow resolving of TargetWorld in proper context
		FTemporaryPlayInEditorIDOverride IDHelper(World->GetPackage()->GetPIEInstanceID());
#endif
		AddedLevels.Reserve(AddedLevels.Num() + LevelInstanceList.Num());

		for (const FGameFeatureLevelInstanceEntry& Entry : LevelInstanceList)
		{
			if (!Entry.Level.IsNull())
			{
				if (!Entry.TargetWorld.IsNull())
				{
					UWorld* TargetWorld = Entry.TargetWorld.Get();
					if (TargetWorld != World)
					{
						// This level is intended for a specific world (not this one)
						continue;
					}
				}

				LoadDynamicLevelForEntry(Entry, World);
			}
		}
	}

	GEngine->BlockTillLevelStreamingCompleted(World);
}

void UGameFeatureAction_AddLevelInstances::OnWorldCleanup(UWorld* World, bool /*bSessionEnded*/, bool /*bCleanupResources*/)
{
	int32 FoundIndex = AddedLevels.IndexOfByPredicate([World](ULevelStreamingDynamic* InStreamingLevel)
		{
			return InStreamingLevel && InStreamingLevel->GetWorld() == World;
		});

	if (FoundIndex != INDEX_NONE)
	{
		CleanUpAddedLevel(AddedLevels[FoundIndex]);
		AddedLevels.RemoveAtSwap(FoundIndex);
	}
}

ULevelStreamingDynamic* UGameFeatureAction_AddLevelInstances::LoadDynamicLevelForEntry(const FGameFeatureLevelInstanceEntry& Entry, UWorld* TargetWorld)
{
	bool bSuccess = false;
	ULevelStreamingDynamic* StreamingLevelRef = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(TargetWorld, Entry.Level, Entry.Location, Entry.Rotation, bSuccess);

	if (!bSuccess)
	{
		UE_LOG(LogGameFeatures, Error, TEXT("[GameFeatureData %s]: Failed to load level instance `%s`."), *GetPathNameSafe(this), *Entry.Level.ToString());
	}
	else if (StreamingLevelRef)
	{
		AddedLevels.Add(StreamingLevelRef);
	}

	return StreamingLevelRef;
}

void UGameFeatureAction_AddLevelInstances::OnLevelLoaded()
{
	if (ensureAlways(bIsActivated))
	{
		// We don't have a way of knowing which instance this was triggered for, so we have to look through them all...
		for (ULevelStreamingDynamic* Level : AddedLevels)
		{
			if (Level && Level->GetLevelStreamingState() == ELevelStreamingState::LoadedNotVisible)
			{
				Level->SetShouldBeVisible(true);
			}
		}
	}
}

void UGameFeatureAction_AddLevelInstances::DestroyAddedLevels()
{
	for (ULevelStreamingDynamic* Level : AddedLevels)
	{
		CleanUpAddedLevel(Level);
	}
	AddedLevels.Empty();
}

void UGameFeatureAction_AddLevelInstances::CleanUpAddedLevel(ULevelStreamingDynamic* Level)
{
	if (Level)
	{
		Level->OnLevelLoaded.RemoveAll(this);
		Level->SetIsRequestingUnloadAndRemoval(true);
	}
}

//////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE