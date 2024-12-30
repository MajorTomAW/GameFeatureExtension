// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeaturesExtensionEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_PLUGIN_BRUSH(RelativePath, ...) FSlateImageBrush(FGameFeaturesExtensionEditorStyle::InContent(RelativePath, ".png"), __VA_ARGS__)

TUniquePtr<FGameFeaturesExtensionEditorStyle> FGameFeaturesExtensionEditorStyle::StyleSet = nullptr;

FGameFeaturesExtensionEditorStyle::FGameFeaturesExtensionEditorStyle()
	: FSlateStyleSet("GameFeaturesExtensionEditorStyle")
{
}

FGameFeaturesExtensionEditorStyle::~FGameFeaturesExtensionEditorStyle()
{
	Shutdown();
}

FGameFeaturesExtensionEditorStyle& FGameFeaturesExtensionEditorStyle::Get()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = MakeUnique<FGameFeaturesExtensionEditorStyle>();
		StyleSet->Initialize();
	}

	return *StyleSet.Get();
}

void FGameFeaturesExtensionEditorStyle::Initialize()
{
	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	Set("Icon.GameFeaturePlugin", new IMAGE_PLUGIN_BRUSH(TEXT("Icon128"), CoreStyleConstants::Icon24x24));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

void FGameFeaturesExtensionEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet);
		StyleSet.Reset();
	}
}

FString FGameFeaturesExtensionEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("GameFeatures"))->GetBaseDir()
		/ TEXT("Templates/GameFeaturePluginContentOnly/Resources");
	
	return (ContentDir / RelativePath) + Extension;
}
