// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureAction_WorldActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_WorldActionBase)

void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Bind to the game instance start delegate
	GameInstanceStartHandles.FindOrAdd(Context) = FWorldDelegates::OnStartGameInstance.AddUObject
	(
		this,
		&ThisClass::HandleGameInstanceStart,
		FGameFeatureStateChangeContext(Context)
	);

	// Add to any worlds that are already loaded
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			OnAddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FDelegateHandle* Handle = GameInstanceStartHandles.Find(Context);
	if (ensure(Handle))
	{
		FWorldDelegates::OnStartGameInstance.Remove(*Handle);
	}
}

void UGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			OnAddToWorld(*WorldContext, ChangeContext);
		}
	}
}
