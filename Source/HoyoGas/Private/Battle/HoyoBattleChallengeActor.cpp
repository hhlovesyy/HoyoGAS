// Copyright Epic Games, Inc. All Rights Reserved.

#include "Battle/HoyoBattleChallengeActor.h"

#include "Battle/HoyoBattleEncounterDefinition.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystems/HoyoBattleFlowSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AHoyoBattleChallengeActor::AHoyoBattleChallengeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(Root);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AHoyoBattleChallengeActor::HandleInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AHoyoBattleChallengeActor::HandleInteractionSphereEndOverlap);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	}
}

bool AHoyoBattleChallengeActor::CanInteract_Implementation(APlayerController* InteractingController) const
{
	if (!InteractingController || !EncounterDefinition)
	{
		return false;
	}

	if (const APawn* InteractingPawn = InteractingController->GetPawn())
	{
		if (!InteractionSphere->IsOverlappingActor(InteractingPawn))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	const ULocalPlayer* LocalPlayer = InteractingController->GetLocalPlayer();
	const UHoyoBattleFlowSubsystem* BattleFlowSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>() : nullptr;
	return BattleFlowSubsystem && BattleFlowSubsystem->GetFlowState() == EHoyoBattleFlowState::Exploration;
}

FText AHoyoBattleChallengeActor::GetInteractionText_Implementation() const
{
	return InteractionText;
}

void AHoyoBattleChallengeActor::Interact_Implementation(APlayerController* InteractingController)
{
	//为什么用Execute_XXX呢？在标准的 C++ 中，如果对象继承了接口，你通常会直接调用 this->CanInteract()。但在 UE5 中，因为接口可能是在蓝图中被实现的（比如设计师新建了一个蓝图 Actor，并没有继承你的 C++ Actor，只是单纯实现了这个接口）。
	//Execute_ 的魔法： Execute_XXX 是 UHT 自动生成的静态路由函数。它的工作逻辑是：去检查传入的这个对象（this），看看它的蓝图中有没有重写这个方法；如果有，就去执行蓝图的逻辑；如果没有，再去执行 C++ 里的 _Implementation。这保证了跨语言（C++/Blueprint）调用的绝对安全。
	if (!IHoyoInteractableInterface::Execute_CanInteract(this, InteractingController))
	{
		return;
	}

	ULocalPlayer* LocalPlayer = InteractingController ? InteractingController->GetLocalPlayer() : nullptr;
	UHoyoBattleFlowSubsystem* BattleFlowSubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>() : nullptr;
	if (!BattleFlowSubsystem)
	{
		return;
	}

	if (BattleFlowSubsystem->RequestEnterBattle(EncounterDefinition, GetActorTransform()))
	{
		UE_LOG(LogTemp, Display, TEXT("BattleChallengeActor '%s' requested encounter '%s'."), *GetNameSafe(this), *EncounterDefinition->GetEncounterId().ToString());
	}
}

void AHoyoBattleChallengeActor::BeginPlay()
{
	Super::BeginPlay();

	if (MeshComponent)
	{
		DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	}

	RefreshVisuals();
}

void AHoyoBattleChallengeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisuals();
}

void AHoyoBattleChallengeActor::HandleInteractionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || !Cast<APawn>(OtherActor))
	{
		return;
	}

	bPlayerInRange = true;
	RefreshVisuals();
	ShowInteractionPrompt();

	if (bDebugLogRangeChanges)
	{
		UE_LOG(LogTemp, Display, TEXT("BattleChallengeActor '%s' player entered interaction range."), *GetNameSafe(this));
	}
}

void AHoyoBattleChallengeActor::HandleInteractionSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this || !Cast<APawn>(OtherActor))
	{
		return;
	}

	bPlayerInRange = false;
	RefreshVisuals();
	HideInteractionPrompt();

	if (bDebugLogRangeChanges)
	{
		UE_LOG(LogTemp, Display, TEXT("BattleChallengeActor '%s' player left interaction range."), *GetNameSafe(this));
	}
}

void AHoyoBattleChallengeActor::ShowInteractionPrompt() const
{
	if (!bShowOnScreenInteractionPrompt || !GEngine || !EncounterDefinition)
	{
		return;
	}

	const uint64 PromptMessageKey = static_cast<uint64>(GetUniqueID());
	//UE 的 Execute_ 宏在早期的生成机制中有时不能完美适配 const 指针的传递，导致编译器报错，所以你被迫用了 const_cast 去掉常性。
	//评价： 这是 UE C++ 开发中非常经典的“向引擎妥协”，你的处理方式是完全可以接受的。在最新的 UE5.3+ 中，如果接口函数声明了 const，通常 UHT 会正确生成接受 const UObject* 的 Execute_ 方法。你可以尝试去掉 const_cast，看看当前版本的编译器是否已经支持。
	//const FText InteractionPromptText = IHoyoInteractableInterface::Execute_GetInteractionText(const_cast<AHoyoBattleChallengeActor*>(this));
	const FText InteractionPromptText = IHoyoInteractableInterface::Execute_GetInteractionText(this);
	const FString PromptText = FString::Printf(TEXT("Press E - %s"), *InteractionPromptText.ToString());
	GEngine->AddOnScreenDebugMessage(PromptMessageKey, 0.0f, FColor::Magenta, PromptText);
}

void AHoyoBattleChallengeActor::HideInteractionPrompt() const
{
	if (!bShowOnScreenInteractionPrompt || !GEngine)
	{
		return;
	}

	const uint64 PromptMessageKey = static_cast<uint64>(GetUniqueID());
	GEngine->AddOnScreenDebugMessage(PromptMessageKey, 0.01f, FColor::Transparent, TEXT(""));
}

void AHoyoBattleChallengeActor::RefreshVisuals()
{
	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(InteractionRadius);
	}

	if (MeshComponent)
	{
		const FLinearColor ActiveColor = bPlayerInRange ? InRangeDisplayColor : DisplayColor;
		const float ActiveScale = bPlayerInRange ? 1.8f : 1.5f;

		MeshComponent->SetWorldScale3D(FVector(ActiveScale, ActiveScale, ActiveScale));
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(ActiveColor));
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(ActiveColor));
		MeshComponent->SetVectorParameterValueOnMaterials(TEXT("TintColor"), FVector(ActiveColor));

		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), ActiveColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), ActiveColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("TintColor"), ActiveColor);
		}
	}
}
