#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIFrameworkTypes.h"
#include "MyUIRegistrySubsystem.generated.h"

class UMyUIRegistryDataAsset;

UCLASS(Config = Game, DefaultConfig)
class HOYOGAS_API UMyUIRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void ReloadRegistry();

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void RegisterScreenConfig(const FMyUIScreenConfig& ScreenConfig);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool FindScreenConfig(FName ScreenTag, FMyUIScreenConfig& OutScreenConfig) const;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework")
	TSoftObjectPtr<UMyUIRegistryDataAsset> RegistryAsset;

	UPROPERTY()
	TMap<FName, FMyUIScreenConfig> ScreenConfigs;

	void AddScreenConfigInternal(const FMyUIScreenConfig& ScreenConfig, bool bWarnOnOverride);
};
