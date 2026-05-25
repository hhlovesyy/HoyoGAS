#pragma once

#include "CoreMinimal.h"
#include "Data/MyUIStoreRegistryDataAsset.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MyUIStoreSubsystem.generated.h"

class UMyUIStoreRegistryDataAsset;
class UUIStoreBase;

UCLASS(Config = Game, DefaultConfig)
class HOYOGAS_API UMyUIStoreSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "UIStore")
	void RefreshPlayerContext();

	UFUNCTION(BlueprintPure, Category = "UIStore")
	UUIStoreBase* GetStoreByTag(FGameplayTag StoreTag) const;

	//给 C++ 程序员用的。直接 `GetStore<UMyInventoryStore>()`，遍历一遍强转类型并返回，类型安全且方便。
	template <typename T>
	T* GetStore() const
	{
		for (const TPair<FGameplayTag, TObjectPtr<UUIStoreBase>>& Pair : StoresByTag)
		{
			if (T* Store = Cast<T>(Pair.Value))
			{
				return Store;
			}
		}

		return nullptr;
	}

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "UIStore")
	TSoftObjectPtr<UMyUIStoreRegistryDataAsset> RegistryAsset;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FMyUIStoreRegistryEntry> RegistryEntriesByTag;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UUIStoreBase>> StoresByTag;

private:
	void LoadRegistry();
	void CreateStoreFromEntry(const struct FMyUIStoreRegistryEntry& Entry);
};
