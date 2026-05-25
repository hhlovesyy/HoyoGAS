#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UIFlowRuleBase.generated.h"

class UGameplayDemoUIFlowSubsystem;
class ULocalPlayer;
class UMyPlayerUISubsystem;

UCLASS(Abstract)
class HOYOGAS_API UUIFlowRuleBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(UGameplayDemoUIFlowSubsystem* InOwningFlowSubsystem);
	virtual void Deinitialize();
	virtual void RefreshBindings();

protected:
	UGameplayDemoUIFlowSubsystem* GetOwningFlowSubsystem() const;
	ULocalPlayer* GetOwningLocalPlayer() const;
	UMyPlayerUISubsystem* GetPlayerUISubsystem() const;

private:
	// Owned by the coordinator subsystem. Rules use the coordinator as their Outer,
	// so their lifetime is bounded by the subsystem lifetime on the local player.
	UPROPERTY(Transient)
	TObjectPtr<UGameplayDemoUIFlowSubsystem> OwningFlowSubsystem;
};
