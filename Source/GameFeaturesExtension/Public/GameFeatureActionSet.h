// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFeatureActionSet.generated.h"

class UGameFeatureAction;
/**
 * Defines a set of GameFeatureActions.
 * Useful for grouping actions together and re-using them in multiple places. 
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UGameFeatureActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGameFeatureActionSet();

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	//~ Begin UPrimaryDataAsset Interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~ End UPrimaryDataAsset Interface

public:
	/** List of Game Feature Plugin URL's this action set depends on */
	UPROPERTY(EditDefaultsOnly, Category = "Dependencies")
	TArray<FString> GameFeaturesToEnable;

	/** List of Game Feature Actions to perform as this action set is loaded/activated/deactivated/unloaded */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;
};
