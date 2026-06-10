#pragma once

#include "CoreMinimal.h"
#include "Cards/SurvivorCardBehavior.h"
#include "GameplayTagContainer.h"
#include "SurvivorCardBehaviorTomoriNotes.generated.h"

class UGameplayEffect;
class USurvivorCardDefinition;

UCLASS()
class HOYOGAS_API USurvivorCardRuntimeDataTomoriNotes : public USurvivorCardRuntimeData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	int32 KillCounter = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	int32 NoteCount = 0;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class HOYOGAS_API USurvivorCardBehaviorTomoriNotes : public USurvivorCardBehavior
{
	GENERATED_BODY()

public:
	virtual void OnEnemyKilled_Implementation(const FSurvivorCardEnemyKillContext& Context) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori", meta = (ClampMin = "1"))
	int32 KillsPerNote = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori", meta = (ClampMin = "1"))
	int32 NotesPerBurst = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori", meta = (ClampMin = "0.0"))
	float BurstDamage = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	TSubclassOf<UGameplayEffect> BurstDamageGameplayEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	FGameplayTag BurstDamageSetByCallerTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	FGameplayTag PoemBurstGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	FGameplayTagContainer RequiredEnemyTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	FGameplayTagContainer BlockedEnemyTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Cards|Tomori")
	bool bShowDebugMessage = true;

	void TriggerPoemBurst(const FSurvivorCardBehaviorContext& Context) const;
	bool CanAffectEnemy(const class ASurvivorDummyEnemy& TargetEnemy) const;
	FGameplayTag ResolveBurstDamageSetByCallerTag() const;
};
