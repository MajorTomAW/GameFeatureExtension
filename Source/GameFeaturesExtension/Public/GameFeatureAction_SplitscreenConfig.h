// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_SplitscreenConfig.generated.h"

/**
 * GameFeatureAction that sets up splitscreen configuration
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Splitscreen Config"))
class UGameFeatureAction_SplitscreenConfig : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction Interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction Interface

	//~ Begin UGameFeatureAction_WorldActionBase Interface
	virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGameFeatureAction_WorldActionBase Interface

public:
	/** If true, splitscreen won't be enabled */
	UPROPERTY(EditAnywhere, Category = "Splitscreen")
	uint8 bDisableSplitscreen : 1 = true;

private:
	TArray<FObjectKey> LocalDisableVotes;
	static TMap<FObjectKey, int32> GlobalDisableVotes;
};
