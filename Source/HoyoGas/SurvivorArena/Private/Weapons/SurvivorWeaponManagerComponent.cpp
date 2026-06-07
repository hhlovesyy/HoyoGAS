#include "Weapons/SurvivorWeaponManagerComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Weapons/SurvivorWeaponDefinition.h"

USurvivorWeaponManagerComponent::USurvivorWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurvivorWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("WeaponManager BeginPlay. Owner=%s"), *GetNameSafe(GetOwner()));
}

void USurvivorWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllGrantedWeapons();
	Super::EndPlay(EndPlayReason);
}

bool USurvivorWeaponManagerComponent::GrantWeapon(USurvivorWeaponDefinition* WeaponDefinition)
{
	if (!WeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon failed because WeaponDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	if (HasWeapon(WeaponDefinition))
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("GrantWeapon skipped because weapon is already granted. Owner=%s Weapon=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition));
		return false;
	}

	FString ValidationError;
	if (!WeaponDefinition->ValidateRuntimeConfiguration(&ValidationError))
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon failed because weapon config is invalid. Owner=%s Weapon=%s Error=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition),
			*ValidationError);
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon failed because owner ASC is null. Owner=%s Weapon=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition));
		return false;
	}

	const UWorld* World = ASC->GetWorld();
	const AActor* OwnerActor = ASC->GetOwnerActor();
	const bool bCanGrantInStandalone = World == nullptr || World->GetNetMode() == NM_Standalone;
	const bool bHasAuthority = OwnerActor && OwnerActor->HasAuthority();
	if (!bHasAuthority && !bCanGrantInStandalone) //在 GAS 的底层架构中，赋予技能（Grant Ability）这个行为绝对只能在服务器（Server）上进行。
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon must run on authority. Owner=%s Weapon=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition));
		return false;
	}
	/*
	*这是 GAS 中最容易让新手迷惑，但也最精妙的设计。ASC 从不直接接收 UGameplayAbility 的实例，而是接收一个 Spec（规格说明书）。
	WeaponDefinition->WeaponAbility：这是技能的类（Class）。
	1：技能的等级（Level）。对于武器，可以用来控制伤害系数的成长。
	INDEX_NONE：输入绑定 ID。这里传入 INDEX_NONE 是因为这是一把通常靠代码自动触发（或后台轮询）的武器，而不是由玩家按左键触发的。
	WeaponDefinition (SourceObject)：重点在这里！ 我们把包含武器数据（比如射速、弹夹量）的数据资产作为 SourceObject 塞进了技能里。这样，当这把武器的 GA 运行时，可以随时通过 GetCurrentSourceObject() 拿到这把武器的具体数值，实现了“一套逻辑，百种武器”的彻底复用。
	 */
	const FGameplayAbilitySpec AbilitySpec(WeaponDefinition->WeaponAbility, 1, INDEX_NONE, WeaponDefinition);
	const FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(AbilitySpec); //接口应用：GiveAbility。它将组装好的 Spec 注册进角色的 ASC 中（确切地说，是添加进 ActivatableAbilities 数组）。引擎不会返回给你这个技能的指针，而是返回一个轻量级的唯一 ID（Handle）。之后你想升级这把武器、卸载这把武器、或者强制它开火，都要凭这个 Handle 来找 ASC 办事。
	if (!AbilityHandle.IsValid())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon failed to give ability. Owner=%s Weapon=%s Ability=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition),
			*GetNameSafe(WeaponDefinition->WeaponAbility));
		return false;
	}

	FSurvivorGrantedWeaponEntry& GrantedEntry = GrantedWeapons.AddDefaulted_GetRef();
	GrantedEntry.WeaponDefinition = WeaponDefinition;
	GrantedEntry.WeaponAbilityHandle = AbilityHandle;

	UE_LOG(LogSurvivorArena, Log, TEXT("Weapon granted. Owner=%s WeaponId=%s Ability=%s"),
		*GetNameSafe(GetOwner()),
		*WeaponDefinition->WeaponId.ToString(),
		*GetNameSafe(WeaponDefinition->WeaponAbility));

	if (WeaponDefinition->bAutoActivateOnGrant)
	{
		const bool bActivated = ASC->TryActivateAbility(AbilityHandle); //TryActivateAbility。常用于这种生存类 Demo 中的被动武器或光环武器。一捡起装备（Grant），立刻启动（Activate），并且这个 GA 内部通常是一个无限循环的 WaitDelay 节点，源源不断地生成子弹。
		if (!bActivated) //如果因为某种原因（比如角色正处于被沉默的 Tag 状态下）导致光环启动失败，代码非常严谨地调用了 ClearAbility，把刚才赋予的技能强行扣除，并清除了内存记录，防止产生“无效且占坑”的幽灵武器。
		{
			ASC->ClearAbility(AbilityHandle);
			GrantedWeapons.RemoveAll([WeaponDefinition](const FSurvivorGrantedWeaponEntry& Entry)
			{
				return Entry.WeaponDefinition == WeaponDefinition;
			});

			UE_LOG(LogSurvivorArena, Error, TEXT("GrantWeapon failed to auto-activate weapon ability. Owner=%s WeaponId=%s"),
				*GetNameSafe(GetOwner()),
				*WeaponDefinition->WeaponId.ToString());
			return false;
		}

		UE_LOG(LogSurvivorArena, Log, TEXT("Weapon auto-activated. Owner=%s WeaponId=%s"),
			*GetNameSafe(GetOwner()),
			*WeaponDefinition->WeaponId.ToString());
	}

	return true;
}

bool USurvivorWeaponManagerComponent::RemoveWeapon(USurvivorWeaponDefinition* WeaponDefinition)
{
	if (!WeaponDefinition)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("RemoveWeapon failed because WeaponDefinition is null. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}

	FSurvivorGrantedWeaponEntry* ExistingEntry = FindGrantedWeaponEntry(WeaponDefinition);
	if (!ExistingEntry)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("RemoveWeapon skipped because weapon is not granted. Owner=%s Weapon=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition));
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("RemoveWeapon failed because owner ASC is null. Owner=%s Weapon=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WeaponDefinition));
		return false;
	}

	if (ExistingEntry->WeaponAbilityHandle.IsValid())
	{
		ASC->CancelAbilityHandle(ExistingEntry->WeaponAbilityHandle);
		ASC->ClearAbility(ExistingEntry->WeaponAbilityHandle);
	}

	GrantedWeapons.RemoveAll([WeaponDefinition](const FSurvivorGrantedWeaponEntry& Entry)
	{
		return Entry.WeaponDefinition == WeaponDefinition;
	});

	UE_LOG(LogSurvivorArena, Log, TEXT("Weapon removed. Owner=%s WeaponId=%s"),
		*GetNameSafe(GetOwner()),
		*WeaponDefinition->WeaponId.ToString());

	return true;
}

bool USurvivorWeaponManagerComponent::HasWeapon(const USurvivorWeaponDefinition* WeaponDefinition) const
{
	return FindGrantedWeaponEntry(WeaponDefinition) != nullptr;
}

UAbilitySystemComponent* USurvivorWeaponManagerComponent::GetOwningAbilitySystemComponent() const
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	return AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
}

FSurvivorGrantedWeaponEntry* USurvivorWeaponManagerComponent::FindGrantedWeaponEntry(USurvivorWeaponDefinition* WeaponDefinition)
{
	return GrantedWeapons.FindByPredicate([WeaponDefinition](const FSurvivorGrantedWeaponEntry& Entry)
	{
		return Entry.WeaponDefinition == WeaponDefinition;
	});
}

const FSurvivorGrantedWeaponEntry* USurvivorWeaponManagerComponent::FindGrantedWeaponEntry(const USurvivorWeaponDefinition* WeaponDefinition) const
{
	return GrantedWeapons.FindByPredicate([WeaponDefinition](const FSurvivorGrantedWeaponEntry& Entry)
	{
		return Entry.WeaponDefinition == WeaponDefinition;
	});
}

void USurvivorWeaponManagerComponent::RemoveAllGrantedWeapons()
{
	if (!GetOwningAbilitySystemComponent())
	{
		GrantedWeapons.Reset();
		return;
	}

	TArray<TObjectPtr<USurvivorWeaponDefinition>> WeaponDefinitionsToRemove;
	WeaponDefinitionsToRemove.Reserve(GrantedWeapons.Num());

	for (const FSurvivorGrantedWeaponEntry& Entry : GrantedWeapons)
	{
		WeaponDefinitionsToRemove.Add(Entry.WeaponDefinition);
	}

	for (USurvivorWeaponDefinition* WeaponDefinition : WeaponDefinitionsToRemove)
	{
		RemoveWeapon(WeaponDefinition);
	}
}
