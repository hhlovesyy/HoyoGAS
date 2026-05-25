#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MyUIStoreBase.generated.h"

class APlayerState;
class ULocalPlayer;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMyUIStoreChanged, class UUIStoreBase*);

UCLASS(Abstract, BlueprintType)
class HOYOGAS_API UUIStoreBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeStore(ULocalPlayer* InOwningPlayer, FGameplayTag InStoreTag);
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState);
	virtual void UnbindFromPlayerContext();

	FGameplayTag GetStoreTag() const;
	ULocalPlayer* GetOwningLocalPlayer() const;

	FOnMyUIStoreChanged& OnStoreChanged();

protected:
	void BroadcastStoreChanged();

	UPROPERTY(Transient)
	TWeakObjectPtr<ULocalPlayer> OwningLocalPlayer;

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> ObservedPawn;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerState> ObservedPlayerState;

	FGameplayTag StoreTag;
	FOnMyUIStoreChanged StoreChangedEvent;
};
