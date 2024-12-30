// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameFeatureActionSetFactory.h"

#include "GameFeatureActionSet.h"

#define LOCTEXT_NAMESPACE "GameFeaturesEditor"

UGameFeatureActionSetFactory::UGameFeatureActionSetFactory()
{
	SupportedClass = UGameFeatureActionSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UGameFeatureActionSetFactory::ConfigureProperties()
{
	return true;
}

FText UGameFeatureActionSetFactory::GetDisplayName() const
{
	return LOCTEXT("GameFeatureActionSetFactoryDisplayName", "Game Feature Action Set");
}

UObject* UGameFeatureActionSetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UGameFeatureActionSet::StaticClass()));
	return NewObject<UGameFeatureActionSet>(InParent, InClass, InName, Flags);
}

#undef LOCTEXT_NAMESPACE