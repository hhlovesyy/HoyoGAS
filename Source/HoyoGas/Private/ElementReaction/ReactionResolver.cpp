#include "ElementReaction/ReactionResolver.h"

namespace
{
	constexpr float GenshinAuraDurationScale = 2.5f;
	constexpr float GenshinAuraDurationBias = 7.0f;
	constexpr float DefaultAuraGaugeValue = 1.0f;
}

FReactionResult FReactionResolver::Resolve(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent,
	float TargetResistance,
	const UDataTable* ReactionRuleDataTable,
	const UDataTable* ReactionVisualDataTable,
	const UDataTable* AuraConfigDataTable)
{
	if (CurrentAura.Element == EGenshinElementType::None)
	{
		return HandleNoAuraCase(CurrentAura, HitEvent, ReactionVisualDataTable, AuraConfigDataTable);
	}

	if (CurrentAura.Element == HitEvent.IncomingElement)
	{
		return HandleSameElementRefreshCase(CurrentAura, HitEvent, ReactionVisualDataTable, AuraConfigDataTable);
	}

	const FReactionRuleRow* Rule = FindReactionRule(
		CurrentAura.Element,
		HitEvent.IncomingElement,
		ReactionRuleDataTable
	);

	if (Rule)
	{
		return HandleReactionCase(CurrentAura, HitEvent, TargetResistance, *Rule, ReactionVisualDataTable);
	}

	return HandleNoReactionOverrideCase(CurrentAura, HitEvent, ReactionVisualDataTable, AuraConfigDataTable);
}

FReactionResult FReactionResolver::MakeDefaultResult(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent)
{
	FReactionResult Result;
	Result.FinalDamage = HitEvent.BaseDamage;
	Result.NewAura = CurrentAura;
	Result.Reaction = EReactionType::None;
	Result.ReactionDisplayText = FText::FromString(TEXT("无反应"));
	Result.ReactionColor = FLinearColor::White;
	Result.DamageTextStyle = EDamageTextStyleType::Normal;
	Result.bUseFreezeMaterial = false;
	Result.FreezeAmount = 0.f;
	Result.ReactionFXRowName = NAME_None;
	return Result;
}

FReactionResult FReactionResolver::HandleNoAuraCase(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent,
	const UDataTable* ReactionVisualDataTable,
	const UDataTable* AuraConfigDataTable)
{
	FReactionResult Result = MakeDefaultResult(CurrentAura, HitEvent);
	Result.NewAura = BuildAuraFromConfig(HitEvent.IncomingElement, AuraConfigDataTable);
	ApplyVisualRowToResult(Result, FindReactionVisualRule(EReactionType::None, ReactionVisualDataTable));
	return Result;
}

FReactionResult FReactionResolver::HandleSameElementRefreshCase(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent,
	const UDataTable* ReactionVisualDataTable,
	const UDataTable* AuraConfigDataTable)
{
	FReactionResult Result = MakeDefaultResult(CurrentAura, HitEvent);

	if (const FElementAuraConfigRow* AuraRow = FindAuraConfig(HitEvent.IncomingElement, AuraConfigDataTable))
	{
		switch (AuraRow->RefreshPolicy)
		{
		case EAuraRefreshPolicy::RefreshDuration:
			Result.NewAura = RefreshAuraBySameElement(
				CurrentAura,
				BuildAuraFromConfig(HitEvent.IncomingElement, AuraConfigDataTable)
			);
			break;

		case EAuraRefreshPolicy::OverrideAll:
			Result.NewAura = BuildAuraFromConfig(HitEvent.IncomingElement, AuraConfigDataTable);
			break;

		case EAuraRefreshPolicy::IgnoreIfSameElement:
			Result.NewAura = CurrentAura;
			break;

		default:
			Result.NewAura = CurrentAura;
			break;
		}
	}
	else
	{
		Result.NewAura = CurrentAura;
	}

	ApplyVisualRowToResult(Result, FindReactionVisualRule(EReactionType::None, ReactionVisualDataTable));
	return Result;
}

FReactionResult FReactionResolver::HandleReactionCase(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent,
	float TargetResistance,
	const FReactionRuleRow& Rule,
	const UDataTable* ReactionVisualDataTable)
{
	FReactionResult Result = MakeDefaultResult(CurrentAura, HitEvent);
	Result.Reaction = Rule.ReactionType;
	Result.bApplyFrozen = Rule.bApplyFrozen;
	Result.FinalDamage = CalculateReactionDamage(HitEvent, TargetResistance, Rule);
	Result.NewAura = ConsumeAuraValue(CurrentAura, Rule.AuraConsumeValue);

	ApplyVisualRowToResult(Result, FindReactionVisualRule(Result.Reaction, ReactionVisualDataTable));
	return Result;
}

FReactionResult FReactionResolver::HandleNoReactionOverrideCase(
	const FElementAura& CurrentAura,
	const FElementHitEvent& HitEvent,
	const UDataTable* ReactionVisualDataTable,
	const UDataTable* AuraConfigDataTable)
{
	FReactionResult Result = MakeDefaultResult(CurrentAura, HitEvent);

	if (const FElementAuraConfigRow* AuraRow = FindAuraConfig(HitEvent.IncomingElement, AuraConfigDataTable))
	{
		if (AuraRow->bCanOverrideOtherAuraWhenNoReaction)
		{
			Result.NewAura = BuildAuraFromConfig(HitEvent.IncomingElement, AuraConfigDataTable);
		}
		else
		{
			Result.NewAura = CurrentAura;
		}
	}
	else
	{
		Result.NewAura = BuildAuraFromConfig(HitEvent.IncomingElement, AuraConfigDataTable);
	}

	ApplyVisualRowToResult(Result, FindReactionVisualRule(EReactionType::None, ReactionVisualDataTable));
	return Result;
}

void FReactionResolver::ApplyVisualRowToResult(
	FReactionResult& Result,
	const FReactionVisualRow* VisualRow)
{
	if (!VisualRow)
	{
		return;
	}

	Result.ReactionDisplayText = VisualRow->ReactionDisplayText;
	Result.ReactionColor = VisualRow->ReactionColor;
	Result.DamageTextStyle = VisualRow->DamageTextStyle;
	Result.bUseFreezeMaterial = VisualRow->bUseFreezeMaterial;
	Result.FreezeAmount = VisualRow->FreezeAmount;
	Result.ReactionFXRowName = VisualRow->ReactionFXRowName;
}

float FReactionResolver::CalculateReactionDamage(
	const FElementHitEvent& HitEvent,
	float TargetResistance,
	const FReactionRuleRow& Rule)
{
	switch (Rule.ReactionType)
	{
	case EReactionType::Vaporize:
	case EReactionType::Melt:
	{
		const float EMBonus = GetAmplifyingEMBonus(HitEvent.SourceStats.ElementalMastery);
		const float ResistMultiplier = GetSimpleResistMultiplier(TargetResistance);

		return HitEvent.BaseDamage
			* Rule.DamageMultiplier
			* EMBonus
			* ResistMultiplier;
	}

	case EReactionType::Frozen:
	{
		const float ResistMultiplier = GetSimpleResistMultiplier(TargetResistance);
		return HitEvent.BaseDamage * ResistMultiplier;
	}

	default:
		return HitEvent.BaseDamage;
	}
}

FElementAura FReactionResolver::ConsumeAuraValue(
	const FElementAura& CurrentAura,
	float AuraConsumeValue)
{
	FElementAura NewAura = CurrentAura;
	NewAura.Value -= AuraConsumeValue;

	if (NewAura.Value <= 0.f)
	{
		return FElementAura{};
	}

	NewAura.Value = FMath::Clamp(NewAura.Value, 0.f, NewAura.MaxValue);
	SyncAuraDurationFromValue(NewAura);
	return NewAura;
}

const FReactionRuleRow* FReactionResolver::FindReactionRule(
	EGenshinElementType OldAura,
	EGenshinElementType IncomingElement,
	const UDataTable* ReactionRuleDataTable)
{
	if (!ReactionRuleDataTable)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("ReactionRuleLookup"));

	TArray<FReactionRuleRow*> AllRows;
	ReactionRuleDataTable->GetAllRows<FReactionRuleRow>(ContextString, AllRows);

	for (const FReactionRuleRow* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}

		if (Row->OldAura == OldAura && Row->IncomingElement == IncomingElement)
		{
			return Row;
		}
	}

	return nullptr;
}

const FReactionVisualRow* FReactionResolver::FindReactionVisualRule(
	EReactionType ReactionType,
	const UDataTable* ReactionVisualDataTable)
{
	if (!ReactionVisualDataTable)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("ReactionVisualLookup"));

	TArray<FReactionVisualRow*> AllRows;
	ReactionVisualDataTable->GetAllRows<FReactionVisualRow>(ContextString, AllRows);

	for (const FReactionVisualRow* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}

		if (Row->ReactionType == ReactionType)
		{
			return Row;
		}
	}

	return nullptr;
}

FString FReactionResolver::ElementToString(EGenshinElementType Element)
{
	switch (Element)
	{
	case EGenshinElementType::Pyro:
		return TEXT("火元素");
	case EGenshinElementType::Hydro:
		return TEXT("水元素");
	case EGenshinElementType::Cryo:
		return TEXT("冰元素");
	default:
		return TEXT("None");
	}
}

FString FReactionResolver::ReactionToString(EReactionType Reaction)
{
	switch (Reaction)
	{
	case EReactionType::Vaporize:
		return TEXT("蒸发反应");
	case EReactionType::Melt:
		return TEXT("融化反应");
	case EReactionType::Frozen:
		return TEXT("冻结反应");
	default:
		return TEXT("None");
	}
}

float FReactionResolver::GetAmplifyingEMBonus(float ElementalMastery)
{
	return 1.0f + (2.78f * ElementalMastery) / (ElementalMastery + 1400.0f);
}

float FReactionResolver::GetSimpleResistMultiplier(float Resistance)
{
	return 1.0f - Resistance;
}

const FElementAuraConfigRow* FReactionResolver::FindAuraConfig(
	EGenshinElementType Element,
	const UDataTable* AuraConfigDataTable)
{
	if (!AuraConfigDataTable)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("AuraConfigLookup"));

	TArray<FElementAuraConfigRow*> AllRows;
	AuraConfigDataTable->GetAllRows<FElementAuraConfigRow>(ContextString, AllRows);

	for (const FElementAuraConfigRow* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}

		if (Row->ElementType == Element)
		{
			return Row;
		}
	}

	return nullptr;
}

FElementAura FReactionResolver::BuildAuraFromConfig(
	EGenshinElementType Element,
	const UDataTable* AuraConfigDataTable)
{
	FElementAura Aura;
	Aura.Element = Element;

	if (const FElementAuraConfigRow* Row = FindAuraConfig(Element, AuraConfigDataTable))
	{
		const float GaugeValue = FMath::Max(Row->DefaultAuraValue, 0.f);
		const float AuraDuration = GetGenshinAuraDurationFromGauge(GaugeValue);

		Aura.Value = GaugeValue;
		Aura.MaxValue = GaugeValue;
		Aura.Duration = AuraDuration;
		Aura.MaxDuration = AuraDuration;
	}
	else
	{
		const float AuraDuration = GetGenshinAuraDurationFromGauge(DefaultAuraGaugeValue);

		Aura.Value = DefaultAuraGaugeValue;
		Aura.MaxValue = DefaultAuraGaugeValue;
		Aura.Duration = AuraDuration;
		Aura.MaxDuration = AuraDuration;
	}

	return Aura;
}

FElementAura FReactionResolver::RefreshAuraBySameElement(
	const FElementAura& CurrentAura,
	const FElementAura& IncomingAura)
{
	if (CurrentAura.Element == EGenshinElementType::None || CurrentAura.MaxValue <= 0.f || CurrentAura.MaxDuration <= 0.f)
	{
		return IncomingAura;
	}

	FElementAura RefreshedAura = CurrentAura;
	RefreshedAura.Value = FMath::Max(CurrentAura.Value, IncomingAura.Value);
	RefreshedAura.MaxValue = FMath::Max(CurrentAura.MaxValue, RefreshedAura.Value);
	RefreshedAura.MaxDuration = RefreshedAura.MaxValue * GetAuraSecondsPerValue(CurrentAura);
	SyncAuraDurationFromValue(RefreshedAura);
	return RefreshedAura;
}

void FReactionResolver::SyncAuraDurationFromValue(FElementAura& Aura)
{
	if (Aura.Value <= 0.f || Aura.MaxValue <= 0.f || Aura.MaxDuration <= 0.f)
	{
		Aura.Value = 0.f;
		Aura.Duration = 0.f;
		Aura.MaxValue = 0.f;
		Aura.MaxDuration = 0.f;
		return;
	}

	Aura.Value = FMath::Clamp(Aura.Value, 0.f, Aura.MaxValue);
	Aura.Duration = Aura.Value * GetAuraSecondsPerValue(Aura);
}

float FReactionResolver::GetAuraSecondsPerValue(const FElementAura& Aura)
{
	if (Aura.MaxValue <= 0.f || Aura.MaxDuration <= 0.f)
	{
		return 0.f;
	}

	return Aura.MaxDuration / Aura.MaxValue;
}

float FReactionResolver::GetGenshinAuraDurationFromGauge(float GaugeValue)
{
	if (GaugeValue <= 0.f)
	{
		return 0.f;
	}

	return (GenshinAuraDurationScale * GaugeValue) + GenshinAuraDurationBias; //只是一个经验公式，因为元素量也会随着时间Tick而减少
}
