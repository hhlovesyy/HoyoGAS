#include "Subsystems/MyUIStoreSubsystem.h"

#include "Data/MyUIStoreRegistryDataAsset.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Stores/MyUIStoreBase.h"

void UMyUIStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadRegistry();
	RefreshPlayerContext();
}

void UMyUIStoreSubsystem::Deinitialize()
{
	//当玩家退出游戏时触发，遍历所有 Store，调用它们的 UnbindFromPlayerContext() 清理残留，防止内存泄漏。
	for (TPair<FGameplayTag, TObjectPtr<UUIStoreBase>>& Pair : StoresByTag)
	{
		if (Pair.Value)
		{
			Pair.Value->UnbindFromPlayerContext();
		}
	}

	StoresByTag.Empty();
	RegistryEntriesByTag.Empty();
	Super::Deinitialize();
}

void UMyUIStoreSubsystem::RefreshPlayerContext()
{
	//在 UE 中，玩家控制器（PlayerController）和角色（Pawn）是经常变化的（比如角色死了重生、或者看剧情时交出控制权）。
	/*
	*	当 Pawn 发生变化时（比如角色复活），代码会调用 RefreshPlayerContext()。
		它会去检查：当前玩家还有没有 Pawn 或 PlayerState？
		如果有 (bHasPlayerContext = true)：把当前的 Pawn 和 PlayerState 塞给各个 Store（BindToPlayerContext），让 Store 重新监听角色的血量、蓝量变化。
		如果没有 (bHasPlayerContext = false)：这时候再看这个 Store 的配置 bPersistent（是否持久）。
		如果是持久的（比如“玩家整体成就数据”），就放着不管。
		如果不是持久的（比如“当前关卡的连击数”），大管家会冷酷无情地解绑并销毁这个 Store（StoresByTag.Remove），绝不留内存垃圾。
	 */
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* PlayerController = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	APawn* CurrentPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	APlayerState* CurrentPlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	const bool bHasPlayerContext = CurrentPawn || CurrentPlayerState;

	for (const TPair<FGameplayTag, FMyUIStoreRegistryEntry>& Pair : RegistryEntriesByTag)
	{
		if (!bHasPlayerContext && !Pair.Value.bPersistent)
		{
			if (TObjectPtr<UUIStoreBase>* ExistingStore = StoresByTag.Find(Pair.Key))
			{
				if (ExistingStore->Get())
				{
					(*ExistingStore)->UnbindFromPlayerContext();
				}
				StoresByTag.Remove(Pair.Key);
			}

			continue;
		}

		if (!StoresByTag.Contains(Pair.Key) && Pair.Value.bAutoCreate)
		{
			CreateStoreFromEntry(Pair.Value);
		}

		if (TObjectPtr<UUIStoreBase>* Store = StoresByTag.Find(Pair.Key))
		{
			if (Store->Get())
			{
				(*Store)->BindToPlayerContext(CurrentPawn, CurrentPlayerState);
			}
		}
	}
}

UUIStoreBase* UMyUIStoreSubsystem::GetStoreByTag(FGameplayTag StoreTag) const
{
	if (!StoreTag.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<UUIStoreBase>* FoundStore = StoresByTag.Find(StoreTag))
	{
		return FoundStore->Get();
	}

	return nullptr;
}

void UMyUIStoreSubsystem::LoadRegistry()
{
	RegistryEntriesByTag.Empty();
	StoresByTag.Empty();

	if (RegistryAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::LoadRegistry skipped because RegistryAsset is not configured."));
		return;
	}

	UMyUIStoreRegistryDataAsset* LoadedRegistry = RegistryAsset.LoadSynchronous();
	if (!LoadedRegistry)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::LoadRegistry failed because RegistryAsset could not be loaded."));
		return;
	}

	for (const FMyUIStoreRegistryEntry& Entry : LoadedRegistry->Stores)
	{
		if (!Entry.StoreTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::LoadRegistry skipped an entry because StoreTag is invalid."));
			continue;
		}

		RegistryEntriesByTag.Add(Entry.StoreTag, Entry);

		if (Entry.bAutoCreate)
		{
			CreateStoreFromEntry(Entry);
		}
	}
}

void UMyUIStoreSubsystem::CreateStoreFromEntry(const FMyUIStoreRegistryEntry& Entry)
{
	if (!Entry.StoreTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::CreateStoreFromEntry skipped because StoreTag is invalid."));
		return;
	}

	if (StoresByTag.Contains(Entry.StoreTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem found duplicate StoreTag '%s'. Later config ignored."), *Entry.StoreTag.ToString());
		return;
	}

	TSubclassOf<UUIStoreBase> StoreClass = Entry.StoreClass.LoadSynchronous();
	if (!StoreClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::CreateStoreFromEntry failed because StoreClass for '%s' is null."), *Entry.StoreTag.ToString());
		return;
	}

	UUIStoreBase* Store = NewObject<UUIStoreBase>(this, StoreClass);
	if (!Store)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIStoreSubsystem::CreateStoreFromEntry failed because store instance could not be created for '%s'."), *Entry.StoreTag.ToString());
		return;
	}

	Store->InitializeStore(GetLocalPlayer(), Entry.StoreTag);
	StoresByTag.Add(Entry.StoreTag, Store);
}
