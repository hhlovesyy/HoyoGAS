#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "SurvivorProjectileBase.generated.h"

class UAbilitySystemComponent;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USceneComponent;
//理解这个类，你就掌握了在这类游戏中实现诸如“穿透箭”、“爆炸火球”或“反弹飞镖”的底层逻辑架构。

UCLASS()
class HOYOGAS_API ASurvivorProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ASurvivorProjectileBase();

	UFUNCTION(BlueprintCallable, Category = "Survivor|Projectile")
	void SetPrimaryCollisionComponent(UPrimitiveComponent* InComponent);

	UFUNCTION(BlueprintCallable, Category = "Survivor|Projectile")
	virtual void InitializeProjectile(
		UAbilitySystemComponent* InSourceASC,
		const FGameplayEffectSpecHandle& InDamageSpecHandle,
		const FVector& Direction,
		float Speed,
		float LifeSeconds);

	UFUNCTION(BlueprintPure, Category = "Survivor|Projectile")
	UPrimitiveComponent* GetPrimaryCollisionComponent() const;

	UFUNCTION(BlueprintPure, Category = "Survivor|Projectile")
	UProjectileMovementComponent* GetProjectileMovementComponent() const;

protected:
	// Default line-flight uses ProjectileMovementComponent.
	// Homing / bezier / custom spline movement should be added either in subclasses
	// or via a future movement policy object without changing the damage carrier contract here.
	//UE5 的 ProjectileMovementComponent 非常强大（能做抛物线、追踪导弹），但对于幸存者游戏里那些极其诡异的飞行轨迹（比如：回旋镖绕着玩家转圈、闪电链折射）它是无能为力的。
	//如果未来要面对更复杂的 3D 动作游戏需求，你的这套架构可以怎样演进？保留这套 FGameplayEffectSpecHandle 的快递合同，但把运动逻辑抽离成一个 UProjectileMovementPolicy。
	virtual void HandleProjectileOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Impact stays virtual so piercing / splitting / explosive projectiles can override hit behavior.
	// bAllowMultipleHits only controls per-target dedupe; it does not define piercing by itself.
	// A piercing projectile would usually set bDestroyOnHit = false and keep bAllowMultipleHits = false
	// so each target is damaged once while the projectile continues through the scene.
	virtual void HandleProjectileImpact(AActor* HitActor, const FHitResult& Hit);
	virtual bool CanHitActor(AActor* OtherActor) const;
	virtual bool ApplyDamageSpecToActor(AActor* TargetActor);

	UFUNCTION()
	void OnPrimaryCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survivor|Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPrimitiveComponent> PrimaryCollisionComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Survivor|Projectile")
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY(BlueprintReadOnly, Category = "Survivor|Projectile")
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	TSet<TWeakObjectPtr<AActor>> HitActors;

	UPROPERTY(Transient)
	bool bProjectileInitialized = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Projectile")
	bool bDestroyOnHit = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Projectile")
	bool bAllowMultipleHits = false;
};
