#include "Data/SurvivorArenaTypes.h"

#include "GameplayEffect.h"

bool FSurvivorRewardDefinitionRow::ValidateRewardDefinition(FString* OutValidationError) const
{
	auto SetValidationError = [OutValidationError](const FString& Message)
	{
		if (OutValidationError)
		{
			*OutValidationError = Message;
		}
	};

	switch (RewardType)
	{
	case ESurvivorRewardType::None:
		if (!TargetWeaponId.IsNone() || EffectToApply || AbilitySetToGrant)
		{
			SetValidationError(TEXT("RewardType=None must not define TargetWeaponId, EffectToApply, or AbilitySetToGrant."));
			return false;
		}
		return true;

	case ESurvivorRewardType::AttributeModifier:
		if (!EffectToApply)
		{
			SetValidationError(TEXT("RewardType=AttributeModifier requires EffectToApply."));
			return false;
		}
		if (!TargetWeaponId.IsNone())
		{
			SetValidationError(TEXT("RewardType=AttributeModifier must not define TargetWeaponId."));
			return false;
		}
		return true;

	case ESurvivorRewardType::GrantAbilitySet:
		if (!AbilitySetToGrant)
		{
			SetValidationError(TEXT("RewardType=GrantAbilitySet requires AbilitySetToGrant."));
			return false;
		}
		if (!TargetWeaponId.IsNone())
		{
			SetValidationError(TEXT("RewardType=GrantAbilitySet must not define TargetWeaponId."));
			return false;
		}
		return true;

	case ESurvivorRewardType::AddWeapon:
	case ESurvivorRewardType::UpgradeWeapon:
		if (TargetWeaponId.IsNone())
		{
			SetValidationError(TEXT("Weapon rewards require TargetWeaponId."));
			return false;
		}
		return true;

	case ESurvivorRewardType::Heal:
	case ESurvivorRewardType::AddCurrency:
		if (EffectToApply || AbilitySetToGrant || !TargetWeaponId.IsNone())
		{
			SetValidationError(TEXT("Heal/AddCurrency rewards must not define weapon, effect, or ability set payloads in the current schema."));
			return false;
		}
		return true;

	default:
		SetValidationError(TEXT("Unknown RewardType."));
		return false;
	}
}
