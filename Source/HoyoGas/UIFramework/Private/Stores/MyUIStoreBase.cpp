#include "Stores/MyUIStoreBase.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

void UUIStoreBase::InitializeStore(ULocalPlayer* InOwningPlayer, FGameplayTag InStoreTag)
{
	OwningLocalPlayer = InOwningPlayer;
	StoreTag = InStoreTag;
}

void UUIStoreBase::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	ObservedPawn = InPawn;
	ObservedPlayerState = InPlayerState;
}

void UUIStoreBase::UnbindFromPlayerContext()
{
	ObservedPawn.Reset();
	ObservedPlayerState.Reset();
}

FGameplayTag UUIStoreBase::GetStoreTag() const
{
	return StoreTag;
}

ULocalPlayer* UUIStoreBase::GetOwningLocalPlayer() const
{
	return OwningLocalPlayer.Get();
}

FOnMyUIStoreChanged& UUIStoreBase::OnStoreChanged()
{
	return StoreChangedEvent;
}

void UUIStoreBase::BroadcastStoreChanged()
{
	StoreChangedEvent.Broadcast(this);
}
