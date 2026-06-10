#include "Cards/SurvivorCardBehaviorTomoriNotes.h"

#include "AbilitySystemComponent.h"
#include "Cards/SurvivorCardDefinition.h"
#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorGameplayTags.h"
#include "Data/SurvivorArenaTypes.h"
#include "Enemies/SurvivorDummyEnemy.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GAS/SurvivorAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

void USurvivorCardBehaviorTomoriNotes::OnEnemyKilled_Implementation(const FSurvivorCardEnemyKillContext& Context) const
{
	USurvivorCardRuntimeDataTomoriNotes* RuntimeData = Cast<USurvivorCardRuntimeDataTomoriNotes>(
		ResolveRuntimeData(Context.CardContext, USurvivorCardRuntimeDataTomoriNotes::StaticClass()));
	if (!RuntimeData || KillsPerNote <= 0 || NotesPerBurst <= 0)
	{
		return;
	}

	RuntimeData->KillCounter += FMath::Max(0, Context.KillCountDelta);

	while (RuntimeData->KillCounter >= KillsPerNote)
	{
		RuntimeData->KillCounter -= KillsPerNote;
		++RuntimeData->NoteCount;
	}

	while (RuntimeData->NoteCount >= NotesPerBurst)
	{
		RuntimeData->NoteCount -= NotesPerBurst;
		TriggerPoemBurst(Context.CardContext);
	}
}

void USurvivorCardBehaviorTomoriNotes::TriggerPoemBurst(const FSurvivorCardBehaviorContext& Context) const
{
	UWorld* World = Context.OwnerActor ? Context.OwnerActor->GetWorld() : nullptr;
	UAbilitySystemComponent* SourceASC = Context.AbilitySystemComponent.Get();
	if (!World || !SourceASC || !BurstDamageGameplayEffect || BurstDamage <= 0.0f)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Tomori Poem Burst skipped. World=%s SourceASC=%s DamageGE=%s BurstDamage=%.2f"),
			World ? TEXT("valid") : TEXT("null"),
			*GetNameSafe(SourceASC),
			*GetNameSafe(BurstDamageGameplayEffect),
			BurstDamage);
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(Context.CardDefinition.Get());

	const FGameplayEffectSpecHandle DamageSpecHandle = SourceASC->MakeOutgoingSpec(BurstDamageGameplayEffect, 1.0f, EffectContext);
	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Tomori Poem Burst failed to create damage spec. Owner=%s DamageGE=%s"),
			*GetNameSafe(Context.OwnerActor),
			*GetNameSafe(BurstDamageGameplayEffect));
		return;
	}

	const FGameplayTag DamageSetByCallerTag = ResolveBurstDamageSetByCallerTag();
	if (DamageSetByCallerTag.IsValid())
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, -BurstDamage);
	}

	int32 DamagedEnemyCount = 0;
	for (TActorIterator<ASurvivorDummyEnemy> It(World); It; ++It)
	{
		ASurvivorDummyEnemy* TargetEnemy = *It;
		if (!TargetEnemy || TargetEnemy == Context.OwnerActor || !CanAffectEnemy(*TargetEnemy))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = TargetEnemy->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			continue;
		}

		const float CurrentHealth = TargetASC->GetNumericAttribute(USurvivorAttributeSet::GetHealthAttribute());
		if (CurrentHealth <= 0.0f)
		{
			continue;
		}

		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
		++DamagedEnemyCount;
	}

	if (DamagedEnemyCount > 0 && PoemBurstGameplayCueTag.IsValid())
	{
		ExecuteGameplayCue(Context, SourceASC, PoemBurstGameplayCueTag, BurstDamage, &EffectContext);
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("Tomori Poem Burst triggered. Owner=%s DamageGE=%s Damage=%.2f DamagedTargets=%d"),
		*GetNameSafe(Context.OwnerActor),
		*GetNameSafe(BurstDamageGameplayEffect),
		BurstDamage,
		DamagedEnemyCount);

	if (bShowDebugMessage && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Cyan,
			FString::Printf(TEXT("Tomori Poem Burst! %d targets hit"), DamagedEnemyCount));
	}
}

bool USurvivorCardBehaviorTomoriNotes::CanAffectEnemy(const ASurvivorDummyEnemy& TargetEnemy) const
{
	const FSurvivorEnemyDefinitionRow* EnemyDefinition = TargetEnemy.GetEnemyDefinition();
	if (!EnemyDefinition)
	{
		return false;
	}

	const FGameplayTagContainer& EnemyTags = EnemyDefinition->EnemyTags;
	if (!RequiredEnemyTags.IsEmpty() && !EnemyTags.HasAll(RequiredEnemyTags))
	{
		return false;
	}

	return BlockedEnemyTags.IsEmpty() || !EnemyTags.HasAny(BlockedEnemyTags);
}

FGameplayTag USurvivorCardBehaviorTomoriNotes::ResolveBurstDamageSetByCallerTag() const
{
	return BurstDamageSetByCallerTag.IsValid()
		? BurstDamageSetByCallerTag
		: SurvivorGameplayTags::Survivor_Damage_SetByCaller;
}
