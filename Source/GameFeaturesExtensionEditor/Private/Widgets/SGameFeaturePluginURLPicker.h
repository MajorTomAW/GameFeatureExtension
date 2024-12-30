// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeaturesExtensionEditor.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "Widgets/SCompoundWidget.h"

class FGameFeaturePluginPickerEntry : public TSharedFromThis<FGameFeaturePluginPickerEntry>
{
public:
	FGameFeaturePluginPickerEntry() = default;
	FGameFeaturePluginPickerEntry(const FString& InURL, const FString& InName = FString())
	{
		// Prettify the URL /../../../MyGame/Plugins/MyPlugin -> MyGame/Plugins/MyPlugin
		PluginURL = FPaths::ConvertRelativePathToFull(InURL);
		FPaths::NormalizeDirectoryName(PluginURL);
		PluginURL.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir()));

		if (InName.IsEmpty())
		{
			PluginName = FPaths::GetBaseFilename(InURL);
			PluginName.RemoveFromEnd(FPaths::GetExtension(InURL));
		}
		else
		{
			PluginName = InName;
		}
	}

	FString PluginURL;
	FString PluginName;
};

class SGameFeaturePluginURLPicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameFeaturePluginURLPicker)
		{
		}
		SLATE_ARGUMENT(FOnGameFeaturePluginPicked, OnPicked)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FGameFeaturePluginPickerEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnSelectionChanged(TSharedPtr<FGameFeaturePluginPickerEntry> InItem, ESelectInfo::Type SelectInfo);
	FText GetPluginCountText() const;

	void OnFilterTextChanged(const FText& InFilterText);
	
	void PopulatePluginList();

private:
	TSharedPtr<SListView<TSharedPtr<FGameFeaturePluginPickerEntry>>> PluginsList;
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<FTextFilterExpressionEvaluator> TextFilterPtr;

	TArray<TSharedPtr<FGameFeaturePluginPickerEntry>> Plugins;
	
	FOnGameFeaturePluginPicked OnPicked;
	bool bPendingFocusNextFrame = false;
	bool bNeedsRefresh = false;
};
