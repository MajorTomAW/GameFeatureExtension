// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"

#include "GameFeatureAction_WorldActionBase.generated.h"

class FDelegateHandle;
class UGameInstance;
class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

/**
 * Base class for GameFeatureActions that affect the world in some way.
 * For example, adding input bindings, setting up player controllers, etc.
 */
UCLASS(Abstract, MinimalAPI)
class UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction Interface
	GAMEFEATURESEXTENSION_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	GAMEFEATURESEXTENSION_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction Interface

protected:
	/** Called when the game instance starts */
	GAMEFEATURESEXTENSION_API void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** Subclasses should override this to add their world-specific functionality */
	GAMEFEATURESEXTENSION_API virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
		PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::OnAddToWorld, );

private:
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};
