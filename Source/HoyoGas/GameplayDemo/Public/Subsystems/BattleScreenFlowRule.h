#pragma once

#include "CoreMinimal.h"
#include "Battle/HoyoBattleTypes.h"
#include "Subsystems/UIFlowRuleBase.h"
#include "BattleScreenFlowRule.generated.h"

class UHoyoBattleEncounterDefinition;
class UHoyoBattleFlowSubsystem;

UCLASS()
class HOYOGAS_API UBattleScreenFlowRule : public UUIFlowRuleBase
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void RefreshBindings() override;

private:
	void HandleBattleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState);
	void HandleBattleEncounterChanged(const UHoyoBattleEncounterDefinition* EncounterDefinition);
	void EvaluateBattleScreenFlow();
	void OpenBattleScreenIfNeeded();
	void CloseBattleScreenIfNeeded();
	void HideExplorationHUDIfNeeded();
	void RestoreExplorationHUDIfNeeded();
	void UnbindBattleFlow();

	TWeakObjectPtr<UHoyoBattleFlowSubsystem> BattleFlowSubsystem;
	FName ActiveBattleScreenTag = NAME_None;
	bool bBattleScreenRequested = false;
	bool bExplorationHUDHiddenForBattle = false;
};
