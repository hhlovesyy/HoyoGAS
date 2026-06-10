#include "Cards/SurvivorCardBehavior.h"

#include "AbilitySystemComponent.h"
#include "Cards/SurvivorCardDefinition.h"
#include "Cards/SurvivorCardLoadoutComponent.h"
#include "GameplayEffectTypes.h"

bool USurvivorCardBehavior::OnCardEquipped_Implementation(const FSurvivorCardBehaviorContext& Context, FSurvivorAppliedCardHandles& OutAppliedHandles) const
{
	return true;
}

void USurvivorCardBehavior::OnCardUnequipped_Implementation(const FSurvivorCardBehaviorContext& Context, const FSurvivorAppliedCardHandles& AppliedHandles) const
{
}

void USurvivorCardBehavior::OnRunStarted_Implementation(const FSurvivorCardBehaviorContext& Context) const
{
}

void USurvivorCardBehavior::OnEnemyKilled_Implementation(const FSurvivorCardEnemyKillContext& Context) const
{
}

void USurvivorCardBehavior::OnCardStackChanged_Implementation(const FSurvivorCardBehaviorContext& Context, int32 OldStackCount, int32 NewStackCount) const
{
}

void USurvivorCardBehavior::TickBehavior_Implementation(const FSurvivorCardBehaviorContext& Context, float DeltaSeconds) const
{
}

const FSurvivorCardRuntimeInstance* USurvivorCardBehavior::ResolveRuntimeInstance(const FSurvivorCardBehaviorContext& Context) const
{
	return Context.LoadoutComponent
		? Context.LoadoutComponent->FindCardRuntimeInstanceById(Context.RuntimeCardInstanceId)
		: nullptr;
}

FSurvivorCardRuntimeState* USurvivorCardBehavior::ResolveMutableRuntimeState(const FSurvivorCardBehaviorContext& Context) const
{
	return Context.LoadoutComponent
		? Context.LoadoutComponent->FindMutableCardRuntimeState(Context.RuntimeCardInstanceId)
		: nullptr;
}

USurvivorCardRuntimeData* USurvivorCardBehavior::ResolveRuntimeData(const FSurvivorCardBehaviorContext& Context, TSubclassOf<USurvivorCardRuntimeData> RuntimeDataClass) const
{
	return Context.LoadoutComponent
		? Context.LoadoutComponent->FindOrAddCardRuntimeData(Context.RuntimeCardInstanceId, ResolveRuntimeStateKey(), RuntimeDataClass)
		: nullptr;
}

void USurvivorCardBehavior::ExecuteGameplayCue(
	const FSurvivorCardBehaviorContext& Context,
	UAbilitySystemComponent* SourceASC,
	FGameplayTag GameplayCueTag,
	float RawMagnitude,
	const FGameplayEffectContextHandle* EffectContext) const
{
	if (!SourceASC || !GameplayCueTag.IsValid())
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.RawMagnitude = RawMagnitude;
	CueParameters.SourceObject = static_cast<const UObject*>(Context.CardDefinition.Get());
	CueParameters.Instigator = Context.Pawn ? static_cast<AActor*>(Context.Pawn.Get()) : Context.OwnerActor.Get();
	CueParameters.EffectCauser = Context.OwnerActor.Get();
	CueParameters.Location = Context.OwnerActor ? Context.OwnerActor->GetActorLocation() : FVector::ZeroVector;

	if (EffectContext)
	{
		CueParameters.EffectContext = *EffectContext;
	}

	SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParameters);
}

FName USurvivorCardBehavior::ResolveRuntimeStateKey() const
{
	return RuntimeStateKey.IsNone() ? GetClass()->GetFName() : RuntimeStateKey;
}
