#include "Actors/CollectiblePickupActor.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/ScoreProgressionComponent.h"
#include "Data/ItemDefinitionRow.h"
#include "Events/GameplayDemoEventPayloads.h"
#include "GameplayDemoStatics.h"
#include "GameFramework/Pawn.h"
#include "HoyoGasPlayerState.h"
#include "MyUITags.h"
#include "Subsystems/MyGameplayTagEventBusSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ACollectiblePickupActor::ACollectiblePickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(Root);
	BillboardComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	BillboardComponent->SetRelativeScale3D(FVector(0.15f));

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->SetupAttachment(Root);
	PickupCollision->SetSphereRadius(80.0f);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	SphereMesh = SphereMeshFinder.Object;
	CubeMesh = CubeMeshFinder.Object;
	ConeMesh = ConeMeshFinder.Object;
}

void ACollectiblePickupActor::BeginPlay()
{
	Super::BeginPlay();
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &ACollectiblePickupActor::HandlePickupOverlap);
	RefreshVisuals();
}

void ACollectiblePickupActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AddActorLocalRotation(FRotator(0.0f, SpinSpeedDegreesPerSecond * DeltaSeconds, 0.0f));
}

//OnConstruction 会在 UE 编辑器里你拖动或者修改这个 Actor 属性的瞬间被调用（类似于蓝图里的 Construction Script）
void ACollectiblePickupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisuals();
}

void ACollectiblePickupActor::HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	APawn* OtherPawn = Cast<APawn>(OtherActor);
	AHoyoGasPlayerState* HoyoGasPlayerState = OtherPawn ? OtherPawn->GetPlayerState<AHoyoGasPlayerState>() : nullptr;
	UInventoryComponent* InventoryComponent = HoyoGasPlayerState ? HoyoGasPlayerState->GetInventoryComponent() : nullptr;
	UScoreProgressionComponent* ScoreComponent = HoyoGasPlayerState ? HoyoGasPlayerState->GetScoreProgressionComponent() : nullptr;
	if (!InventoryComponent || !ScoreComponent)
	{
		return;
	}

	FItemDefinitionRow ItemDefinition;
	if (!UGameplayDemoStatics::TryGetItemDefinition(this, ItemId, ItemDefinition))
	{
		return;
	}

	InventoryComponent->AddItem(ItemId, 1);
	ScoreComponent->AddScore(ItemDefinition.ScoreValue);

	if (ItemDefinition.Rarity >= 3)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UMyGameplayTagEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UMyGameplayTagEventBusSubsystem>())
			{
				FGameplayDemoRarePickupEventPayload Payload;
				Payload.ItemId = ItemId;
				Payload.DisplayName = ItemDefinition.DisplayName;
				Payload.ToastMessage = FText::Format(INVTEXT("Rare pickup: {0}"), ItemDefinition.DisplayName);
				Payload.Rarity = ItemDefinition.Rarity;
				EventBus->PublishTyped(MyUITags::Gameplay_Event_RarePickup, Payload);
			}
		}
	}

	Destroy();
}

void ACollectiblePickupActor::RefreshVisuals()
{
	FItemDefinitionRow ItemDefinition;
	if (!UGameplayDemoStatics::TryGetItemDefinition(this, ItemId, ItemDefinition))
	{
		return;
	}

	UStaticMesh* MeshToUse = ItemDefinition.Mesh.LoadSynchronous();
	if (!MeshToUse)
	{
		if (ItemDefinition.ShapeTypeTag == TEXT("Cube"))
		{
			MeshToUse = CubeMesh;
		}
		else if (ItemDefinition.ShapeTypeTag == TEXT("Cone"))
		{
			MeshToUse = ConeMesh;
		}
		else
		{
			MeshToUse = SphereMesh;
		}
	}

	MeshComponent->SetStaticMesh(MeshToUse);
	MeshComponent->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(ItemDefinition.TintColor));
	MeshComponent->SetVectorParameterValueOnMaterials(TEXT("TintColor"), FVector(ItemDefinition.TintColor));

	if (UTexture2D* BillboardTexture = ItemDefinition.BillboardTexture.LoadSynchronous())
	{
		BillboardComponent->SetSprite(BillboardTexture);
		BillboardComponent->SetHiddenInGame(false);
	}
	else
	{
		BillboardComponent->SetHiddenInGame(true);
	}
}
