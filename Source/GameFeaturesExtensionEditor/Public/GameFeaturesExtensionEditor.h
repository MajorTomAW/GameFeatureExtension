#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManagerTypes.h"
#include "Modules/ModuleManager.h"

/** Delegate used to notify when a game feature plugin has been picked */
DECLARE_DELEGATE_OneParam(FOnGameFeaturePluginPicked, const FString&);

class GAMEFEATURESEXTENSIONEDITOR_API FGameFeaturesExtensionEditorModule : public IModuleInterface
{
public:
    //~ Begin IModuleInterface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //~ End IModuleInterface

    virtual TSharedRef<SWidget> CreateGameFeaturePluginPicker(const FOnGameFeaturePluginPicked& OnPicked);
    void CheckPrimaryAssetDataRule(UClass* AssetClass, FPrimaryAssetTypeInfo TypeInfo);

private:
    void OnAssetManagerCreated();
    void AddPrimaryAssetDataRule(UClass* AssetClass, FPrimaryAssetTypeInfo TypeInfo);
};
