#include "Weapons/SurvivorProjectileBase.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASurvivorProjectileBase::ASurvivorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	//UProjectileMovementComponent (简称 PMC) 是虚幻引擎中专门用于处理非角色类、具有弹道特性的物体移动的内置组件。它继承自 UMovementComponent
	//简单来说，如果 CharacterMovementComponent 是专门用来伺候“长着两条腿、受输入控制的活人”的，那么 ProjectileMovementComponent 就是专门用来伺候“被扔出去、靠惯性和物理规则飞行的死物”的。
	//PMC 在底层最大的价值在于它封装了极其严谨的移动与碰撞清算逻辑（SafeMoveUpdatedComponent）。
	//当子弹每一帧往前推进时，PMC 会带着你注册的那个真实的 Collision Component 去做物理扫模（Sweep）。一旦在这一帧的移动路线上碰到了东西，它会自动计算出击中点的法线（Normal）、剩余的飞行时间，并精准地停在物体表面，从而触发 OnComponentHit 或 OnComponentBeginOverlap，绝对不会嵌进墙里。
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 0.0f;
	ProjectileMovementComponent->MaxSpeed = 0.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bAutoActivate = false;
	ProjectileMovementComponent->SetUpdatedComponent(SceneRoot);
}

void ASurvivorProjectileBase::SetPrimaryCollisionComponent(UPrimitiveComponent* InComponent)
{
	//这个基类不强制创建任何特定的碰撞体，而是把 SceneRoot 作为最外层根节点。它暴露了 SetPrimaryCollisionComponent，允许在蓝图里随意添加碰撞体（比如加个 Box），然后调用这个节点把它注册为主要的“碰撞感应器”。
	//它还会自动帮你处理事件绑定的“擦屁股”工作（移除旧的，绑定新的），并且动态把 ProjectileMovementComponent 的更新目标转移到新的碰撞体上。
	//SetUpdatedComponent：在 UE5 中，UProjectileMovementComponent (简称 PMC) 其实并不直接移动 AActor 本身。它像是一台“独立的外挂发动机”，你需要明确告诉这台发动机：“你应该推着哪个车架子走？”——这个被推的车架子，就是 UpdatedComponent。
	//默认情况下，PMC 会推着 Actor 的根组件 (RootComponent，在这里就是那个没有任何体积的数学点 SceneRoot) 移动。但这样做会引发一个极其致命的 Bug：当 PMC 推着 SceneRoot 进行高速移动时，底层的物理扫模（Sweep）会以 SceneRoot 的形状（一个体积为 0 的点）去进行碰撞检测。这会导致子弹极易穿透薄墙（这就是著名的 Tunneling 穿模效应），哪怕子弹的子层级挂着一个巨大的 Box 碰撞体，因为物理引擎在 Sweep 时根本不看子组件的体积。
	//所以，ProjectileMovementComponent->SetUpdatedComponent(PrimaryCollisionComponent); 这句话的意思是：“把发动机的受力点，从没有体积的根节点，转移到真正的碰撞体身上。” 这样，物理引擎就会用这个碰撞体的实际长宽高等包围盒数据去进行扫模检测，确保极速飞行时绝对不会漏判。
	
	// 1. 相同组件直接拦截（避免重复绑定）
	if (PrimaryCollisionComponent == InComponent)
	{
		return;
	}

	// 2. 剥离旧状态（清理现场）
	if (PrimaryCollisionComponent)
	{
		PrimaryCollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap);
	}

	// 3. 状态更新
	PrimaryCollisionComponent = InComponent;
	USceneComponent* NewMoveTarget = SceneRoot; // 兜底策略：如果没碰撞体，发动机就退化去推 Root

	// 4. 绑定新状态
	if (PrimaryCollisionComponent)
	{
		PrimaryCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap);
		NewMoveTarget = PrimaryCollisionComponent; // 有真实碰撞体，就让发动机推真实碰撞体
	}

	// 5. 统一更新发动机的驱动目标
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->SetUpdatedComponent(NewMoveTarget);
	}
}

void ASurvivorProjectileBase::InitializeProjectile(
	UAbilitySystemComponent* InSourceASC,
	const FGameplayEffectSpecHandle& InDamageSpecHandle,
	const FVector& Direction,
	float Speed,
	float LifeSeconds)
{
	//玩家释放技能（Gameplay Ability）时，算出伤害（比如基础 10 点 + 50% 攻击力），把这个算好的伤害打包成一个包裹（FGameplayEffectSpecHandle）。
	//调用 InitializeProjectile 时，把这个包裹（SpecHandle）和是谁发出的（SourceASC）塞给投射物。
	//签收： 投射物在天上飞，只要撞到了敌人，触发 ApplyDamageSpecToActor。它完全不管伤害是多少，直接把怀里的包裹塞进敌人的 TargetASC 里。这样，所有的伤害修正、暴击计算逻辑都在发射者身上完成了，投射物类极其干净。
	SourceASC = InSourceASC;
	DamageEffectSpecHandle = InDamageSpecHandle;
	HitActors.Reset();

	FVector SafeDirection = Direction;
	SafeDirection.Z = Direction.Z;
	if (!SafeDirection.Normalize())
	{
		SafeDirection = FVector::ForwardVector;
	}

	SetActorRotation(SafeDirection.Rotation());

	if (ProjectileMovementComponent)
	{
		const float SafeSpeed = FMath::Max(0.0f, Speed);
		ProjectileMovementComponent->Velocity = SafeDirection * SafeSpeed;
		ProjectileMovementComponent->InitialSpeed = SafeSpeed;
		ProjectileMovementComponent->MaxSpeed = SafeSpeed;
		ProjectileMovementComponent->Activate(true);
	}

	SetLifeSpan(FMath::Max(0.0f, LifeSeconds));

	UE_LOG(LogSurvivorArena, Log, TEXT("Projectile initialized. Projectile=%s SourceASC=%s Speed=%.2f LifeSeconds=%.2f SpecValid=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InSourceASC),
		Speed,
		LifeSeconds,
		InDamageSpecHandle.IsValid() ? TEXT("true") : TEXT("false"));
}

UPrimitiveComponent* ASurvivorProjectileBase::GetPrimaryCollisionComponent() const
{
	return PrimaryCollisionComponent;
}

UProjectileMovementComponent* ASurvivorProjectileBase::GetProjectileMovementComponent() const
{
	return ProjectileMovementComponent;
}

void ASurvivorProjectileBase::HandleProjectileOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!CanHitActor(OtherActor))
	{
		return;
	}

	if (!bAllowMultipleHits && HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor); //这个机制是做防止一个角色解算多次碰撞伤害的，防止抖动
	HandleProjectileImpact(OtherActor, SweepResult);
}

void ASurvivorProjectileBase::HandleProjectileImpact(AActor* HitActor, const FHitResult& Hit)
{
	ApplyDamageSpecToActor(HitActor);

	if (bDestroyOnHit)
	{
		Destroy();
	}
}

bool ASurvivorProjectileBase::CanHitActor(AActor* OtherActor) const
{
	if (!OtherActor || OtherActor == this)
	{
		return false;
	}
	//关于Instigator的说明：在虚幻引擎（UE5）中，Instigator 是一个非常重要且底层的核心概念。字面意思是“煽动者”或“始作俑者”，在引擎的 gameplay 架构中，它特指触发当前行为、释放当前技能或造成当前伤害的那个核心角色实体（Pawn / Character）。
	//理解它与 Owner（拥有者）的区别。这是虚幻引擎为了解耦“物理层级”和“逻辑责任”而设计的双轨制。
	//比如说玩家（Actor）开枪，子弹的Owner是枪，但是Instigator赋值为玩家，这样子弹既不能对枪造成伤害，当然也不能对玩家造成伤害；
	if (OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return false;
	}

	return true;
}

void ASurvivorProjectileBase::ApplyDamageSpecToActor(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	if (!SourceASC.IsValid())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Projectile %s cannot apply damage because SourceASC is null."), *GetNameSafe(this));
		return;
	}

	if (!DamageEffectSpecHandle.IsValid() || !DamageEffectSpecHandle.Data.IsValid())
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Projectile %s cannot apply damage because DamageEffectSpecHandle is invalid."), *GetNameSafe(this));
		return;
	}

	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);

	UE_LOG(LogSurvivorArena, Log, TEXT("Projectile %s applied damage spec to %s"), *GetNameSafe(this), *GetNameSafe(TargetActor));
}

void ASurvivorProjectileBase::OnPrimaryCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleProjectileOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}
