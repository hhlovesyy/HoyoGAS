#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MyUIStoreRegistryDataAsset.generated.h"

class UUIStoreBase;

USTRUCT(BlueprintType)
struct HOYOGAS_API FMyUIStoreRegistryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	FGameplayTag StoreTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	TSoftClassPtr<UUIStoreBase> StoreClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	bool bAutoCreate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	bool bPersistent = true;
};

UCLASS(BlueprintType)
class HOYOGAS_API UMyUIStoreRegistryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UIStore")
	TArray<FMyUIStoreRegistryEntry> Stores;
};
