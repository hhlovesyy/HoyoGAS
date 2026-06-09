#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "SurvivorCardRuntimeTypes.generated.h"

class APlayerState;
class APawn;
class UAbilitySystemComponent;
class USurvivorCardDefinition;
class USurvivorCardRuntimeData;
class USurvivorCardLoadoutComponent;

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorAppliedCardHandles
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

	TArray<TWeakObjectPtr<AActor>> SpawnedActors;

	bool IsEmpty() const;
	void Reset();
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCardRuntimeDataEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName StateId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardRuntimeData> RuntimeData = nullptr;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCardRuntimeState
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FSurvivorCardRuntimeDataEntry> RuntimeDataEntries;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCardRuntimeInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	int32 RuntimeInstanceId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardDefinition> CardDefinition = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FSurvivorAppliedCardHandles AppliedHandles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FSurvivorCardRuntimeState RuntimeState;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCardBehaviorContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<APawn> Pawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardLoadoutComponent> LoadoutComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorCardDefinition> CardDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	int32 RuntimeCardInstanceId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	int32 StackCount = 0;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCardEnemyKillContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	FSurvivorCardBehaviorContext CardContext;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<AActor> KilledEnemyActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "SurvivorArena")
	int32 KillCountDelta = 1;
};
