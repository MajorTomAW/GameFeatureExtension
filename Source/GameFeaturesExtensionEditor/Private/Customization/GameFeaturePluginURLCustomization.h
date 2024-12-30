// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"

class IPropertyHandle;

class FGameFeaturePluginURLCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FGameFeaturePluginURLCustomization);
	}

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

private:
	TSharedRef<SWidget> GenerateMenuContent();
	FText GetPluginName() const;
	FText GetPluginURL() const;

	void OnPluginPicked(const FString& PluginURL);

private:
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<SComboButton> ComboButton;
};