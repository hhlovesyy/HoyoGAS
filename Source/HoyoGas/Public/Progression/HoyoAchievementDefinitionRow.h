#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Progression/HoyoCheckTypes.h"
#include "HoyoAchievementDefinitionRow.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FHoyoAchievementDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	FName AchievementId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	FHoyoCheckConditionSet UnlockConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Achievement")
	FText ToastMessage;
};
