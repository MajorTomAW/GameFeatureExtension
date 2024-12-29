// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "GameplayTagContainer.h"
#include "UIExtensionSystem.h"

#include "GameFeatureAction_AddWidget.generated.h"

class UCommonActivatableWidget;
struct FWorldContext;
struct FComponentRequestHandle;

/**
 * Request to add a layout widget to the player's viewport.
 */
USTRUCT()
struct FGameFeatureWidgetLayoutRequest
{
	GENERATED_BODY()

public:
	/** The layout widget to add */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AssetBundles = "Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	/** The layer tag to insert this widget into */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
};

/**
 * Request to add a single widget to the player's HUD.
 */
USTRUCT()
struct FGameFeatureWidgetHUDElementRequest
{
	GENERATED_BODY()

public:
	/** The widget element to add */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AssetBundles = "Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** The slot tag to insert this widget into */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.Slot"))
	FGameplayTag SlotTag;

	/** The priority of this widget, higher values will be drawn on top of lower values */
	UPROPERTY(EditAnywhere, Category = "UI")
	int32 Priority = -1;
};

/**
 * GameFeatureAction responsible for adding widgets to the player's viewport.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Widget"))
class UGameFeatureAction_AddWidget : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction Interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~ End UGameFeatureAction Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

private:
	/** List of layout widgets to add */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (TitleProperty = "{LayerTag} -> {LayoutClass}"))
	TArray<FGameFeatureWidgetLayoutRequest> Layouts;

	/** List of HUD widgets to add */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (TitleProperty = "{SlotTag} -> {WidgetClass}"))
	TArray<FGameFeatureWidgetHUDElementRequest> Widgets;

private:
	struct FPerActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		TArray<FUIExtensionHandle> ExtensionHandles;
	};

	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~ Begin UGameFeatureAction_WorldActionBase Interface
	virtual void OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~ End UGameFeatureAction_WorldActionBase Interface

	void Reset(FPerContextData& ActiveData);
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	void AddWidgets(AActor* Actor, FPerContextData& ActiveData);
	void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);
};
