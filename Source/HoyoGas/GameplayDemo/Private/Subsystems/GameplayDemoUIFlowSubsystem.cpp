#include "Subsystems/GameplayDemoUIFlowSubsystem.h"

#include "Subsystems/BattleScreenFlowRule.h"
#include "Subsystems/LevelUpDialogFlowRule.h"
#include "Subsystems/RarePickupToastFlowRule.h"
#include "Subsystems/UIFlowRuleBase.h"

void UGameplayDemoUIFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FlowRules.Reset();

	URarePickupToastFlowRule* RarePickupToastRule = NewObject<URarePickupToastFlowRule>(this);
	RarePickupToastRule->Initialize(this);
	FlowRules.Add(RarePickupToastRule);

	ULevelUpDialogFlowRule* LevelUpDialogRule = NewObject<ULevelUpDialogFlowRule>(this);
	LevelUpDialogRule->Initialize(this);
	FlowRules.Add(LevelUpDialogRule);

	UBattleScreenFlowRule* BattleScreenRule = NewObject<UBattleScreenFlowRule>(this);
	BattleScreenRule->Initialize(this);
	FlowRules.Add(BattleScreenRule);

	RefreshFlowBindings();
}

void UGameplayDemoUIFlowSubsystem::Deinitialize()
{
	for (int32 Index = FlowRules.Num() - 1; Index >= 0; --Index)
	{
		if (UUIFlowRuleBase* FlowRule = FlowRules[Index])
		{
			FlowRule->Deinitialize();
		}
	}

	FlowRules.Empty();
	Super::Deinitialize();
}

void UGameplayDemoUIFlowSubsystem::RefreshFlowBindings()
{
	for (UUIFlowRuleBase* FlowRule : FlowRules)
	{
		if (FlowRule)
		{
			FlowRule->RefreshBindings();
		}
	}
}
