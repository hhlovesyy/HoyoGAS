#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/UIFlowRuleBase.h"
#include "RarePickupToastFlowRule.generated.h"

class UMyGameplayTagEventBusSubsystem;
struct FGameplayDemoRarePickupEventPayload;

UCLASS()
class HOYOGAS_API URarePickupToastFlowRule : public UUIFlowRuleBase
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void RefreshBindings() override;

private:
	void HandleRarePickupEvent(const FGameplayTag& EventTag, const FGameplayDemoRarePickupEventPayload& Payload);
	void UnbindEventBus();

	TWeakObjectPtr<UMyGameplayTagEventBusSubsystem> EventBus;
	FDelegateHandle RarePickupEventHandle;
};
