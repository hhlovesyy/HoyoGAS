// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MyAbilitySystemComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOYOGAS_API UMyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMyAbilitySystemComponent();
};
