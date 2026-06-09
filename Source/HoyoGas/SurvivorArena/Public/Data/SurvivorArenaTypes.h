#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SurvivorArenaTypes.generated.h"

class APawn;
class AActor;
class UGameplayEffect;
class UTexture2D;
class USurvivorPickupDefinition;
class USurvivorAbilitySet;
class USurvivorWeaponDefinition;

UENUM(BlueprintType)
enum class ESurvivorRunState : uint8
{
	None,
	CharacterSelect,
	Preparing,
	InRun,
	LevelUpSelecting,
	Shop,
	Paused,
	Victory,
	Defeat,
	ReturningToHub
};

UENUM(BlueprintType)
enum class ESurvivorRewardType : uint8
{
	None,
	AttributeModifier,
	GrantAbilitySet,
	AddWeapon,
	UpgradeWeapon,
	Heal,
	AddCurrency
};

UENUM(BlueprintType)
enum class ESurvivorPickupType : uint8
{
	None,
	Coin,
	Experience
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorRunStartConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	FName CharacterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	FName LevelId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	bool bUseFixedSeed = false;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorCharacterDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName CharacterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<TSoftObjectPtr<USurvivorWeaponDefinition>> StartingWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<TObjectPtr<USurvivorAbilitySet>> StartingAbilitySets;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorRewardDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName RewardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	ESurvivorRewardType RewardType = ESurvivorRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName TargetWeaponId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSubclassOf<UGameplayEffect> EffectToApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USurvivorAbilitySet> AbilitySetToGrant = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0"))
	int32 Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0"))
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0"))
	int32 ShopPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FGameplayTagContainer BlockedTags;

	// Reward payloads can intentionally overlap, but RewardType must remain the
	// canonical source of truth for legal combinations.
	bool ValidateRewardDefinition(FString* OutValidationError = nullptr) const;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorPickupDropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSoftObjectPtr<USurvivorPickupDefinition> PickupDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0"))
	int32 MaxCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena", meta = (ClampMin = "0.0"))
	float SpawnRadius = 50.0f;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorEnemyDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName EnemyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	float MaxHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	float MoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	float ContactDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	int32 XPDrop = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	int32 CoinDrop = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FSurvivorPickupDropEntry> PickupDrops;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FGameplayTagContainer EnemyTags;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorEnemySpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	FName EnemyId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	float SpawnRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	int32 MaxAliveCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	float SpawnDistanceMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SurvivorArena")
	float SpawnDistanceMax = 0.0f;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FSurvivorWaveDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FName WaveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TArray<FSurvivorEnemySpawnEntry> SpawnEntries;
};
