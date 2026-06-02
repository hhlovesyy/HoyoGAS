#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivorWeaponManagerComponent.generated.h"

class USurvivorWeaponDefinition;

UCLASS(ClassGroup = (Survivor), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API USurvivorWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivorWeaponManagerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	void StartDebugWeapon();

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	void StopDebugWeapon();

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	void HandleDebugWeaponFire();

	UFUNCTION(BlueprintCallable, Category = "Survivor|Weapon")
	void DrawDebugFireDirections(const TArray<FVector>& Directions, float LineLength);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survivor|Weapon")
	bool bAutoStartDebugWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survivor|Weapon")
	bool bEnableWeaponDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survivor|Weapon")
	TObjectPtr<USurvivorWeaponDefinition> DebugWeaponDefinition = nullptr;

protected:
	FTimerHandle DebugWeaponTimerHandle;
};
