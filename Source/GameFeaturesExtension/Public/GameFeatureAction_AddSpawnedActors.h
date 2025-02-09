// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "Math/Transform.h"
#include "Misc/CoreMiscDefines.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "GameFeatureAction_AddSpawnedActors.generated.h"

class AActor;
class FText;
class UObject;
class UWorld;
struct FAssetBundleData;
struct FGameFeatureDeactivatingContext;
struct FWorldContext;

/** Record for the an actor to spawn along with a game feature data. */
USTRUCT()
struct FSpawningActorEntry
{
	GENERATED_BODY()

	// What kind of actor to spawn
	UPROPERTY(EditAnywhere, Category = "Actor")
	TSubclassOf<AActor> ActorType;

	// Where to spawn the actor
	UPROPERTY(EditAnywhere, Category = "Actor|Transform")
	FTransform SpawnTransform;
};

/** Record for the game feature data. Specifies which actors to spawn for target worlds. */
USTRUCT()
struct FSpawningWorldActorsEntry
{
	GENERATED_BODY()

	// The world to spawn the actors for (can be left blank, in which case we'll spawn them for all worlds)
	UPROPERTY(EditAnywhere, Category="Feature Data")
	TSoftObjectPtr<UWorld> TargetWorld;

	// The actors to spawn
	UPROPERTY(EditAnywhere, Category = "Feature Data")
	TArray<FSpawningActorEntry> Actors;
};	

/**
 * GameFeature action which will spawn actors into particular levels at runtime.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Spawned Actors"))
class UGameFeatureAction_AddSpawnedActors final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~ End UGameFeatureAction interface

	//~ Begin UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject interface

	UPROPERTY(EditAnywhere, Category="Actor")
	TArray<FSpawningWorldActorsEntry> ActorsList;

private:
	//~ Begin UGameFeatureAction_WorldActionBase interface
	virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGameFeatureAction_WorldActionBase interface

	void Reset();

	TArray<TWeakObjectPtr<AActor>> SpawnedActors;
};
