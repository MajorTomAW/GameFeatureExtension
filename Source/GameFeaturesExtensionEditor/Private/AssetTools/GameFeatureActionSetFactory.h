// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "GameFeatureActionSetFactory.generated.h"


UCLASS(HideCategories = Object)
class UGameFeatureActionSetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameFeatureActionSetFactory();

protected:
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual FText GetDisplayName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface
};
