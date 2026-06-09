#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SurvivorDummyEnemy.generated.h"

class UAbilitySystemComponent;
class UStaticMeshComponent;
class USceneComponent;
class USurvivorAbilitySystemComponent;
class USurvivorAttributeSet;
struct FSurvivorEnemyDefinitionRow;

UCLASS()
class HOYOGAS_API ASurvivorDummyEnemy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASurvivorDummyEnemy();

	virtual void BeginPlay() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	void PlayHitFeedback();
	void ResetHitFeedback();
	void DrawHealthDebug(float CurrentHealth) const;
	void SpawnConfiguredDrops();
	const FSurvivorEnemyDefinitionRow* ResolveEnemyDefinition() const;
	void Die();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Enemy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot; //SceneComponent一般有Transform，适合那种有实体的，也有层级关系，ActorComponent更多是逻辑相关的

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Enemy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMesh; //像这个继承链里就是SceneComponent，注意这里用的是 UStaticMeshComponent（静态网格体）而不是带骨骼的 USkeletalMeshComponent。在《土豆兄弟》或《吸血鬼幸存者》这种俯视角游戏中，很多低级小怪如果不需要复杂的融合动画，直接用顶点的 World Position Offset（WPO）做形变，或者用静态网格体配合简单的移动，能极大地节省 CPU 动画结算开销。

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Enemy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USurvivorAbilitySystemComponent> AbilitySystemComponent; //这个就是ActorComponent

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Enemy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USurvivorAttributeSet> AttributeSet; //暂时先不用把角色和怪物的AttributeSet拆成两份，后面属性有比较大的差异的时候才需要再拆

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Enemy")
	bool bDrawDebugHealth = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Enemy")
	bool bEnableHitFeedback = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Enemy")
	float HitFeedbackDuration = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Enemy")
	float HitFeedbackScaleMultiplier = 1.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survivor|Enemy")
	FName EnemyDefinitionId = NAME_None;

	UPROPERTY(Transient)
	bool bIsDead = false;

	UPROPERTY(Transient)
	FVector VisualMeshDefaultScale = FVector(1.0f, 1.0f, 1.0f);

	FDelegateHandle HealthChangedDelegateHandle;
	FTimerHandle HitFeedbackTimerHandle;
};
