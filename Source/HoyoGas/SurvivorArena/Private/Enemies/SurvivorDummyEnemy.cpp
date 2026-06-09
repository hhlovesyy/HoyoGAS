#include "Enemies/SurvivorDummyEnemy.h"

#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorArenaSettings.h"
#include "Data/SurvivorArenaTypes.h"
#include "DrawDebugHelpers.h"
#include "Engine/DataTable.h"
#include "GAS/SurvivorAbilitySystemComponent.h"
#include "GAS/SurvivorAttributeSet.h"
#include "Pickups/SurvivorPickupActor.h"
#include "Pickups/SurvivorPickupDefinition.h"
#include "Pooling/SurvivorActorPoolSubsystem.h"
#include "TimerManager.h"

ASurvivorDummyEnemy::ASurvivorDummyEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot); //一般来说这种Actor的根节点都是一个SceneComponent

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //关闭物理模拟（Physics），只开启查询（Query）。极其关键的性能优化。QueryOnly 意味着这个网格体只参与射线检测（Line Trace）和重叠判定（Overlap），它不会受重力影响掉下悬崖，也不会被撞飞。引擎不需要为它计算复杂的物理碰撞解算。
	
	//以下几句的含义：含义：允许触发重叠事件；将自己在碰撞矩阵中注册为 Pawn（通常代表存活实体）；默认对所有其他通道的对象（墙壁、子弹、玩家）都采取“重叠（Overlap）”响应，而不是“阻挡（Block）”。
	//重点：这是为了配合 GAS 的 GameplayAbility_TargetActor 或碰撞体的 OnComponentBeginOverlap 逻辑。比如玩家的剑气扫过来（一个弹体），剑气穿过这个 Mesh 触发 Overlap，随后给它贴上一个 Damage Gameplay Effect（GE）。
	VisualMesh->SetGenerateOverlapEvents(true);
	VisualMesh->SetCollisionObjectType(ECC_Pawn);
	VisualMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	/*
	*GAS 极其吃网络带宽。UE 官方为 ASC 提供了三种同步模式：
		Full：同步所有 GE 和 Gameplay Tags。通常只用于单机游戏。
		Mixed：同步 Tags，但只向拥有者同步 GE。通常用于玩家自己（Player Character）。
		Minimal：只向客户端同步 Gameplay Tags，完全不同步 GE 的具体数据。这是专门为 AI/敌人准备的模式。 客户端只需要知道这个怪物身上有没有“被眩晕”的 Tag 从而播放特效，根本不需要知道这个眩晕 GE 的持续时间、是谁施放的等底层数据。
	 */
	AbilitySystemComponent = CreateDefaultSubobject<USurvivorAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal); //开启网络同步，并将同步模式设置为 Minimal（极简模式）。
	
	//如果玩家和怪物用的都是 USurvivorAttributeSet，你只需要写一遍伤害计算逻辑。玩家打怪，或者怪打玩家，底层走的是完全一样的数据流。如果不共用，你还得去判断对方到底是什么类型，然后再去取不同的 Set，代码会非常臃肿。
	//如果未来有些高级怪物需要蓝条，有些不需要。你不需要去修改继承关系。你只需要把 USurvivorAttributeSet 拆成 UHealthAttributeSet 和 UManaAttributeSet。
	//普通小怪只挂载 Health 模块，玩家和 Boss 挂载 Health 和 Mana 两个模块即可。ASC 会自动汇总它们。
	AttributeSet = CreateDefaultSubobject<USurvivorAttributeSet>(TEXT("AttributeSet"));
}

void ASurvivorDummyEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USurvivorAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ASurvivorDummyEnemy::HandleHealthChanged);
	}

	if (VisualMesh)
	{
		VisualMeshDefaultScale = VisualMesh->GetRelativeScale3D();
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("DummyEnemy BeginPlay. Enemy=%s Health=%.2f"), *GetNameSafe(this), AttributeSet ? AttributeSet->GetHealth() : 0.0f);
}

UAbilitySystemComponent* ASurvivorDummyEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASurvivorDummyEnemy::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	UE_LOG(LogSurvivorArena, Log, TEXT("DummyEnemy health changed. Enemy=%s Old=%.2f New=%.2f"), *GetNameSafe(this), ChangeData.OldValue, ChangeData.NewValue);

	if (ChangeData.NewValue < ChangeData.OldValue)
	{
		PlayHitFeedback();
	}

	if (bDrawDebugHealth)
	{
		DrawHealthDebug(ChangeData.NewValue);
	}

	if (!bIsDead && ChangeData.NewValue <= 0.0f)
	{
		Die();
	}
}

void ASurvivorDummyEnemy::PlayHitFeedback()
{
	if (!bEnableHitFeedback || !VisualMesh)
	{
		return;
	}

	VisualMesh->SetRelativeScale3D(VisualMeshDefaultScale * HitFeedbackScaleMultiplier);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitFeedbackTimerHandle);
		World->GetTimerManager().SetTimer(HitFeedbackTimerHandle, this, &ASurvivorDummyEnemy::ResetHitFeedback, HitFeedbackDuration, false);
	}
}

void ASurvivorDummyEnemy::ResetHitFeedback()
{
	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(VisualMeshDefaultScale);
	}
}

void ASurvivorDummyEnemy::DrawHealthDebug(float CurrentHealth) const
{
	if (!GetWorld())
	{
		return;
	}

	DrawDebugString(
		GetWorld(),
		GetActorLocation() + FVector(0.0f, 0.0f, 120.0f),
		FString::Printf(TEXT("HP: %.1f"), CurrentHealth),
		const_cast<ASurvivorDummyEnemy*>(this),
		FColor::White,
		1.5f,
		false);
}

const FSurvivorEnemyDefinitionRow* ASurvivorDummyEnemy::ResolveEnemyDefinition() const
{
	if (EnemyDefinitionId.IsNone())
	{
		return nullptr;
	}

	const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
	UDataTable* EnemyDefinitionTable = Settings ? Settings->EnemyDefinitionTable.LoadSynchronous() : nullptr;
	if (!EnemyDefinitionTable)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("DummyEnemy %s could not resolve enemy definition because EnemyDefinitionTable is null."), *GetNameSafe(this));
		return nullptr;
	}

	return EnemyDefinitionTable->FindRow<FSurvivorEnemyDefinitionRow>(EnemyDefinitionId, TEXT("ASurvivorDummyEnemy::ResolveEnemyDefinition"));
}

void ASurvivorDummyEnemy::SpawnConfiguredDrops()
{
	const FSurvivorEnemyDefinitionRow* EnemyDefinition = ResolveEnemyDefinition();
	if (!EnemyDefinition || EnemyDefinition->PickupDrops.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USurvivorActorPoolSubsystem* PoolSubsystem = World->GetSubsystem<USurvivorActorPoolSubsystem>();
	FRandomStream RandomStream(FMath::Rand());

	for (const FSurvivorPickupDropEntry& DropEntry : EnemyDefinition->PickupDrops)
	{
		if (DropEntry.PickupDefinition.IsNull())
		{
			continue;
		}

		if (RandomStream.FRand() > DropEntry.DropChance)
		{
			continue;
		}

		USurvivorPickupDefinition* PickupDefinition = DropEntry.PickupDefinition.LoadSynchronous();
		if (!PickupDefinition)
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("DummyEnemy %s failed to load pickup definition for drop entry."), *GetNameSafe(this));
			continue;
		}

		FString ValidationError;
		if (!PickupDefinition->ValidateDefinition(&ValidationError))
		{
			UE_LOG(LogSurvivorArena, Warning, TEXT("DummyEnemy %s pickup definition '%s' is invalid: %s"), *GetNameSafe(this), *GetNameSafe(PickupDefinition), *ValidationError);
			continue;
		}

		const int32 SpawnCount = FMath::Max(DropEntry.MinCount, DropEntry.MaxCount > DropEntry.MinCount ? RandomStream.RandRange(DropEntry.MinCount, DropEntry.MaxCount) : DropEntry.MinCount);
		for (int32 SpawnIndex = 0; SpawnIndex < SpawnCount; ++SpawnIndex)
		{
			const FVector RandomOffset = FVector(RandomStream.FRandRange(-DropEntry.SpawnRadius, DropEntry.SpawnRadius), RandomStream.FRandRange(-DropEntry.SpawnRadius, DropEntry.SpawnRadius), 0.0f);
			const FTransform SpawnTransform(GetActorTransform().GetRotation(), GetActorLocation() + RandomOffset, FVector::OneVector);
			AActor* SpawnedActor = PoolSubsystem
				? PoolSubsystem->AcquireActor(PickupDefinition->PickupActorClass, SpawnTransform)
				: World->SpawnActor<AActor>(PickupDefinition->PickupActorClass, SpawnTransform);

			if (ASurvivorPickupActor* PickupActor = Cast<ASurvivorPickupActor>(SpawnedActor))
			{
				PickupActor->InitializePickup(PickupDefinition, PickupDefinition->BaseValue);
			}
		}
	}
}

void ASurvivorDummyEnemy::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	ResetHitFeedback();
	SpawnConfiguredDrops();
	UE_LOG(LogSurvivorArena, Log, TEXT("DummyEnemy died. Enemy=%s"), *GetNameSafe(this));
	Destroy();
}
