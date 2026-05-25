#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDefinitionRow.generated.h"

class UStaticMesh;
class UTexture2D;

USTRUCT(BlueprintType)
struct HOYOGAS_API FItemDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	TSoftObjectPtr<UTexture2D> BillboardTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FLinearColor TintColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	int32 ScoreValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	FName ShapeTypeTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayDemo")
	int32 Rarity = 1;
};
