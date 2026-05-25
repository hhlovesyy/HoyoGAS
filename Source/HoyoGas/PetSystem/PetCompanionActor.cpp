#include "PetCompanionActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "ElementReaction/ElementDummyTarget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

APetCompanionActor::APetCompanionActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PetMesh"));
	PetMesh->SetupAttachment(SceneRoot);

	ElementIconWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ElementIconWidget"));
	ElementIconWidget->SetupAttachment(SceneRoot);
	ElementIconWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	ElementIconWidget->SetWidgetSpace(EWidgetSpace::Screen);
	ElementIconWidget->SetDrawSize(FVector2D(64.f, 64.f));
}

void APetCompanionActor::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();

	FindDebugTarget();
	RefreshElementIcon();

	if (bRandomizeElement)
	{
		StartRandomElementLoop();
	}

	if (bAutoAttack)
	{
		StartAutoAttack();
	}
}

void APetCompanionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	// ===== 最小版原地漂浮 =====
	FVector NewLocation = InitialLocation;
	NewLocation.Z += FMath::Sin(RunningTime * FloatSpeed) * FloatAmplitude;
	SetActorLocation(NewLocation);

	// ===== 最小版 Q弹 =====
	const float ScaleOffset = FMath::Sin(RunningTime * FloatSpeed * 2.0f) * ScaleAmplitude;
	const FVector NewScale(
		1.0f + ScaleOffset,
		1.0f + ScaleOffset,
		1.0f - ScaleOffset * 0.5f
	);

	SetActorScale3D(NewScale);
}

void APetCompanionActor::FindDebugTarget()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AElementDummyTarget::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		DebugTarget = Cast<AElementDummyTarget>(FoundActors[0]);
	}

	if (!DebugTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("PetCompanionActor: DebugTarget not found"));
	}
}

void APetCompanionActor::StartAutoAttack()
{
	if (AttackInterval <= 0.f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&APetCompanionActor::FireElementAtTarget,
		AttackInterval,
		true
	);
}

void APetCompanionActor::StartRandomElementLoop()
{
	if (RandomElementInterval <= 0.f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		RandomElementTimerHandle,
		this,
		&APetCompanionActor::RandomizeElement,
		RandomElementInterval,
		true
	);
}

void APetCompanionActor::RandomizeElement()
{
	const int32 RandIndex = FMath::RandRange(0, 2);

	switch (RandIndex)
	{
	case 0:
		CurrentElement = EGenshinElementType::Pyro;
		break;

	case 1:
		CurrentElement = EGenshinElementType::Hydro;
		break;

	case 2:
		CurrentElement = EGenshinElementType::Cryo;
		break;

	default:
		CurrentElement = EGenshinElementType::Pyro;
		break;
	}

	RefreshElementIcon();
	BP_OnElementChanged(CurrentElement);

	UE_LOG(LogTemp, Warning, TEXT("Pet element changed to: %d"), (int32)CurrentElement);
}

void APetCompanionActor::RefreshElementIcon()
{
	// 现在先留空
	// 后面你可以：
	// 1. 更新 Widget 图标
	// 2. 改颜色
	// 3. 播一个元素切换特效
}

void APetCompanionActor::FireElementAtTarget()
{
	if (!DebugTarget)
	{
		FindDebugTarget();
		if (!DebugTarget)
		{
			return;
		}
	}

	// 这里先播一个蓝图事件，后面你可以在蓝图里接攻击动作
	BP_PlayAttackAnim();

	// 当前最小版：先直接命中
	ExecuteAttackHit();
}

void APetCompanionActor::ExecuteAttackHit()
{
	if (!DebugTarget)
	{
		return;
	}

	FElementHitEvent HitEvent;
	HitEvent.IncomingElement = CurrentElement;
	HitEvent.BaseDamage = BaseDamage;

	// 先写死来源数值，后面再接成长 / GAS
	HitEvent.SourceStats.ElementalMastery = ElementalMastery;
	HitEvent.SourceStats.CharacterLevel = CharacterLevel;

	DebugTarget->ApplyElementHit(HitEvent);

	if (GEngine)
	{
		const FString DebugMsg = FString::Printf(
			TEXT("Pet Attack -> Element: %d, Damage: %.1f, EM: %.1f, Lv: %d"),
			(int32)CurrentElement,
			BaseDamage,
			ElementalMastery,
			CharacterLevel
		);

		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, DebugMsg);
	}
}