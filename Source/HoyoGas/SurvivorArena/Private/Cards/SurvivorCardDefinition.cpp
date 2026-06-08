#include "Cards/SurvivorCardDefinition.h"

bool USurvivorCardDefinition::ValidateDefinition(FString* OutError) const
{
	if (MaxStack < 1)
	{
		if (OutError)
		{
			*OutError = TEXT("MaxStack must be at least 1.");
		}
		return false;
	}

	if (bUnique && MaxStack != 1)
	{
		if (OutError)
		{
			*OutError = TEXT("Unique cards must use MaxStack = 1.");
		}
		return false;
	}

	return true;
}
