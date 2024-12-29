// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureAction_SplitscreenConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_SplitscreenConfig)

#define LOCTEXT_NAMESPACE "GameFeatures"

TMap<FObjectKey, int32> UGameFeatureAction_SplitscreenConfig::GlobalDisableVotes;

void UGameFeatureAction_SplitscreenConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (int i = LocalDisableVotes.Num() - 1; i >= 0; i--)
	{
		FObjectKey ViewportKey = LocalDisableVotes[i];
		UGameViewportClient* GVC = Cast<UGameViewportClient>(ViewportKey.ResolveObjectPtr());
		const FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GVC);

		if (GVC && WorldContext)
		{
			if (!Context.ShouldApplyToWorldContext(*WorldContext))
			{
				continue;
			}
		}

		int32& VoteCount = GlobalDisableVotes[ViewportKey];
		if (VoteCount <= 1)
		{
			GlobalDisableVotes.Remove(ViewportKey);

			if (GVC && WorldContext)
			{
				GVC->SetForceDisableSplitscreen(false);
			}
		}
		else
		{
			--VoteCount;
		}

		LocalDisableVotes.RemoveAt(i);
	}
}

void UGameFeatureAction_SplitscreenConfig::OnAddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	if (!bDisableSplitscreen)
	{
		return;
	}

	if (UGameInstance* GameInstance = WorldContext.OwningGameInstance)
	{
		if (UGameViewportClient* GVC = GameInstance->GetGameViewportClient())
		{
			FObjectKey ViewportKey(GVC);
			LocalDisableVotes.Add(ViewportKey);

			int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
			VoteCount++;

			if (VoteCount == 1)
			{
				GVC->SetForceDisableSplitscreen(true);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE