#include "Inventory/SurvivorRunInventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Core/SurvivorArenaLog.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Weapons/SurvivorWeaponDefinition.h"

USurvivorRunInventoryComponent::USurvivorRunInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurvivorRunInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("RunInventoryComponent initialized. Owner=%s"), *GetNameSafe(GetOwner()));
}

bool USurvivorRunInventoryComponent::GrantStartingAbilitySet(USurvivorAbilitySet* AbilitySet, UObject* SourceObject)
{
	if (!AbilitySet)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingAbilitySet failed because AbilitySet is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	UAbilitySystemComponent* ASC = ResolveOwnerASC();
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingAbilitySet failed because owner ASC is null. Owner=%s AbilitySet=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(AbilitySet));
		return false;
	}

	const FSurvivorGrantedAbilitySetHandles GrantedHandles = AbilitySet->GiveToAbilitySystemAndCollect(ASC, SourceObject);
	GrantedStartingAbilitySetHandles.Add(GrantedHandles);

	UE_LOG(LogSurvivorArena, Log, TEXT("Granted starting ability set. Owner=%s AbilitySet=%s Count=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(AbilitySet),
		GrantedStartingAbilitySetHandles.Num());

	BroadcastInventoryChanged();
	return true;
}

bool USurvivorRunInventoryComponent::GrantStartingWeapon(USurvivorWeaponDefinition* WeaponDefinition)
{
	USurvivorWeaponManagerComponent* WeaponManager = ResolveOwnerWeaponManager();
	if (!WeaponDefinition || !WeaponManager)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantStartingWeapon failed because WeaponDefinition or WeaponManager is null. Owner=%s Weapon=%s WeaponManager=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition),
			*GetNameSafe(WeaponManager));
		return false;
	}

	FSurvivorGrantedWeaponHandles GrantedHandles;
	if (!WeaponManager->GrantWeaponAndCollect(WeaponDefinition, GrantedHandles))
	{
		return false;
	}

	GrantedStartingWeapons.Add(GrantedHandles);

	UE_LOG(LogSurvivorArena, Log, TEXT("Granted starting weapon. Owner=%s Weapon=%s Count=%d"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(WeaponDefinition),
		GrantedStartingWeapons.Num());

	BroadcastInventoryChanged();
	return true;
}

void USurvivorRunInventoryComponent::ClearStartingLoadout()
{
	if (UAbilitySystemComponent* ASC = ResolveOwnerASC())
	{
		for (FSurvivorGrantedAbilitySetHandles& GrantedHandles : GrantedStartingAbilitySetHandles)
		{
			GrantedHandles.TakeFromAbilitySystem(ASC);
		}
	}

	GrantedStartingAbilitySetHandles.Reset();

	if (USurvivorWeaponManagerComponent* WeaponManager = ResolveOwnerWeaponManager())
	{
		for (const FSurvivorGrantedWeaponHandles& GrantedHandles : GrantedStartingWeapons)
		{
			WeaponManager->RemoveWeaponByHandles(GrantedHandles);
		}
	}

	GrantedStartingWeapons.Reset();
	BroadcastInventoryChanged();
}

int32 USurvivorRunInventoryComponent::GetGrantedStartingWeaponCount() const
{
	return GrantedStartingWeapons.Num();
}

int32 USurvivorRunInventoryComponent::GetGrantedStartingAbilitySetCount() const
{
	return GrantedStartingAbilitySetHandles.Num();
}

FOnSurvivorRunInventoryChanged& USurvivorRunInventoryComponent::OnInventoryChanged()
{
	return InventoryChangedEvent;
}

UAbilitySystemComponent* USurvivorRunInventoryComponent::ResolveOwnerASC() const
{
	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	return nullptr;
}

USurvivorWeaponManagerComponent* USurvivorRunInventoryComponent::ResolveOwnerWeaponManager() const
{
	const APlayerState* PlayerStateOwner = Cast<APlayerState>(GetOwner());
	APawn* Pawn = PlayerStateOwner ? PlayerStateOwner->GetPawn() : Cast<APawn>(GetOwner());
	return Pawn ? Pawn->FindComponentByClass<USurvivorWeaponManagerComponent>() : nullptr;
}

void USurvivorRunInventoryComponent::BroadcastInventoryChanged()
{
	InventoryChangedEvent.Broadcast(this);
}
