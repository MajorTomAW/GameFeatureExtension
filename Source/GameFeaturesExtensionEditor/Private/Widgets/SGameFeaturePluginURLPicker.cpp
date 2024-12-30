// Copyright Epic Games, Inc. All Rights Reserved.


#include "SGameFeaturePluginURLPicker.h"

#include "GameFeaturesSubsystem.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBorder.h"

class SGameFeaturePluginURLPickerItem : public STableRow<TSharedPtr<FGameFeaturePluginPickerEntry>>
{
public:
	SLATE_BEGIN_ARGS(SGameFeaturePluginURLPickerItem)
	{
	}
		SLATE_ARGUMENT(FText, PluginDisplayName)
		SLATE_ARGUMENT(FText, PluginURL)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		PluginDisplayName = InArgs._PluginDisplayName;
		PluginURL = InArgs._PluginURL;

		this->ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f, 3.f, 6.f, 3.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(PluginDisplayName)
				.ToolTipText(PluginURL)
			]
		];

		ConstructInternal(
			STableRow::FArguments()
			.ShowSelection(true),
			InOwnerTableView);
	}

private:
	FText PluginDisplayName;
	FText PluginURL;
};




BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGameFeaturePluginURLPicker::Construct(const FArguments& InArgs)
{
	OnPicked = InArgs._OnPicked;
	TextFilterPtr = MakeShareable(new FTextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString));

	SAssignNew(PluginsList, SListView<TSharedPtr<FGameFeaturePluginPickerEntry>>)
	.SelectionMode(ESelectionMode::Single)
	.ListItemsSource(&Plugins)
	.OnGenerateRow(this, &SGameFeaturePluginURLPicker::OnGenerateRow)
	.OnSelectionChanged(this, &SGameFeaturePluginURLPicker::OnSelectionChanged)
	.ItemHeight(20.f)
	.HeaderRow
	(
		SNew(SHeaderRow)
		.Visibility(EVisibility::Collapsed)
		+ SHeaderRow::Column(TEXT("Plugin"))
		.DefaultLabel(NSLOCTEXT("GameFeaturePluginURLPicker", "Plugin", "Plugin"))
	);

	TSharedPtr<SWidget> PluginPickerContent =
		SNew(SBox)
		.MaxDesiredHeight(800.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("NoBorder"))
			[
				SNew(SVerticalBox)

				// Search box
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.f, 2.f)
					[
						SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(this, &SGameFeaturePluginURLPicker::OnFilterTextChanged)
					]
				]

				// List of plugins
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SScrollBorder, PluginsList.ToSharedRef())
							[
								PluginsList.ToSharedRef()
							]
						]
					]
				]

				// Plugin count
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.Padding(8.f, 0.f)
					[
						SNew(STextBlock)
						.Text(this, &SGameFeaturePluginURLPicker::GetPluginCountText)
					]
				]
			]
		];

	this->ChildSlot
	[
		PluginPickerContent.ToSharedRef()
	];

	bPendingFocusNextFrame = true;
	bNeedsRefresh = true;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGameFeaturePluginURLPicker::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Move focus to search box if needed
	if (bPendingFocusNextFrame && SearchBox.IsValid())
	{
		FWidgetPath WidgetToFocusPath;
		FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchBox.ToSharedRef(), WidgetToFocusPath);
		FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
		bPendingFocusNextFrame = false;
	}

	// Repopulate the list of plugins if needed
	if (bNeedsRefresh)
	{
		bNeedsRefresh = false;
		PopulatePluginList();
	}
}

TSharedRef<ITableRow> SGameFeaturePluginURLPicker::OnGenerateRow(
	TSharedPtr<FGameFeaturePluginPickerEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGameFeaturePluginURLPickerItem, OwnerTable)
		.PluginDisplayName(FText::FromString(InItem->PluginName))
		.PluginURL(FText::FromString(InItem->PluginURL));
}

void SGameFeaturePluginURLPicker::OnSelectionChanged(
	TSharedPtr<FGameFeaturePluginPickerEntry> InItem, ESelectInfo::Type SelectInfo)
{
	// Don't act on selection change when it is for navigation
	if (SelectInfo == ESelectInfo::OnNavigation)
	{
		return;
	}

	if (!InItem.IsValid())
	{
		return;
	}

	OnPicked.ExecuteIfBound(InItem->PluginURL);
}

FText SGameFeaturePluginURLPicker::GetPluginCountText() const
{
	const int32 NumPlugins = Plugins.Num();
	return FText::Format(NSLOCTEXT("PluginPicker", "PluginCountLabel", "{0} {0}|plural(one=plugin,other=plugins)"), NumPlugins);
}

void SGameFeaturePluginURLPicker::OnFilterTextChanged(const FText& InFilterText)
{
	TextFilterPtr->SetFilterText(InFilterText);
	SearchBox->SetError(TextFilterPtr->GetFilterErrorText());

	bNeedsRefresh = true;
}

void SGameFeaturePluginURLPicker::PopulatePluginList()
{
	Plugins.Empty();
	TArray<FString> PluginURLs;

	const UGameFeaturesSubsystem& GameFeaturesSub = UGameFeaturesSubsystem::Get();
	GameFeaturesSub.GetLoadedGameFeaturePluginFilenamesForCooking(PluginURLs);

	for (const auto& URL : PluginURLs)
	{
		if (TextFilterPtr->GetFilterType() == ETextFilterExpressionType::Empty)
		{
			Plugins.Add(MakeShareable(new FGameFeaturePluginPickerEntry(URL)));
			continue;
		}

		FString Name = FPaths::GetBaseFilename(URL);
		Name.RemoveFromEnd(FPaths::GetExtension(URL));
		
		if (TextFilterPtr->TestTextFilter(FBasicStringFilterExpressionContext(Name)))
		{
			Plugins.Add(MakeShareable(new FGameFeaturePluginPickerEntry(URL, Name)));
		}
	}

	PluginsList->RequestListRefresh();
}
