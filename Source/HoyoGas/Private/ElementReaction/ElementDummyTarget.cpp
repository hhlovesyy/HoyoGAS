#include "HoyoGas/Public/ElementReaction/ElementDummyTarget.h"
#include "HoyoGas/Public/ElementReaction/ReactionResolver.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ElementReaction/AuraBarWidget.h"
#include "ElementReaction/DamageTextWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ElementReaction/ReactionFXRow.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "TimerManager.h"

AElementDummyTarget::AElementDummyTarget()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	
	AuraWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AuraWidgetComponent"));
	AuraWidgetComponent->SetupAttachment(RootComponent);
	AuraWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	AuraWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	AuraWidgetComponent->SetDrawSize(FVector2D(220.f, 60.f));
	AuraWidgetComponent->SetDrawAtDesiredSize(false);
	AuraWidgetComponent->SetVisibility(true);
	AuraWidgetComponent->SetHiddenInGame(false);
}

void AElementDummyTarget::BeginPlay()
{
	Super::BeginPlay();

	if (MeshComponent)
	{
		DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	}

	InitAuraWidget();

	// 初始化时把状态清干净
	ClearAura();
	ClearFrozenState();

	RefreshAuraWidget();
}

void AElementDummyTarget::ApplyElementHit(const FElementHitEvent& HitEvent)
{
	const FString OldAuraStr = FReactionResolver::ElementToString(CurrentAura.Element);

	FReactionResult Result = FReactionResolver::Resolve(
		CurrentAura,
		HitEvent,
		Resistance,
		ReactionRuleDataTable,
		ReactionVisualDataTable,
		AuraConfigDataTable
	);

	Health -= Result.FinalDamage;
	CurrentAura = Result.NewAura;
	
	if (Result.bApplyFrozen)
	{
		ApplyFrozenState(2.0f);
	}

	const FString IncomingStr = FReactionResolver::ElementToString(HitEvent.IncomingElement);
	const FString ReactionStr = FReactionResolver::ReactionToString(Result.Reaction);
	const FString NewAuraStr = FReactionResolver::ElementToString(CurrentAura.Element);

	FString DebugText = FString::Printf(
		TEXT("老的 Aura: %s\nIncoming: %s\n元素反应: %s\n基础伤害: %.1f\n最终伤害: %.1f\n新的 Aura: %s\n生命值: %.1f"),
		*OldAuraStr,
		*IncomingStr,
		*ReactionStr,
		HitEvent.BaseDamage,
		Result.FinalDamage,
		*NewAuraStr,
		Health
	);
	
	DebugText += FString::Printf(
		TEXT("\nAuraValue: %.2f / %.2f\nAuraDuration: %.2f / %.2f"),
		CurrentAura.Value,
		CurrentAura.MaxValue,
		CurrentAura.Duration,
		CurrentAura.MaxDuration
	);

	if (Result.bApplyFrozen)
	{
		DebugText += FString::Printf(TEXT("\nFrozen: True (%.1fs)"), FrozenRemainingTime);
	}
	
	PrintDebug(DebugText);
	RefreshAuraWidget();
	
	SpawnDamageText(Result);
	PlayReactionVisual(Result);
}

void AElementDummyTarget::SpawnDamageText(const FReactionResult& Result)
{
	if (!DamageTextClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageTextClass is null"));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	FVector WorldPos = GetActorLocation() + FVector(0.f, 0.f, 120.f);
	WorldPos += FVector(
		FMath::RandRange(-20.f, 20.f),
		FMath::RandRange(-20.f, 20.f),
		FMath::RandRange(0.f, 20.f)
	);

	FVector2D ScreenPos;
	const bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldPos, ScreenPos, false);
	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	const bool bInsideViewport =
		bProjected &&
		ScreenPos.X >= 0.0f &&
		ScreenPos.X <= static_cast<float>(ViewportX) &&
		ScreenPos.Y >= 0.0f &&
		ScreenPos.Y <= static_cast<float>(ViewportY);

	if (!bInsideViewport)
	{
		UE_LOG(LogTemp, Display, TEXT("[UIFramework] SkipDamageText Actor=%s Projected=%s ScreenPos=(%.1f, %.1f) Viewport=(%d, %d)"),
			*GetNameSafe(this),
			bProjected ? TEXT("true") : TEXT("false"),
			ScreenPos.X,
			ScreenPos.Y,
			ViewportX,
			ViewportY);
		return;
	}

	UDamageTextWidget* Widget = CreateWidget<UDamageTextWidget>(PC, DamageTextClass);
	if (!Widget)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer);
	if (!PlayerUISubsystem || !PlayerUISubsystem->AddWidgetInstanceToOverlayLayer(Widget, EMyUILayer::Toast))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnDamageText failed because Toast layer is unavailable."));
		return;
	}

	Widget->SetScreenPosition(ScreenPos);

	Widget->InitDamage(
		Result.FinalDamage,
		Result.ReactionDisplayText,
		Result.ReactionColor,
		Result.DamageTextStyle
	);
}

void AElementDummyTarget::PrintDebug(const FString& Message) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, Message);
	}

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

void AElementDummyTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 处理 Aura：Value 是主变量，MaxDuration / MaxValue 编码这层 Aura 的固定衰减率
	if (CurrentAura.Element != EGenshinElementType::None)
	{
		// 每秒衰减多少 AuraValue
		const float DecayRate = (CurrentAura.MaxDuration > 0.f)
			? (CurrentAura.MaxValue / CurrentAura.MaxDuration)
			: 0.f;

		// 用 Value 作为主变量进行衰减
		CurrentAura.Value -= DecayRate * DeltaTime;
		CurrentAura.Value = FMath::Clamp(CurrentAura.Value, 0.f, CurrentAura.MaxValue);

		// Duration 始终由剩余 Value 反推，不再单独驱动状态
		if (CurrentAura.MaxValue > 0.f)
		{
			CurrentAura.Duration = (CurrentAura.Value / CurrentAura.MaxValue) * CurrentAura.MaxDuration;
		}
		else
		{
			CurrentAura.Duration = 0.f;
		}

		// 如果 AuraValue 见底，就清空 Aura
		if (CurrentAura.Value <= 0.f)
		{
			ClearAura();
		}
	}

	// 2. 处理 Frozen 状态持续时间
	if (bIsFrozen)
	{
		FrozenRemainingTime -= DeltaTime;
		if (FrozenRemainingTime <= 0.f)
		{
			ClearFrozenState();
		}
	}

	RefreshAuraWidget();
}

void AElementDummyTarget::ClearAura()
{
	CurrentAura.Element = EGenshinElementType::None;
	CurrentAura.Value = 0.f;
	CurrentAura.MaxValue = 0.f;
	CurrentAura.Duration = 0.f;
	CurrentAura.MaxDuration = 0.f;

	RefreshAuraWidget();
	ResetReactionVisual();
}

bool AElementDummyTarget::HasAura() const
{
	return CurrentAura.Element != EGenshinElementType::None;
}

float AElementDummyTarget::GetAuraPercent() const
{
	if (CurrentAura.Element == EGenshinElementType::None)
	{
		return 0.f;
	}

	if (CurrentAura.MaxValue <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentAura.Value / CurrentAura.MaxValue, 0.f, 1.f);
}

FLinearColor AElementDummyTarget::GetAuraColor() const
{
	switch (CurrentAura.Element)
	{
	case EGenshinElementType::Pyro:
		return FLinearColor(1.0f, 0.15f, 0.1f, 1.0f);

	case EGenshinElementType::Hydro:
		return FLinearColor(0.1f, 0.45f, 1.0f, 1.0f);

	case EGenshinElementType::Cryo:
		return FLinearColor(0.85f, 0.95f, 1.0f, 1.0f);

	default:
		return FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
	}
}

FText AElementDummyTarget::GetAuraText() const
{
	FString ElementName = TEXT("None");

	switch (CurrentAura.Element)
	{
	case EGenshinElementType::Pyro:
		ElementName = TEXT("火元素");
		break;
	case EGenshinElementType::Hydro:
		ElementName = TEXT("水元素");
		break;
	case EGenshinElementType::Cryo:
		ElementName = TEXT("冰元素");
		break;
	default:
		break;
	}

	const FString Text = FString::Printf(
		TEXT("%s  %.0f%%  (%.1fs)"),
		*ElementName,
		GetAuraPercent() * 100.f,
		CurrentAura.Duration
	);

	return FText::FromString(Text);
}

void AElementDummyTarget::RefreshAuraWidget()
{
	if (!CachedAuraWidget)
	{
		return;
	}

	CachedAuraWidget->RefreshFromTarget();
}

void AElementDummyTarget::InitAuraWidget()
{
	if (!AuraWidgetComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraWidgetComponent is null"));
		return;
	}

	AuraWidgetComponent->InitWidget();

	UUserWidget* RawWidget = AuraWidgetComponent->GetUserWidgetObject();
	if (!RawWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraWidgetComponent has no UserWidgetObject. Check WidgetClass in BP_DummyEnemy."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("RawWidget Class = %s"), *RawWidget->GetClass()->GetName());

	CachedAuraWidget = Cast<UAuraBarWidget>(RawWidget);
	if (!CachedAuraWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Cast to UAuraBarWidget failed. Check WBP_AuraBar parent class."));
		return;
	}

	CachedAuraWidget->SetObservedTarget(this);
	CachedAuraWidget->RefreshFromTarget();

	UE_LOG(LogTemp, Warning, TEXT("Aura widget initialized successfully"));
}

void AElementDummyTarget::PlayReactionVisual(const FReactionResult& Result)
{
	UpdateMaterialVisualState(Result);
	TriggerReactionFX(Result);
}

void AElementDummyTarget::ResetReactionVisual()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("FrozenAmount"), 0.0f);
	}

	ClearPersistentFrozenFX();
}

void AElementDummyTarget::UpdateMaterialVisualState(const FReactionResult& Result)
{
	if (!DynamicMaterial)
	{
		return;
	}

	const float FrozenAmount = Result.bUseFreezeMaterial ? Result.FreezeAmount : 0.0f;
	DynamicMaterial->SetScalarParameterValue(TEXT("FrozenAmount"), FrozenAmount);
}

void AElementDummyTarget::TriggerReactionFX(const FReactionResult& Result)
{
	if (!ReactionFXDataTable || Result.ReactionFXRowName.IsNone())
	{
		return;
	}

	static const FString ContextString(TEXT("ReactionFXLookup"));
	const FReactionFXRow* FXRow = ReactionFXDataTable->FindRow<FReactionFXRow>(Result.ReactionFXRowName, ContextString);

	if (!FXRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("No FX row found for %s"), *Result.ReactionFXRowName.ToString());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TriggerReactionFX: %s"), *FXRow->DebugText.ToString());

	if (FXRow->CascadeFX)
	{
		const FVector SpawnLocation = GetActorLocation() + FXRow->SpawnOffset;
		const FRotator SpawnRotation = FRotator::ZeroRotator;
		UParticleSystemComponent* SpawnedFXComponent = nullptr;

		if (FXRow->bAttachToTarget)
		{
			SpawnedFXComponent = UGameplayStatics::SpawnEmitterAttached(
				FXRow->CascadeFX,
				RootComponent,
				NAME_None,
				FXRow->SpawnOffset,
				SpawnRotation,
				FXRow->SpawnScale,
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}
		else
		{
			SpawnedFXComponent = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				FXRow->CascadeFX,
				FTransform(SpawnRotation, SpawnLocation, FXRow->SpawnScale),
				true
			);
		}

		if (Result.Reaction == EReactionType::Frozen)
		{
			ClearPersistentFrozenFX();
			ActiveFrozenFXComponent = SpawnedFXComponent;
		}

		if (SpawnedFXComponent && FXRow->AutoDestroyDelaySeconds > 0.f)
		{
			TWeakObjectPtr<UParticleSystemComponent> WeakFXComponent = SpawnedFXComponent;
			FTimerHandle AutoDestroyTimerHandle;

			GetWorldTimerManager().SetTimer(
				AutoDestroyTimerHandle,
				[WeakFXComponent]()
				{
					if (!WeakFXComponent.IsValid())
					{
						return;
					}

					WeakFXComponent->DeactivateSystem();
					WeakFXComponent->DestroyComponent();
				},
				FXRow->AutoDestroyDelaySeconds,
				false
			);

			if (ActiveFrozenFXComponent == SpawnedFXComponent)
			{
				ActiveFrozenFXComponent = nullptr;
			}
		}
	}
}

void AElementDummyTarget::ClearPersistentFrozenFX()
{
	if (!ActiveFrozenFXComponent)
	{
		return;
	}

	ActiveFrozenFXComponent->DeactivateSystem();
	ActiveFrozenFXComponent->DestroyComponent();
	ActiveFrozenFXComponent = nullptr;
}

void AElementDummyTarget::ApplyFrozenState(float Duration)
{
	bIsFrozen = true;
	FrozenRemainingTime = Duration;

	UE_LOG(LogTemp, Warning, TEXT("Frozen Applied, Duration = %.2f"), Duration);
}

void AElementDummyTarget::ClearFrozenState()
{
	bIsFrozen = false;
	FrozenRemainingTime = 0.f;
	ResetReactionVisual();

	UE_LOG(LogTemp, Warning, TEXT("Frozen Cleared"));
}
