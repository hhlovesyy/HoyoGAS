#pragma once

#include "CoreMinimal.h"
#include "Subsystems/UIFlowRuleBase.h"
#include "LevelUpDialogFlowRule.generated.h"

class UProgressionUIStore;
class UUIStoreBase;

UCLASS()
class HOYOGAS_API ULevelUpDialogFlowRule : public UUIFlowRuleBase
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void RefreshBindings() override;

private:
	void HandleStoreChanged(UUIStoreBase* ChangedStore);
	void EvaluateLevelUpFlow();
	void UnbindProgressionStore();

	TWeakObjectPtr<UProgressionUIStore> ProgressionStore;
	bool bLevelUpDialogRequested = false;
};
