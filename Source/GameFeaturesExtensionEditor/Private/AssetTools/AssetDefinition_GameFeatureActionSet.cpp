// Copyright Epic Games, Inc. All Rights Reserved.


#include "AssetDefinition_GameFeatureActionSet.h"

#include "GameFeatureActionSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_GameFeatureActionSet)

#define LOCTEXT_NAMESPACE "GameFeaturesEditor"

FText UAssetDefinition_GameFeatureActionSet::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Game Feature Action Set");
}

FLinearColor UAssetDefinition_GameFeatureActionSet::GetAssetColor() const
{
	return FLinearColor(FColor(201, 29, 85));
}

TSoftClassPtr<> UAssetDefinition_GameFeatureActionSet::GetAssetClass() const
{
	return UGameFeatureActionSet::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameFeatureActionSet::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel", "Game Features"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE