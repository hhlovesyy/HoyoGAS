#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayDemoUIFlowSubsystem.generated.h"

class UUIFlowRuleBase;

UCLASS()
class HOYOGAS_API UGameplayDemoUIFlowSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "GameplayDemo|UIFlow")
	void RefreshFlowBindings();

private:
	// The coordinator owns all flow rules as instanced UObjects. They use this subsystem
	// as their Outer and are created during Initialize, then torn down during Deinitialize.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UUIFlowRuleBase>> FlowRules;
};
