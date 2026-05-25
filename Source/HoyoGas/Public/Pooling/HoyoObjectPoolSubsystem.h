#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HoyoObjectPoolSubsystem.generated.h"

USTRUCT()
struct FHoyoUserWidgetPool
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> IdleWidgets;
};

UCLASS()
class HOYOGAS_API UHoyoObjectPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "HoyoGas|ObjectPool")
	UUserWidget* AcquireUserWidget(TSubclassOf<UUserWidget> WidgetClass, UObject* OwningContext);

	UFUNCTION(BlueprintCallable, Category = "HoyoGas|ObjectPool")
	bool ReleaseUserWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "HoyoGas|ObjectPool")
	void TrimUserWidgetPool(TSubclassOf<UUserWidget> WidgetClass, int32 MaxIdleCount);

	UFUNCTION(BlueprintCallable, Category = "HoyoGas|ObjectPool")
	void ClearUserWidgetPools();

private:
	FHoyoUserWidgetPool& FindOrAddWidgetPool(TSubclassOf<UUserWidget> WidgetClass);
	FHoyoUserWidgetPool* FindWidgetPool(TSubclassOf<UUserWidget> WidgetClass);
	UUserWidget* CreateUserWidget(TSubclassOf<UUserWidget> WidgetClass, UObject* OwningContext) const;

	UPROPERTY(Transient)
	TArray<FHoyoUserWidgetPool> WidgetPools;
};
