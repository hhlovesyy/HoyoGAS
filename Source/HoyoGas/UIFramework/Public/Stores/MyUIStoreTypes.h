#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MyUIStoreTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct HOYOGAS_API FInventoryItemSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	int32 ScoreValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	TSoftObjectPtr<UTexture2D> BillboardTexture;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FPendingLevelUpSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	bool bHasPendingLevelUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	int32 PendingLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FText PendingLevelText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FText PendingRewardText;
};
