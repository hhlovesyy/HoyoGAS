#include "Pickups/SurvivorPickupDefinition.h"

bool USurvivorPickupDefinition::ValidateDefinition(FString* OutError) const
{
	if (PickupType == ESurvivorPickupType::None)
	{
		if (OutError)
		{
			*OutError = TEXT("PickupType is None.");
		}
		return false;
	}

	if (!PickupActorClass)
	{
		if (OutError)
		{
			*OutError = TEXT("PickupActorClass is null.");
		}
		return false;
	}

	return true;
}
