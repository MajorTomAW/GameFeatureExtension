#include "GameFeaturesExtensionEditor.h"

#include "GameFeatureActionSet.h"
#include "Customization/GameFeaturePluginURLCustomization.h"
#include "Engine/AssetManager.h"
#include "SSettingsEditorCheckoutNotice.h"
#include "Engine/AssetManager.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "Engine/AssetManagerSettings.h"
#include "Framework/Notifications/NotificationManager.h"

#include "Styles/GameFeaturesExtensionEditorStyle.h"
#include "Widgets/SGameFeaturePluginURLPicker.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FGameFeaturesExtensionEditorModule"

void FGameFeaturesExtensionEditorModule::StartupModule()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomPropertyTypeLayout("GameFeaturePluginURL",
        FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameFeaturePluginURLCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	FGameFeaturesExtensionEditorStyle::Get();

	UAssetManager::CallOrRegister_OnAssetManagerCreated(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGameFeaturesExtensionEditorModule::OnAssetManagerCreated));
}

void FGameFeaturesExtensionEditorModule::OnAssetManagerCreated()
{
	FPrimaryAssetTypeInfo TypeInfo;
	TypeInfo.bHasBlueprintClasses = false;
	CheckPrimaryAssetDataRule(UGameFeatureActionSet::StaticClass(), TypeInfo);
}

void FGameFeaturesExtensionEditorModule::ShutdownModule()
{
    FGameFeaturesExtensionEditorStyle::Shutdown();
}

TSharedRef<SWidget> FGameFeaturesExtensionEditorModule::CreateGameFeaturePluginPicker(const FOnGameFeaturePluginPicked& OnPicked)
{
	return SNew(SGameFeaturePluginURLPicker)
		.OnPicked(OnPicked);
}

void FGameFeaturesExtensionEditorModule::CheckPrimaryAssetDataRule(UClass* AssetClass, FPrimaryAssetTypeInfo TypeInfo)
{
	const FPrimaryAssetId TestAssetId(AssetClass->GetFName(), NAME_None);
	FPrimaryAssetRules Rules = UAssetManager::Get().GetPrimaryAssetRules(TestAssetId);
	
	if (FApp::HasProjectName() && Rules.IsDefault())
	{
		FMessageLog("LoadErrors").Error()
		->AddToken(FTextToken::Create(FText::Format(LOCTEXT("MissingRuleForPrimaryAsset", "Asset Manager settings do not include an entry for assets of type {0}, which is required for game feature plugins to function."), FText::FromName(AssetClass->GetFName()))))
		->AddToken(FActionToken::Create(LOCTEXT("AddRuleForPrimaryAsset", "Add entry to PrimaryAssetTypesToScan ?"), FText(),
			FOnActionTokenExecuted::CreateRaw(this, &FGameFeaturesExtensionEditorModule::AddPrimaryAssetDataRule, AssetClass, TypeInfo), true));
	}
}

void FGameFeaturesExtensionEditorModule::AddPrimaryAssetDataRule(UClass* AssetClass, FPrimaryAssetTypeInfo TypeInfo)
{
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();
		
	bool bSuccess = false;
	FText NotificationOpText;

	if (!SettingsHelpers::IsCheckedOut(ConfigFileName, true))
	{
		FText ErrorMessage;
		bSuccess = SettingsHelpers::CheckOutOrAddFile(ConfigFileName, true, IsRunningCommandlet(), &ErrorMessage);
		if (bSuccess)
		{
			NotificationOpText = LOCTEXT("CheckedOutAssetManagerIni", "Checked out {0}");
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("%s"), *ErrorMessage.ToString());
			bSuccess = SettingsHelpers::MakeWritable(ConfigFileName);

			if (bSuccess)
			{
				NotificationOpText = LOCTEXT("MadeWritableAssetManagerIni", "Made {0} writable (you may need to manually add to revision control)");
			}
			else
			{
				NotificationOpText = LOCTEXT("FailedToTouchAssetManagerIni", "Failed to check out {0} or make it writable, so no rule was added");
			}
		}
	}
	else
	{
		NotificationOpText = LOCTEXT("UpdatedAssetManagerIni", "Updated {0}");
		bSuccess = true;
	}

	// Add the rule to project settings
	if (bSuccess)
	{
		FPrimaryAssetTypeInfo NewTypeInfo
		(
			AssetClass->GetFName(),
			AssetClass,
			true,
			false
		);

		NewTypeInfo.bHasBlueprintClasses = TypeInfo.bHasBlueprintClasses;
		NewTypeInfo.bIsDynamicAsset = TypeInfo.bIsDynamicAsset;
		NewTypeInfo.bIsEditorOnly = TypeInfo.bIsEditorOnly;

		NewTypeInfo.Rules.bApplyRecursively = true;
		NewTypeInfo.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;

		Settings->Modify(true);
		Settings->PrimaryAssetTypesToScan.Add(NewTypeInfo);
		Settings->PostEditChange();
		Settings->TryUpdateDefaultConfigFile();

		UAssetManager::Get().ReinitializeFromConfig();
	}

	// Show a message that the file was checked out/updated and must be submitted
	FNotificationInfo Info(FText::Format(NotificationOpText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGameFeaturesExtensionEditorModule, GameFeaturesExtensionEditor)