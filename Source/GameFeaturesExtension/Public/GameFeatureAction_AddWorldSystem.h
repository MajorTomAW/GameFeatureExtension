// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreTypes.h"
#include "Engine/EngineTypes.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "GameFeatureAction_AddWorldSystem.generated.h"

class FText;
class UWorld;
struct FAssetBundleData;
struct FFrame;
struct FGameFeatureDeactivatingContext;
struct FWorldContext;

/** Base class for world system objects. */
UCLASS(Abstract, Blueprintable)
class UGameFeatureWorldSystem : public UObject
{
	GENERATED_BODY()

public:
	/** Called when the game feature is activating and initializes this world system. */
	virtual void NativeInitialize(const FWorldContext& WorldContext, const FGameFeatureActivatingContext& FeatureContext);

	/** Called when the game feature is activating and initializes this world system. */
	UFUNCTION(BlueprintImplementableEvent, Category = WorldSystem)
	void Initialize(const UObject* WorldContextObject);
};

/** Record for the game feature data. Specifies what type of system to instantiate. */
USTRUCT()
struct FGameFeatureWorldSystemEntry
{
	GENERATED_BODY()

	// The world to spawn this system for (can be left blank, in which case we'll spawn this for all worlds)
	UPROPERTY(EditAnywhere, Category="World System")
	TSoftObjectPtr<UWorld> TargetWorld;

	// The type to instantiate
	UPROPERTY(EditAnywhere, Category = "World System")
	TSubclassOf<UGameFeatureWorldSystem> SystemType;
};	

/**
 * GameFeatureAction responsible for spawning singleton style world objects for a specific feature.
 * (Allows for users to define those system objects in Blueprints)
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add World System"))
class UGameFeatureAction_AddWorldSystem : public UGameFeatureAction_WorldActionBase
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

	UPROPERTY(EditAnywhere, Category="World System")
	TArray<FGameFeatureWorldSystemEntry> WorldSystemsList;

	UFUNCTION(BlueprintCallable, Category = "Game Features|World Systems", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "SystemType"))
	static UGameFeatureWorldSystem* FindGameFeatureWorldSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType, UObject* WorldContextObject);

private:
	//~ Begin UGameFeatureAction_WorldActionBase interface
	virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGameFeatureAction_WorldActionBase interface

	void Reset();
};


/** 
 * C++ WorldSubsystem to manage requested system instances 
 * (ref counts requests to account for multiple feature requesting the same system). 
 */
UCLASS()
class UGameFeatureWorldSystemManager : public UWorldSubsystem
{
	GENERATED_BODY()

	//~ Begin UWorldSubsystem interface
	virtual void PostInitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	//~ End UWorldSubsystem interface

private:
	friend class UGameFeatureAction_AddWorldSystem;

	UGameFeatureWorldSystem* RequestSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType);
	void ReleaseRequestForSystemOfType(TSubclassOf<UGameFeatureWorldSystem> SystemType);

	UPROPERTY(transient)
	TMap<TObjectPtr<UGameFeatureWorldSystem>, int32> SystemInstances;

	bool bIsInitialized = false;
};