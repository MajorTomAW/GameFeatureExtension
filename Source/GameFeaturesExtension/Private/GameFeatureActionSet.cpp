// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureActionSet.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "GameFeatureAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureActionSet)

#define LOCTEXT_NAMESPACE "GameFeatures"

UGameFeatureActionSet::UGameFeatureActionSet()
{
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureActionSet::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			const EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry in Actions array at index {0}"),
				FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif
#if WITH_EDITORONLY_DATA
void UGameFeatureActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE