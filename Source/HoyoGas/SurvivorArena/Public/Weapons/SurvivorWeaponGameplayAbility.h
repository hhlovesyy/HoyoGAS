#pragma once

#include "CoreMinimal.h"
#include "GAS/SurvivorGameplayAbilityBase.h"
#include "SurvivorWeaponGameplayAbility.generated.h"

struct FSurvivorFirePatternContext;
class ASurvivorProjectileBase;
class USurvivorWeaponDefinition;

UCLASS()
class HOYOGAS_API USurvivorWeaponGameplayAbility : public USurvivorGameplayAbilityBase
{
	GENERATED_BODY()

public:
	USurvivorWeaponGameplayAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	void HandleAutoFireTick();
	bool ValidateWeaponDefinition(const USurvivorWeaponDefinition* WeaponDefinition) const;
	USurvivorWeaponDefinition* ResolveWeaponDefinition() const;
	FSurvivorFirePatternContext BuildFirePatternContext(AActor* AvatarActor) const;
	FVector ResolveFireOrigin(AActor* AvatarActor) const;
	void FireWeaponOnce();
	void SpawnProjectileForDirection(
		const USurvivorWeaponDefinition* WeaponDefinition,
		AActor* AvatarActor,
		const FVector& SpawnOrigin,
		const FVector& Direction);

	FTimerHandle AutoFireTimerHandle;
};
