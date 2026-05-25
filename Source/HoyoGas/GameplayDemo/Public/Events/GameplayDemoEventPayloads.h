#pragma once

#include "CoreMinimal.h"
#include "GameplayDemoEventPayloads.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FGameplayDemoRarePickupEventPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo|Events")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo|Events")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo|Events")
	FText ToastMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo|Events")
	int32 Rarity = 0;
};
