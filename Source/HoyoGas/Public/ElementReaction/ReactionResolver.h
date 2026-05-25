#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ElementTypes.h"
#include "ReactionRuleRow.h"
#include "ReactionVisualRow.h"
#include "ElementAuraConfigRow.h"

class FReactionResolver
{
public:
	static FReactionResult Resolve(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent,
		float TargetResistance,
		const UDataTable* ReactionRuleDataTable,
		const UDataTable* ReactionVisualDataTable,
		const UDataTable* AuraConfigDataTable
	);

	static FString ElementToString(EGenshinElementType Element);
	static FString ReactionToString(EReactionType Reaction);

private:
	static FReactionResult MakeDefaultResult(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent
	);

	static FReactionResult HandleNoAuraCase(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent,
		const UDataTable* ReactionVisualDataTable,
		const UDataTable* AuraConfigDataTable
	);

	static FReactionResult HandleSameElementRefreshCase(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent,
		const UDataTable* ReactionVisualDataTable,
		const UDataTable* AuraConfigDataTable
	);

	static FReactionResult HandleReactionCase(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent,
		float TargetResistance,
		const FReactionRuleRow& Rule,
		const UDataTable* ReactionVisualDataTable
	);

	static FReactionResult HandleNoReactionOverrideCase(
		const FElementAura& CurrentAura,
		const FElementHitEvent& HitEvent,
		const UDataTable* ReactionVisualDataTable,
		const UDataTable* AuraConfigDataTable
	);

	static void ApplyVisualRowToResult(
		FReactionResult& Result,
		const FReactionVisualRow* VisualRow
	);

	static float CalculateReactionDamage(
		const FElementHitEvent& HitEvent,
		float TargetResistance,
		const FReactionRuleRow& Rule
	);

	static FElementAura ConsumeAuraValue(
		const FElementAura& CurrentAura,
		float AuraConsumeValue
	);

	static const FReactionRuleRow* FindReactionRule(
		EGenshinElementType OldAura,
		EGenshinElementType IncomingElement,
		const UDataTable* ReactionRuleDataTable
	);

	static const FReactionVisualRow* FindReactionVisualRule(
		EReactionType ReactionType,
		const UDataTable* ReactionVisualDataTable
	);
	
	static const FElementAuraConfigRow* FindAuraConfig(
		EGenshinElementType Element,
		const UDataTable* AuraConfigDataTable
	);

	static FElementAura BuildAuraFromConfig(
		EGenshinElementType Element,
		const UDataTable* AuraConfigDataTable
	);

	static FElementAura RefreshAuraBySameElement(
		const FElementAura& CurrentAura,
		const FElementAura& IncomingAura
	);

	static void SyncAuraDurationFromValue(FElementAura& Aura);

	static float GetAuraSecondsPerValue(const FElementAura& Aura);
	static float GetGenshinAuraDurationFromGauge(float GaugeValue);
	
	static float GetAmplifyingEMBonus(float ElementalMastery);
	static float GetSimpleResistMultiplier(float Resistance);
};
