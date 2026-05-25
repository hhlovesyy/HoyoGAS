#include "Components/CharacterEquipmentComponent.h"

#include "Data/CharacterGrowthSettings.h"
#include "Engine/DataTable.h"

namespace
{
const TCHAR* RelicContext = TEXT("CharacterEquipmentComponent");
}

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCharacterEquipmentComponent::AddRelicInstance(FHoyoRelicInstance RelicInstance)
{
	if (RelicInstance.RelicDefinitionId.IsNone())
	{
		return false;
	}

	if (!NormalizeRelicFromDefinition(RelicInstance))
	{
		return false;
	}

	if (!RelicInstance.InstanceId.IsValid())
	{
		RelicInstance.InstanceId = FGuid::NewGuid();
	}

	if (FindRelicInventoryIndex(RelicInstance.InstanceId) != INDEX_NONE)
	{
		return false;
	}

	RelicInstance.EquippedCharacterId = NAME_None;
	RelicInventory.Add(RelicInstance);
	return true;
}

bool UCharacterEquipmentComponent::RemoveRelicInstance(FGuid RelicInstanceId)
{
	const int32 RelicIndex = FindRelicInventoryIndex(RelicInstanceId);
	if (RelicIndex == INDEX_NONE)
	{
		return false;
	}

	FName PreviousCharacterId = NAME_None;
	ClearRelicFromLoadouts(RelicInstanceId, PreviousCharacterId);
	RelicInventory.RemoveAt(RelicIndex);

	if (!PreviousCharacterId.IsNone())
	{
		OnEquipmentChanged.Broadcast(PreviousCharacterId);
	}

	return true;
}

bool UCharacterEquipmentComponent::EquipRelic(FName CharacterId, FGuid RelicInstanceId)
{
	/*
	*装备遗器。它做几件事：
	•检查角色 ID
	•检查遗器是否存在
	•从表里校验遗器定义
	•如果这件遗器穿在别人身上，先卸下来
	•如果目标槽位已有遗器，旧的卸下
	•更新 EquippedCharacterId
	•广播 OnEquipmentChanged
	 */
	if (CharacterId.IsNone() || !RelicInstanceId.IsValid())
	{
		return false;
	}

	const int32 RelicIndex = FindRelicInventoryIndex(RelicInstanceId);
	if (RelicIndex == INDEX_NONE)
	{
		return false;
	}

	FHoyoRelicInstance& RelicToEquip = RelicInventory[RelicIndex];
	if (!NormalizeRelicFromDefinition(RelicToEquip))
	{
		return false;
	}

	FName PreviousCharacterId = NAME_None;
	ClearRelicFromLoadouts(RelicInstanceId, PreviousCharacterId);

	FHoyoCharacterRelicLoadout& Loadout = FindOrAddLoadout(CharacterId);
	FHoyoEquippedRelicSlot& EquippedSlot = FindOrAddEquippedSlot(Loadout, RelicToEquip.Slot);

	if (EquippedSlot.RelicInstanceId.IsValid() && EquippedSlot.RelicInstanceId != RelicInstanceId)
	{
		const int32 PreviousRelicIndex = FindRelicInventoryIndex(EquippedSlot.RelicInstanceId);
		if (PreviousRelicIndex != INDEX_NONE)
		{
			RelicInventory[PreviousRelicIndex].EquippedCharacterId = NAME_None;
		}
	}

	EquippedSlot.RelicInstanceId = RelicInstanceId;
	RelicToEquip.EquippedCharacterId = CharacterId;

	if (!PreviousCharacterId.IsNone() && PreviousCharacterId != CharacterId)
	{
		OnEquipmentChanged.Broadcast(PreviousCharacterId);
	}

	OnEquipmentChanged.Broadcast(CharacterId);
	return true;
}

bool UCharacterEquipmentComponent::UnequipRelic(FName CharacterId, EHoyoRelicSlot Slot)
{
	const int32 LoadoutIndex = FindLoadoutIndex(CharacterId);
	if (LoadoutIndex == INDEX_NONE)
	{
		return false;
	}

	FHoyoCharacterRelicLoadout& Loadout = CharacterRelicLoadouts[LoadoutIndex];
	for (FHoyoEquippedRelicSlot& EquippedSlot : Loadout.EquippedSlots)
	{
		if (EquippedSlot.Slot != Slot || !EquippedSlot.RelicInstanceId.IsValid())
		{
			continue;
		}

		const int32 RelicIndex = FindRelicInventoryIndex(EquippedSlot.RelicInstanceId);
		if (RelicIndex != INDEX_NONE)
		{
			RelicInventory[RelicIndex].EquippedCharacterId = NAME_None;
		}

		EquippedSlot.RelicInstanceId.Invalidate();
		OnEquipmentChanged.Broadcast(CharacterId);
		return true;
	}

	return false;
}

void UCharacterEquipmentComponent::GetAllRelics(TArray<FHoyoRelicInstance>& OutRelics) const
{
	OutRelics = RelicInventory;
}

bool UCharacterEquipmentComponent::GetRelicInstance(FGuid RelicInstanceId, FHoyoRelicInstance& OutRelic) const
{
	const int32 RelicIndex = FindRelicInventoryIndex(RelicInstanceId);
	if (RelicIndex == INDEX_NONE)
	{
		return false;
	}

	OutRelic = RelicInventory[RelicIndex];
	return true;
}

void UCharacterEquipmentComponent::GetEquippedRelics(FName CharacterId, TArray<FHoyoRelicInstance>& OutRelics) const
{
	OutRelics.Reset();

	const int32 LoadoutIndex = FindLoadoutIndex(CharacterId);
	if (LoadoutIndex == INDEX_NONE)
	{
		return;
	}

	for (const FHoyoEquippedRelicSlot& EquippedSlot : CharacterRelicLoadouts[LoadoutIndex].EquippedSlots)
	{
		FHoyoRelicInstance RelicInstance;
		if (GetRelicInstance(EquippedSlot.RelicInstanceId, RelicInstance))
		{
			OutRelics.Add(RelicInstance);
		}
	}
}

bool UCharacterEquipmentComponent::GetEquippedRelicInSlot(FName CharacterId, EHoyoRelicSlot Slot, FHoyoRelicInstance& OutRelic) const
{
	const int32 LoadoutIndex = FindLoadoutIndex(CharacterId);
	if (LoadoutIndex == INDEX_NONE)
	{
		return false;
	}

	for (const FHoyoEquippedRelicSlot& EquippedSlot : CharacterRelicLoadouts[LoadoutIndex].EquippedSlots)
	{
		if (EquippedSlot.Slot == Slot)
		{
			return GetRelicInstance(EquippedSlot.RelicInstanceId, OutRelic);
		}
	}

	return false;
}

void UCharacterEquipmentComponent::GetActiveRelicSetBonuses(FName CharacterId, TArray<FHoyoRelicSetActivation>& OutActivations) const
{
	OutActivations.Reset();

	TArray<FHoyoRelicInstance> EquippedRelics;
	GetEquippedRelics(CharacterId, EquippedRelics);

	TMap<FName, int32> SetPieceCounts;
	for (const FHoyoRelicInstance& RelicInstance : EquippedRelics)
	{
		if (!RelicInstance.SetId.IsNone())
		{
			int32& Count = SetPieceCounts.FindOrAdd(RelicInstance.SetId);
			++Count;
		}
	}

	for (const TPair<FName, int32>& Pair : SetPieceCounts)
	{
		FHoyoRelicSetDefinitionRow SetDefinition;
		if (!GetRelicSetDefinition(Pair.Key, SetDefinition))
		{
			continue;
		}

		FHoyoRelicSetActivation Activation;
		Activation.SetId = Pair.Key;
		Activation.EquippedPieceCount = Pair.Value;

		for (const FHoyoRelicSetBonusDefinition& Bonus : SetDefinition.Bonuses)
		{
			if (Pair.Value >= Bonus.RequiredPieceCount)
			{
				Activation.ActiveBonusPieceCounts.Add(Bonus.RequiredPieceCount);
			}
		}

		Activation.ActiveBonusPieceCounts.Sort();
		OutActivations.Add(Activation);
	}

	OutActivations.Sort([](const FHoyoRelicSetActivation& A, const FHoyoRelicSetActivation& B)
	{
		return A.SetId.LexicalLess(B.SetId);
	});
}

bool UCharacterEquipmentComponent::GrantRelicDevelopmentLoadout(FName CharacterId, bool bReplaceExisting)
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (CharacterId.IsNone() || !AreRelicDevelopmentActionsEnabled())
	{
		return false;
	}

	UDataTable* RelicTable = LoadRelicDefinitionTable();
	if (!RelicTable)
	{
		return false;
	}

	TArray<FHoyoRelicDefinitionRow*> RelicDefinitions;
	RelicTable->GetAllRows<FHoyoRelicDefinitionRow>(RelicContext, RelicDefinitions);

	TSet<EHoyoRelicSlot> FilledSlots;
	for (FHoyoRelicDefinitionRow* RelicDefinition : RelicDefinitions)
	{
		if (!RelicDefinition || FilledSlots.Contains(RelicDefinition->Slot))
		{
			continue;
		}

		FHoyoRelicInstance ExistingRelic;
		if (!bReplaceExisting && GetEquippedRelicInSlot(CharacterId, RelicDefinition->Slot, ExistingRelic))
		{
			FilledSlots.Add(RelicDefinition->Slot);
			continue;
		}

		FHoyoRelicInstance NewRelicInstance;
		if (!BuildDevelopmentRelicInstance(*RelicDefinition, NewRelicInstance))
		{
			continue;
		}

		if (AddRelicInstance(NewRelicInstance) && EquipRelic(CharacterId, NewRelicInstance.InstanceId))
		{
			FilledSlots.Add(RelicDefinition->Slot);
		}
	}

	return FilledSlots.Num() > 0;
#endif
}

bool UCharacterEquipmentComponent::AreRelicDevelopmentActionsEnabled() const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UCharacterGrowthSettings* Settings = GetDefault<UCharacterGrowthSettings>();
	return Settings && Settings->bEnableRelicDevelopmentActions;
#endif
}

bool UCharacterEquipmentComponent::GetRelicDefinition(FName RelicDefinitionId, FHoyoRelicDefinitionRow& OutDefinition) const
{
	if (RelicDefinitionId.IsNone())
	{
		return false;
	}

	const UDataTable* Table = LoadRelicDefinitionTable();
	if (!Table)
	{
		return false;
	}

	if (const FHoyoRelicDefinitionRow* Row = Table->FindRow<FHoyoRelicDefinitionRow>(RelicDefinitionId, RelicContext, false))
	{
		OutDefinition = *Row;
		return true;
	}

	return false;
}

bool UCharacterEquipmentComponent::GetRelicSetDefinition(FName SetId, FHoyoRelicSetDefinitionRow& OutDefinition) const
{
	if (SetId.IsNone())
	{
		return false;
	}

	const UDataTable* Table = LoadRelicSetDefinitionTable();
	if (!Table)
	{
		return false;
	}

	if (const FHoyoRelicSetDefinitionRow* Row = Table->FindRow<FHoyoRelicSetDefinitionRow>(SetId, RelicContext, false))
	{
		OutDefinition = *Row;
		return true;
	}

	return false;
}

bool UCharacterEquipmentComponent::GetRelicAffixDefinition(FName AffixId, FHoyoRelicAffixDefinitionRow& OutDefinition) const
{
	if (AffixId.IsNone())
	{
		return false;
	}

	const UDataTable* Table = LoadRelicAffixDefinitionTable();
	if (!Table)
	{
		return false;
	}

	if (const FHoyoRelicAffixDefinitionRow* Row = Table->FindRow<FHoyoRelicAffixDefinitionRow>(AffixId, RelicContext, false))
	{
		OutDefinition = *Row;
		return true;
	}

	return false;
}

int32 UCharacterEquipmentComponent::FindRelicInventoryIndex(FGuid RelicInstanceId) const
{
	if (!RelicInstanceId.IsValid())
	{
		return INDEX_NONE;
	}

	return RelicInventory.IndexOfByPredicate([RelicInstanceId](const FHoyoRelicInstance& RelicInstance)
	{
		return RelicInstance.InstanceId == RelicInstanceId;
	});
}

int32 UCharacterEquipmentComponent::FindLoadoutIndex(FName CharacterId) const
{
	if (CharacterId.IsNone())
	{
		return INDEX_NONE;
	}

	return CharacterRelicLoadouts.IndexOfByPredicate([CharacterId](const FHoyoCharacterRelicLoadout& Loadout)
	{
		return Loadout.CharacterId == CharacterId;
	});
}

FHoyoCharacterRelicLoadout& UCharacterEquipmentComponent::FindOrAddLoadout(FName CharacterId)
{
	const int32 ExistingIndex = FindLoadoutIndex(CharacterId);
	if (ExistingIndex != INDEX_NONE)
	{
		return CharacterRelicLoadouts[ExistingIndex];
	}

	FHoyoCharacterRelicLoadout& NewLoadout = CharacterRelicLoadouts.AddDefaulted_GetRef();
	NewLoadout.CharacterId = CharacterId;
	return NewLoadout;
}

FHoyoEquippedRelicSlot& UCharacterEquipmentComponent::FindOrAddEquippedSlot(FHoyoCharacterRelicLoadout& Loadout, EHoyoRelicSlot Slot)
{
	for (FHoyoEquippedRelicSlot& EquippedSlot : Loadout.EquippedSlots)
	{
		if (EquippedSlot.Slot == Slot)
		{
			return EquippedSlot;
		}
	}

	FHoyoEquippedRelicSlot& NewSlot = Loadout.EquippedSlots.AddDefaulted_GetRef();
	NewSlot.Slot = Slot;
	return NewSlot;
}

void UCharacterEquipmentComponent::ClearRelicFromLoadouts(FGuid RelicInstanceId, FName& OutPreviousCharacterId)
{
	OutPreviousCharacterId = NAME_None;

	for (FHoyoCharacterRelicLoadout& Loadout : CharacterRelicLoadouts)
	{
		for (FHoyoEquippedRelicSlot& EquippedSlot : Loadout.EquippedSlots)
		{
			if (EquippedSlot.RelicInstanceId == RelicInstanceId)
			{
				EquippedSlot.RelicInstanceId.Invalidate();
				OutPreviousCharacterId = Loadout.CharacterId;
			}
		}
	}
}

bool UCharacterEquipmentComponent::NormalizeRelicFromDefinition(FHoyoRelicInstance& RelicInstance) const
{
	FHoyoRelicDefinitionRow RelicDefinition;
	if (!GetRelicDefinition(RelicInstance.RelicDefinitionId, RelicDefinition))
	{
		return false;
	}

	RelicInstance.SetId = RelicDefinition.SetId;
	RelicInstance.Slot = RelicDefinition.Slot;
	RelicInstance.Rarity = RelicDefinition.Rarity;
	return true;
}

bool UCharacterEquipmentComponent::BuildDevelopmentRelicInstance(const FHoyoRelicDefinitionRow& RelicDefinition, FHoyoRelicInstance& OutRelicInstance) const
{
	if (RelicDefinition.RelicDefinitionId.IsNone() || RelicDefinition.AllowedMainAffixIds.IsEmpty())
	{
		return false;
	}

	OutRelicInstance = FHoyoRelicInstance();
	OutRelicInstance.InstanceId = FGuid::NewGuid();
	OutRelicInstance.RelicDefinitionId = RelicDefinition.RelicDefinitionId;
	OutRelicInstance.SetId = RelicDefinition.SetId;
	OutRelicInstance.Slot = RelicDefinition.Slot;
	OutRelicInstance.Rarity = RelicDefinition.Rarity;
	OutRelicInstance.Level = RelicDefinition.Rarity >= 5 ? 15 : 12;
	OutRelicInstance.MainAffix.AffixId = RelicDefinition.AllowedMainAffixIds[0];

	if (RelicDefinition.Slot != EHoyoRelicSlot::Head && RelicDefinition.Slot != EHoyoRelicSlot::Hands)
	{
		for (const FName CandidateAffixId : RelicDefinition.AllowedMainAffixIds)
		{
			FHoyoRelicAffixDefinitionRow AffixDefinition;
			if (GetRelicAffixDefinition(CandidateAffixId, AffixDefinition)
				&& AffixDefinition.ValueType == EHoyoRelicStatValueType::Percent)
			{
				OutRelicInstance.MainAffix.AffixId = CandidateAffixId;
				break;
			}
		}
	}

	switch (RelicDefinition.Slot)
	{
	case EHoyoRelicSlot::Head:
		OutRelicInstance.MainAffix.Value = 705.0f;
		break;
	case EHoyoRelicSlot::Hands:
		OutRelicInstance.MainAffix.Value = 352.0f;
		break;
	case EHoyoRelicSlot::LinkRope:
		OutRelicInstance.MainAffix.Value = 0.194f;
		break;
	default:
		OutRelicInstance.MainAffix.Value = 0.432f;
		break;
	}

	auto AddSubAffix = [&OutRelicInstance](FName AffixId, float Value, int32 UpgradeCount)
	{
		if (AffixId.IsNone() || OutRelicInstance.MainAffix.AffixId == AffixId)
		{
			return;
		}

		FHoyoRelicAffixInstance& SubAffix = OutRelicInstance.SubAffixes.AddDefaulted_GetRef();
		SubAffix.AffixId = AffixId;
		SubAffix.Value = Value;
		SubAffix.UpgradeCount = UpgradeCount;
	};

	AddSubAffix(TEXT("CritRate"), 0.058f, 1);
	AddSubAffix(TEXT("CritDamage"), 0.116f, 1);
	AddSubAffix(TEXT("AttackPercent"), 0.086f, 0);
	AddSubAffix(TEXT("AttackFlat"), 38.0f, 0);

	return true;
}

UDataTable* UCharacterEquipmentComponent::LoadRelicDefinitionTable() const
{
	const UCharacterGrowthSettings* Settings = GetDefault<UCharacterGrowthSettings>();
	return Settings ? Settings->RelicDefinitionTable.LoadSynchronous() : nullptr;
}

UDataTable* UCharacterEquipmentComponent::LoadRelicSetDefinitionTable() const
{
	const UCharacterGrowthSettings* Settings = GetDefault<UCharacterGrowthSettings>();
	return Settings ? Settings->RelicSetDefinitionTable.LoadSynchronous() : nullptr;
}

UDataTable* UCharacterEquipmentComponent::LoadRelicAffixDefinitionTable() const
{
	const UCharacterGrowthSettings* Settings = GetDefault<UCharacterGrowthSettings>();
	return Settings ? Settings->RelicAffixDefinitionTable.LoadSynchronous() : nullptr;
}
