#include "Weapons/SurvivorWeaponGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Player/SurvivorCharacter.h"
#include "TimerManager.h"
#include "Weapons/SurvivorFirePattern.h"
#include "Weapons/SurvivorProjectileBase.h"
#include "Weapons/SurvivorWeaponDefinition.h"

USurvivorWeaponGameplayAbility::USurvivorWeaponGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void USurvivorWeaponGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	USurvivorWeaponDefinition* WeaponDefinition = ResolveWeaponDefinition();
	if (!ValidateWeaponDefinition(WeaponDefinition))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Weapon ability activation failed because World is null. Ability=%s"), *GetNameSafe(this));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("Weapon ability activated. Ability=%s WeaponId=%s Owner=%s"),
		*GetNameSafe(this),
		*WeaponDefinition->WeaponId.ToString(),
		*GetNameSafe(GetAvatarActorFromActorInfo()));

	FireWeaponOnce();

	World->GetTimerManager().SetTimer(
		AutoFireTimerHandle,
		this,
		&USurvivorWeaponGameplayAbility::HandleAutoFireTick,
		WeaponDefinition->FireInterval,
		true);
}

void USurvivorWeaponGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USurvivorWeaponGameplayAbility::HandleAutoFireTick()
{
	FireWeaponOnce();
}

bool USurvivorWeaponGameplayAbility::ValidateWeaponDefinition(const USurvivorWeaponDefinition* WeaponDefinition) const
{
	if (!WeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Weapon ability failed because SourceObject is not a SurvivorWeaponDefinition. Ability=%s"), *GetNameSafe(this));
		return false;
	}

	FString ValidationError;
	if (!WeaponDefinition->ValidateRuntimeConfiguration(&ValidationError))
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Weapon ability failed validation. Ability=%s Weapon=%s Error=%s"),
			*GetNameSafe(this),
			*GetNameSafe(WeaponDefinition),
			*ValidationError);
		return false;
	}

	return true;
}

USurvivorWeaponDefinition* USurvivorWeaponGameplayAbility::ResolveWeaponDefinition() const
{
	return Cast<USurvivorWeaponDefinition>(GetCurrentSourceObject());
}

FSurvivorFirePatternContext USurvivorWeaponGameplayAbility::BuildFirePatternContext(AActor* AvatarActor) const
{
	FSurvivorFirePatternContext PatternContext;
	PatternContext.OwnerActor = AvatarActor;
	PatternContext.Origin = ResolveFireOrigin(AvatarActor);
	PatternContext.ForwardDirection = AvatarActor ? AvatarActor->GetActorForwardVector() : FVector::ForwardVector;
	PatternContext.ForwardDirection.Z = 0.0f;
	if (!PatternContext.ForwardDirection.Normalize())
	{
		PatternContext.ForwardDirection = FVector::ForwardVector;
	}

	return PatternContext;
}

FVector USurvivorWeaponGameplayAbility::ResolveFireOrigin(AActor* AvatarActor) const
{
	if (const ASurvivorCharacter* SurvivorCharacter = Cast<ASurvivorCharacter>(AvatarActor))
	{
		if (const USkeletalMeshComponent* WeaponMesh = SurvivorCharacter->GetWeaponMesh())
		{
			return WeaponMesh->GetComponentLocation();
		}
	}

	return AvatarActor ? AvatarActor->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f) : FVector::ZeroVector;
}

void USurvivorWeaponGameplayAbility::FireWeaponOnce()
{
	USurvivorWeaponDefinition* WeaponDefinition = ResolveWeaponDefinition();
	if (!ValidateWeaponDefinition(WeaponDefinition))
	{
		if (GetCurrentActorInfo())
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Weapon fire failed because AvatarActor is null. Ability=%s WeaponId=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString());
		return;
	}

	const FSurvivorFirePatternContext PatternContext = BuildFirePatternContext(AvatarActor);
	TArray<FVector> Directions;
	WeaponDefinition->FirePattern->GenerateDirections(PatternContext, Directions);
	if (Directions.Num() == 0)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Weapon fire produced no directions. Ability=%s WeaponId=%s FirePattern=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString(),
			*GetNameSafe(WeaponDefinition->FirePattern));
		return;
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("Weapon fire. Ability=%s WeaponId=%s DirectionCount=%d"),
		*GetNameSafe(this),
		*WeaponDefinition->WeaponId.ToString(),
		Directions.Num());

	for (const FVector& Direction : Directions)
	{
		SpawnProjectileForDirection(WeaponDefinition, AvatarActor, PatternContext.Origin, Direction);
	}
}

void USurvivorWeaponGameplayAbility::SpawnProjectileForDirection(
	const USurvivorWeaponDefinition* WeaponDefinition,
	AActor* AvatarActor,
	const FVector& SpawnOrigin,
	const FVector& Direction)
{
	if (!WeaponDefinition || !AvatarActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!World || !SourceASC)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile spawn failed because World or SourceASC is null. Ability=%s WeaponId=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString());
		return;
	}

	const FGameplayEffectSpecHandle DamageSpecHandle =
		MakeOutgoingGameplayEffectSpec(WeaponDefinition->DamageGameplayEffect, static_cast<float>(GetAbilityLevel()));
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile spawn failed because DamageSpecHandle is invalid. Ability=%s WeaponId=%s GE=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString(),
			*GetNameSafe(WeaponDefinition->DamageGameplayEffect));
		return;
	}

	if (SpawnOrigin.ContainsNaN())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile spawn failed because SpawnOrigin is invalid. Ability=%s WeaponId=%s SpawnOrigin=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString(),
			*SpawnOrigin.ToString());
		return;
	}

	FVector SafeDirection = Direction;
	SafeDirection.Z = 0.0f;
	if (!SafeDirection.Normalize())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile spawn failed because direction is invalid. Ability=%s WeaponId=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString());
		return;
	}

	const FTransform SpawnTransform(SafeDirection.Rotation(), SpawnOrigin);
	ASurvivorProjectileBase* Projectile = World->SpawnActorDeferred<ASurvivorProjectileBase>(
		WeaponDefinition->ProjectileClass,
		SpawnTransform,
		AvatarActor,
		Cast<APawn>(AvatarActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("Projectile spawn failed. Ability=%s WeaponId=%s ProjectileClass=%s SpawnOrigin=%s Direction=%s"),
			*GetNameSafe(this),
			*WeaponDefinition->WeaponId.ToString(),
			*GetNameSafe(WeaponDefinition->ProjectileClass),
			*SpawnOrigin.ToString(),
			*SafeDirection.ToString());
		return;
	}

	Projectile->SetActorEnableCollision(false);
	Projectile->FinishSpawning(SpawnTransform);
	Projectile->InitializeProjectile(
		SourceASC,
		DamageSpecHandle,
		SafeDirection,
		WeaponDefinition->ProjectileSpeed,
		WeaponDefinition->ProjectileLifeSeconds);
	Projectile->SetActorEnableCollision(true);
}
