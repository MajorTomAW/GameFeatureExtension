// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeaturePluginURLCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameFeaturePluginURL.h"
#include "GameFeaturesExtensionEditor.h"
#include "Styles/GameFeaturesExtensionEditorStyle.h"

void FGameFeaturePluginURLCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructPropertyHandle = PropertyHandle;

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	[
		SAssignNew(ComboButton, SComboButton)
		.ContentPadding(0.f)
		.IsEnabled(true)
		.HasDownArrow(true)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnGetMenuContent(this, &FGameFeaturePluginURLCustomization::GenerateMenuContent)
		.ToolTipText(this, &FGameFeaturePluginURLCustomization::GetPluginURL)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage)
				.Image(FGameFeaturesExtensionEditorStyle::Get().GetBrush("Icon.GameFeaturePlugin"))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 12.f, 0.f)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(this, &FGameFeaturePluginURLCustomization::GetPluginName)
			]
		]
	];

	// this avoids making duplicate reset boxes
	PropertyHandle->MarkResetToDefaultCustomized();
}

void FGameFeaturePluginURLCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

TSharedRef<SWidget> FGameFeaturePluginURLCustomization::GenerateMenuContent()
{
	FOnGameFeaturePluginPicked OnPicked =
		FOnGameFeaturePluginPicked::CreateSP(this, &FGameFeaturePluginURLCustomization::OnPluginPicked);
	
	return SNew(SBox)
		.WidthOverride(280.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(480.f)
			[
				FModuleManager::LoadModuleChecked<FGameFeaturesExtensionEditorModule>("GameFeaturesExtensionEditor").CreateGameFeaturePluginPicker(OnPicked)
			]
		];
}

FText FGameFeaturePluginURLCustomization::GetPluginName() const
{
	FString PluginName;
	StructPropertyHandle->GetChildHandle("PluginName")->GetValue(PluginName);

	if (!PluginName.IsEmpty())
	{
		return FText::FromString(PluginName);
	}
	
	return FText::FromString(TEXT("None"));
}

FText FGameFeaturePluginURLCustomization::GetPluginURL() const
{
	FString PluginURL;
	StructPropertyHandle->GetChildHandle("PluginURL")->GetValue(PluginURL);

	if (!PluginURL.IsEmpty())
	{
		return FText::FromString(PluginURL);
	}

	return FText::GetEmpty();
}

void FGameFeaturePluginURLCustomization::OnPluginPicked(const FString& PluginURL)
{
	if (StructPropertyHandle && StructPropertyHandle->IsValidHandle())
	{
		const TSharedPtr<IPropertyHandle> ChildHandle_URL = StructPropertyHandle->GetChildHandle("PluginURL");
		const TSharedPtr<IPropertyHandle> ChildHandle_Name = StructPropertyHandle->GetChildHandle("PluginName");
		
		FScopedTransaction Transaction(NSLOCTEXT("GameFeaturesExtensionCustomization", "ChangePluginURL", "Change Plugin URL"));

		ChildHandle_Name->NotifyPreChange();
		ChildHandle_URL->NotifyPreChange();

		ChildHandle_URL->SetValue(PluginURL);
		FString PluginName = FPaths::GetBaseFilename(PluginURL);
		PluginName.RemoveFromEnd(FPaths::GetExtension(PluginURL));
		ChildHandle_Name->SetValue(PluginName);

		ChildHandle_URL->NotifyPostChange(EPropertyChangeType::ValueSet);
		ChildHandle_Name->NotifyPostChange(EPropertyChangeType::ValueSet);

		ChildHandle_URL->NotifyFinishedChangingProperties();
		ChildHandle_Name->NotifyFinishedChangingProperties();
	}

	ComboButton->SetIsOpen(false);
}
