// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureAction_AddInputMappingContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/AssetManager.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputMappingContext)

#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	RegisterInputMappingContexts();
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.ControllersAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}
	
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	UnregisterInputMappingContexts();
}

void UGameFeatureAction_AddInputMappingContext::OnAddToWorld(
	const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (GameInstance == nullptr)
	{
		return;
	}

	if (World == nullptr || !World->IsGameWorld())
	{
		return;
	}

	if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
	{
		UGameFrameworkComponentManager::FExtensionHandlerDelegate AddInputMappingsDelegate =
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);

		TSharedPtr<FComponentRequestHandle> RequestHandle =
			ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddInputMappingsDelegate);

		ActiveData.ExtensionRequestHandles.Add(RequestHandle);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputMappingContext::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	int32 Index = 0;
	for (const FInputMappingContextAndPriority& Entry : InputMappings)
	{
		if (Entry.InputMapping.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InputMappingNull", "InputMapping at index {0} is null"), FText::AsNumber(Index)));
		}

		++Index;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContexts()
{
	RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject
	(
		this,
		&ThisClass::RegisterInputContextMappingsForGameInstance
	);

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator It = WorldContexts.CreateConstIterator(); It; ++It)
	{
		RegisterInputContextMappingsForGameInstance(It->OwningGameInstance);
	}
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance == nullptr)
	{
		return;
	}

	if (GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this))
	{
		return;
	}

	GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::RegisterInputMappingContextsForLocalPlayer);
	GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::UnregisterInputMappingContextsForLocalPlayer);

	for (TArray<ULocalPlayer*>::TConstIterator It = GameInstance->GetLocalPlayerIterator(); It; ++It)
	{
		RegisterInputMappingContextsForLocalPlayer(*It);
	}
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!ensure(LocalPlayer))
	{
		return;
	}

	UE_LOG(LogGameFeatures, Display, TEXT("%hs Registering Input Mapping Contexts for LocalPlayer [%s]"), __func__, *LocalPlayer->GetName());

	UAssetManager& AssetManager = UAssetManager::Get();
	if (UEnhancedInputLocalPlayerSubsystem* InputSub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (UEnhancedInputUserSettings* Settings = InputSub->GetUserSettings())
		{
			for (const auto& Entry : InputMappings)
			{
				// Skip entries that don't want to be registered with the settings
				if (!Entry.bRegisterWithSettings)
				{
					continue;
				}

				const UInputMappingContext* MappingContext = Entry.InputMapping.Get();
				if (!MappingContext)
				{
					MappingContext = Entry.InputMapping.LoadSynchronous();
					ensureAlwaysMsgf(MappingContext, TEXT("Failed to load asset [%s]"), *Entry.InputMapping.ToString());
				}

				if (MappingContext)
				{
					Settings->RegisterInputMappingContext(MappingContext);
				}
			}
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("Failed to find EnhancedInputUserSettings for LocalPlayer [%s]. Input mappings wont be added. Make sure you're set to use the EnhancedInput system via config file."), *LocalPlayer->GetName());
		}
	}
	else
	{
		UE_LOG(LogGameFeatures, Error, TEXT("Failed to find EnhancedInputLocalPlayerSubsystem for LocalPlayer [%s]. Input mappings wont be added. Make sure you're set to use the EnhancedInput system via config file."), *LocalPlayer->GetName());
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContexts()
{
	FWorldDelegates::OnStartGameInstance.Remove(RegisterInputContextMappingsForGameInstanceHandle);
	RegisterInputContextMappingsForGameInstanceHandle.Reset();

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator It = WorldContexts.CreateConstIterator(); It; ++It)
	{
		UnregisterInputContextMappingsForGameInstance(It->OwningGameInstance);
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);

	for (TArray<ULocalPlayer*>::TConstIterator It = GameInstance->GetLocalPlayerIterator(); It; ++It)
	{
		UnregisterInputMappingContextsForLocalPlayer(*It);
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!ensure(LocalPlayer))
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (UEnhancedInputUserSettings* Settings = InputSub->GetUserSettings())
		{
			for (const auto& Entry : InputMappings)
			{
				// Skip entries that don't want to be registered with the settings
				if (!Entry.bRegisterWithSettings)
				{
					continue;
				}

				if (UInputMappingContext* MappingContext = Entry.InputMapping.Get())
				{
					Settings->UnregisterInputMappingContext(MappingContext);
				}
			}
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();
	
	while (!ActiveData.ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> PlayerController = ActiveData.ControllersAddedTo.Top();
		if (PlayerController.IsValid())
		{
			RemoveInputMapping(PlayerController.Get(), ActiveData);
		}
		else
		{
			ActiveData.ControllersAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::HandleControllerExtension(
	AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* PC = CastChecked<APlayerController>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	UE_LOG(LogGameFeatures, Display, TEXT("%hs Handling Controller Extension for Player [%s] (%s)"), __func__, *PC->GetName(), *EventName.ToString());

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) ||
		(EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMapping(PC, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) ||
		(EventName == "BindInputsNow"))
	{
		AddInputMappingForPlayer(PC->GetLocalPlayer(), ActiveData);
	}
}

void UGameFeatureAction_AddInputMappingContext::AddInputMappingForPlayer(UPlayer* Player, FPerContextData& ActiveData)
{
	UE_LOG(LogGameFeatures, Display, TEXT("%hs Adding Input Mapping Contexts for Player [%s]"), __func__, *Player->GetName());
	
	if (ULocalPlayer* LP = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const auto& Entry : InputMappings)
			{
				if (const UInputMappingContext* MappingContext = Entry.InputMapping.Get())
				{
					InputSub->AddMappingContext(MappingContext, Entry.Priority);
				}
			}

			UE_LOG(LogGameFeatures, Display, TEXT("Added Input Mapping Contexts (%d) for Player [%s]"), InputMappings.Num(), *Player->GetName());
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("Failed to find EnhancedInputLocalPlayerSubsystem for LocalPlayer [%s]. Input mappings wont be added. Make sure you're set to use the EnhancedInput system via config file."), *Player->GetName());
		}
	}
	else
	{
		UE_LOG(LogGameFeatures, Error, TEXT("Failed to cast Player to LocalPlayer for Player [%s]. Input mappings wont be added."), *Player->GetName());
	}
}

void UGameFeatureAction_AddInputMappingContext::RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData)
{
	if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const auto& Entry : InputMappings)
			{
				if (const UInputMappingContext* MappingContext = Entry.InputMapping.Get())
				{
					InputSub->RemoveMappingContext(MappingContext);
				}
			}
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("Failed to find EnhancedInputLocalPlayerSubsystem for LocalPlayer [%s]. Input mappings wont be removed. Make sure you're set to use the EnhancedInput system via config file."), *PlayerController->GetName());
		}
	}
	else
	{
		UE_LOG(LogGameFeatures, Error, TEXT("Failed to find LocalPlayer for PlayerController [%s]. Input mappings wont be removed."), *PlayerController->GetName());
	}
}

#undef LOCTEXT_NAMESPACE