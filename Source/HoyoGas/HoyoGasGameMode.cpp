// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoyoGasGameMode.h"
#include "Actors/CollectiblePickupActor.h"
#include "Data/ItemDefinitionRow.h"
#include "GameplayDemoSettings.h"
#include "GameplayDemoStatics.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "HoyoGasPlayerState.h"
#include "Kismet/GameplayStatics.h"

AHoyoGasGameMode::AHoyoGasGameMode()
{
	PlayerStateClass = AHoyoGasPlayerState::StaticClass();
}

void AHoyoGasGameMode::StartPlay()
{
	Super::StartPlay();
}

void AHoyoGasGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!bGameplayDemoPickupsSpawned && NewPlayer)
	{
		bGameplayDemoPickupsSpawned = SpawnGameplayDemoPickups(NewPlayer->GetPawn());
	}
}

bool AHoyoGasGameMode::SpawnGameplayDemoPickups(APawn* PlayerPawn)
{
	const UGameplayDemoSettings* Settings = GetDefault<UGameplayDemoSettings>();
	if (!Settings || !Settings->bAutoSpawnPickups || !GetWorld())
	{
		return false;
	}

	if (!IsValid(PlayerPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("AHoyoGasGameMode::SpawnGameplayDemoPickups skipped because PlayerPawn is not ready."));
		return false;
	}

	const FVector SpawnOrigin = PlayerPawn->GetActorLocation();
	TArray<FName> SpawnItemIds = Settings->DefaultSpawnItemIds;
	if (SpawnItemIds.IsEmpty())
	{
		TArray<FItemDefinitionRow> ItemDefinitions;
		UGameplayDemoStatics::GetAllItemDefinitions(this, ItemDefinitions);
		for (const FItemDefinitionRow& ItemDefinition : ItemDefinitions)
		{
			SpawnItemIds.Add(ItemDefinition.ItemId);
		}
	}

	if (SpawnItemIds.IsEmpty())
	{
		return false;
	}

	FRandomStream RandomStream(55423);
	for (int32 Index = 0; Index < Settings->DefaultPickupCount; ++Index)
	{
		const float AngleRadians = RandomStream.FRandRange(0.0f, 2.0f * PI);
		const float Distance = RandomStream.FRandRange(150.0f, Settings->PickupSpawnRadius);
		const FVector SpawnOffset(FMath::Cos(AngleRadians) * Distance, FMath::Sin(AngleRadians) * Distance, Settings->PickupSpawnHeight);
		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnOrigin + SpawnOffset);

		//如果你用普通的 SpawnActor，物品一生成就会立刻执行它的 BeginPlay，但此时它还不知道自己是什么道具（ItemId 还没赋上）。使用 Deferred（延迟生成），引擎会先把它在内存里捏出来，等你把 ItemId 塞给它之后，再调用 FinishSpawningActor 让它正式进入世界并触发 BeginPlay。这从根本上杜绝了初始化时序 Bug。
		ACollectiblePickupActor* PickupActor = GetWorld()->SpawnActorDeferred<ACollectiblePickupActor>(ACollectiblePickupActor::StaticClass(), SpawnTransform);
		if (!PickupActor)
		{
			continue;
		}

		PickupActor->ItemId = SpawnItemIds[RandomStream.RandRange(0, SpawnItemIds.Num() - 1)];
		UGameplayStatics::FinishSpawningActor(PickupActor, SpawnTransform);
	}

	return true;
}
