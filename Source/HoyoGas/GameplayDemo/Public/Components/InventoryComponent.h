#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FInventoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "GameplayDemo")
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "GameplayDemo")
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChangedSignature);

UCLASS(ClassGroup = (GameplayDemo), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HOYOGAS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "GameplayDemo|Inventory")
	void AddItem(FName ItemId, int32 Count = 1);

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Inventory")
	int32 GetItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Inventory")
	void GetAllItems(TArray<FInventoryItemStack>& OutItems) const;

	UFUNCTION(BlueprintPure, Category = "GameplayDemo|Inventory")
	int32 GetTotalItemCount() const;

	UPROPERTY(BlueprintAssignable, Category = "GameplayDemo|Inventory")
	FOnInventoryChangedSignature OnInventoryChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "GameplayDemo|Inventory")
	TMap<FName, int32> ItemCounts;
};
