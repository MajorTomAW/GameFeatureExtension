// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

class FGameFeaturesExtensionEditorStyle final : public FSlateStyleSet
{
public:
	FGameFeaturesExtensionEditorStyle();
	virtual ~FGameFeaturesExtensionEditorStyle();
	friend class FGameFeaturesExtensionEditorModule;

	/** Returns the singleton instance of the style set */
	static FGameFeaturesExtensionEditorStyle& Get();

protected:
	void Initialize();
	static void Shutdown();

private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
	static TUniquePtr<FGameFeaturesExtensionEditorStyle> StyleSet;
};