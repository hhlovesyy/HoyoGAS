#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElementReaction/ElementTypes.h"
#include "PetCompanionActor.generated.h"

class USkeletalMeshComponent;
class AElementDummyTarget;
class UWidgetComponent;

UCLASS()
class HOYOGAS_API APetCompanionActor : public AActor
{
	GENERATED_BODY()

public:
	APetCompanionActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ===== Components =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> PetMesh;

	// 以后你要做头顶图标时可以用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> ElementIconWidget;

	// ===== Runtime =====
	UPROPERTY()
	TObjectPtr<AElementDummyTarget> DebugTarget;

	FTimerHandle AttackTimerHandle;
	FTimerHandle RandomElementTimerHandle;

	// 用来做最简单的原地浮动
	float RunningTime = 0.f;
	FVector InitialLocation = FVector::ZeroVector;

protected:
	// ===== Basic Config =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	EGenshinElementType CurrentElement = EGenshinElementType::Pyro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	float AttackInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	float BaseDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	float ElementalMastery = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	int32 CharacterLevel = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	bool bAutoAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	bool bRandomizeElement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Combat")
	float RandomElementInterval = 3.0f;

	// ===== Idle Float / Q弹最小版 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Idle")
	float FloatAmplitude = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Idle")
	float FloatSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet|Idle")
	float ScaleAmplitude = 0.03f;

protected:
	// ===== Main Logic =====
	void FindDebugTarget();
	void StartAutoAttack();
	void StartRandomElementLoop();

	UFUNCTION(BlueprintCallable, Category = "Pet|Combat")
	void FireElementAtTarget();

	UFUNCTION(BlueprintCallable, Category = "Pet|Combat")
	void RandomizeElement();

	// 以后你做 UI 图标时在这里刷新
	UFUNCTION(BlueprintCallable, Category = "Pet|UI")
	void RefreshElementIcon();

	// 以后做动画时可以在蓝图里重写
	UFUNCTION(BlueprintImplementableEvent, Category = "Pet|Animation")
	void BP_PlayAttackAnim();

	// 以后做发光、图标、颜色变化
	UFUNCTION(BlueprintImplementableEvent, Category = "Pet|Visual")
	void BP_OnElementChanged(EGenshinElementType NewElement);

	// 如果你以后要让动画 Notify 真正触发伤害，可以调这个
	UFUNCTION(BlueprintCallable, Category = "Pet|Combat")
	void ExecuteAttackHit();
};