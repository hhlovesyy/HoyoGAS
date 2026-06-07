#include "Weapons/SurvivorWeaponDefinition.h"

bool USurvivorWeaponDefinition::ValidateRuntimeConfiguration(FString* OutValidationError) const
{
	auto SetValidationError = [OutValidationError](const FString& Message)
	{
		if (OutValidationError)
		{
			*OutValidationError = Message;
		}
	};

	if (WeaponId.IsNone())
	{
		SetValidationError(TEXT("WeaponId is required."));
		return false;
	}

	if (!WeaponAbility)
	{
		SetValidationError(TEXT("WeaponAbility is required."));
		return false;
	}

	if (!DamageGameplayEffect)
	{
		SetValidationError(TEXT("DamageGameplayEffect is required."));
		return false;
	}

	if (!ProjectileClass)
	{
		SetValidationError(TEXT("ProjectileClass is required."));
		return false;
	}

	if (!FirePattern)
	{
		SetValidationError(TEXT("FirePattern is required."));
		return false;
	}

	if (FireInterval <= 0.0f)
	{
		SetValidationError(TEXT("FireInterval must be > 0."));
		return false;
	}

	if (ProjectileSpeed <= 0.0f)
	{
		SetValidationError(TEXT("ProjectileSpeed must be > 0."));
		return false;
	}

	if (ProjectileLifeSeconds <= 0.0f)
	{
		SetValidationError(TEXT("ProjectileLifeSeconds must be > 0."));
		return false;
	}

	return true;
}
