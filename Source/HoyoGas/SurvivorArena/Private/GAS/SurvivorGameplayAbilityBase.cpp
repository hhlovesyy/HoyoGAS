#include "GAS/SurvivorGameplayAbilityBase.h"

#include "GAS/SurvivorAbilitySystemComponent.h"

USurvivorAbilitySystemComponent* USurvivorGameplayAbilityBase::GetSurvivorASCFromActorInfo() const
{
	return Cast<USurvivorAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}
