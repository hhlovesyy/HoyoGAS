#include "Stores/CharacterPanelUIStore.h"

#include "Components/CharacterEquipmentComponent.h"
#include "Data/CharacterPanelSettings.h"
#include "Data/CharacterGrowthSettings.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Progression/HoyoProgressSubsystem.h"

void UCharacterPanelUIStore::InitializeStore(ULocalPlayer* InOwningPlayer, FGameplayTag InStoreTag)
{
	Super::InitializeStore(InOwningPlayer, InStoreTag);
	BindProgressSubsystem();
}

void UCharacterPanelUIStore::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);
	BindProgressSubsystem();
	BindEquipmentComponent(InPlayerState);
	RebuildFromCharacterDefinitionTable();
}

void UCharacterPanelUIStore::UnbindFromPlayerContext()
{
	UnbindProgressSubsystem();
	UnbindEquipmentComponent();
	bHasCurrentCharacter = false;
	CurrentCharacter = FCharacterDefinitionRow();
	Super::UnbindFromPlayerContext();
	BroadcastStoreChanged();
}

void UCharacterPanelUIStore::SetCurrentCharacterId(FName InCharacterId)
{
	if (CurrentCharacterId == InCharacterId)
	{
		RebuildFromCharacterDefinitionTable();
		return;
	}

	CurrentCharacterId = InCharacterId;
	RebuildFromCharacterDefinitionTable();
}

FName UCharacterPanelUIStore::GetCurrentCharacterId() const
{
	return CurrentCharacterId;
}

const FCharacterDefinitionRow& UCharacterPanelUIStore::GetCurrentCharacter() const
{
	return CurrentCharacter;
}

bool UCharacterPanelUIStore::HasCurrentCharacter() const
{
	return bHasCurrentCharacter;
}

bool UCharacterPanelUIStore::IsStoryUnlocked(const FCharacterStoryEntry& Story) const
{
	if (!ProgressSubsystem.IsValid())
	{
		return Story.UnlockConditions.Conditions.IsEmpty();
	}

	return ProgressSubsystem->Evaluate(Story.UnlockConditions);
}

FText UCharacterPanelUIStore::GetStoryLockedReason(const FCharacterStoryEntry& Story) const
{
	return Story.LockedReasonText.IsEmpty()
		? NSLOCTEXT("CharacterPanel", "DefaultStoryLockedReason", "Story locked.")
		: Story.LockedReasonText;
}

bool UCharacterPanelUIStore::IsRelicDevelopmentActionEnabled() const
{
	return EquipmentComponent.IsValid() && EquipmentComponent->AreRelicDevelopmentActionsEnabled();
}

bool UCharacterPanelUIStore::GrantRelicDevelopmentLoadoutForCurrentCharacter(FText& OutStatusText)
{
	if (!EquipmentComponent.IsValid())
	{
		OutStatusText = NSLOCTEXT("CharacterPanel", "RelicDevNoEquipmentComponent", "Equipment component is not available.");
		return false;
	}

	if (!EquipmentComponent->AreRelicDevelopmentActionsEnabled())
	{
		OutStatusText = NSLOCTEXT("CharacterPanel", "RelicDevDisabled", "Relic development actions are disabled.");
		return false;
	}

	if (CurrentCharacterId.IsNone())
	{
		OutStatusText = NSLOCTEXT("CharacterPanel", "RelicDevNoCharacter", "No current character.");
		return false;
	}

	if (!EquipmentComponent->GrantRelicDevelopmentLoadout(CurrentCharacterId, true))
	{
		OutStatusText = NSLOCTEXT("CharacterPanel", "RelicDevNoChange", "No relic was generated from DataTable.");
		return false;
	}

	OutStatusText = NSLOCTEXT("CharacterPanel", "RelicDevGranted", "Development relics regenerated from DataTable.");
	BroadcastStoreChanged();
	return true;
}

bool UCharacterPanelUIStore::GetEquippedRelicInSlot(EHoyoRelicSlot Slot, FHoyoRelicInstance& OutRelic) const
{
	return EquipmentComponent.IsValid()
		&& !CurrentCharacterId.IsNone()
		&& EquipmentComponent->GetEquippedRelicInSlot(CurrentCharacterId, Slot, OutRelic);
}

void UCharacterPanelUIStore::GetActiveRelicSetBonuses(TArray<FHoyoRelicSetActivation>& OutActivations) const
{
	OutActivations.Reset();
	if (EquipmentComponent.IsValid() && !CurrentCharacterId.IsNone())
	{
		EquipmentComponent->GetActiveRelicSetBonuses(CurrentCharacterId, OutActivations);
	}
}

bool UCharacterPanelUIStore::GetRelicDefinition(FName RelicDefinitionId, FHoyoRelicDefinitionRow& OutDefinition) const
{
	return EquipmentComponent.IsValid() && EquipmentComponent->GetRelicDefinition(RelicDefinitionId, OutDefinition);
}

bool UCharacterPanelUIStore::GetRelicSetDefinition(FName SetId, FHoyoRelicSetDefinitionRow& OutDefinition) const
{
	return EquipmentComponent.IsValid() && EquipmentComponent->GetRelicSetDefinition(SetId, OutDefinition);
}

bool UCharacterPanelUIStore::GetRelicAffixDefinition(FName AffixId, FHoyoRelicAffixDefinitionRow& OutDefinition) const
{
	return EquipmentComponent.IsValid() && EquipmentComponent->GetRelicAffixDefinition(AffixId, OutDefinition);
}

void UCharacterPanelUIStore::RebuildFromCharacterDefinitionTable()
{
	const UCharacterPanelSettings* Settings = GetDefault<UCharacterPanelSettings>();
	const FName DesiredCharacterId = CurrentCharacterId.IsNone() && Settings
		? Settings->DefaultCharacterId
		: CurrentCharacterId;

	CurrentCharacterId = DesiredCharacterId.IsNone() ? TEXT("Trailblazer_Stelle") : DesiredCharacterId;

	FCharacterDefinitionRow LoadedRow;
	if (TryLoadCharacterDefinitionFromTable(CurrentCharacterId, LoadedRow))
	{
		CurrentCharacter = LoadedRow;
		bHasCurrentCharacter = true;
		AutoGrantRelicDevelopmentLoadoutIfNeeded();
		BroadcastStoreChanged();
		return;
	}

	CurrentCharacter = BuildFallbackCharacterDefinition();
	CurrentCharacterId = CurrentCharacter.CharacterId;
	bHasCurrentCharacter = true;
	AutoGrantRelicDevelopmentLoadoutIfNeeded();
	BroadcastStoreChanged();
}

FCharacterDefinitionRow UCharacterPanelUIStore::BuildFallbackCharacterDefinition() const
{
	FCharacterDefinitionRow Row;
	Row.CharacterId = TEXT("Trailblazer_Stelle");
	Row.DisplayName = NSLOCTEXT("CharacterPanel", "FallbackCharacterName", "Stelle");
	Row.Title = NSLOCTEXT("CharacterPanel", "FallbackCharacterTitle", "Trailblazer");
	Row.PathName = NSLOCTEXT("CharacterPanel", "FallbackPathName", "Destruction");
	Row.FactionName = NSLOCTEXT("CharacterPanel", "FallbackFactionName", "Astral Express");
	Row.ElementName = NSLOCTEXT("CharacterPanel", "FallbackElementName", "Physical");
	Row.FirstMetDateText = NSLOCTEXT("CharacterPanel", "FallbackFirstMet", "Herta Space Station");
	Row.Rarity = 5;
	Row.DefaultLevel = 1;
	Row.MaxLevel = 80;
	Row.ShortBio = NSLOCTEXT("CharacterPanel", "FallbackShortBio", "A traveler awakened at the space station, carrying the power of a Stellaron and searching for answers aboard the Astral Express.");

	FCharacterStoryEntry Story;
	Story.StoryId = TEXT("Story_01");
	Story.Title = NSLOCTEXT("CharacterPanel", "FallbackStoryTitle", "Character Story I");
	Story.RichTextBody = NSLOCTEXT("CharacterPanel", "FallbackStoryBody", "<Title>Awakening</>\nShe opened her eyes with no clear memory of the past. The journey ahead became the first answer she could choose for herself.");
	Row.Stories.Add(Story);

	return Row;
}

bool UCharacterPanelUIStore::TryLoadCharacterDefinitionFromTable(FName CharacterId, FCharacterDefinitionRow& OutRow) const
{
	if (CharacterId.IsNone())
	{
		return false;
	}

	const UCharacterPanelSettings* Settings = GetDefault<UCharacterPanelSettings>();
	if (!Settings || Settings->CharacterDefinitionTable.IsNull())
	{
		return false;
	}

	UDataTable* CharacterTable = Settings->CharacterDefinitionTable.LoadSynchronous();
	if (!CharacterTable)
	{
		return false;
	}

	if (const FCharacterDefinitionRow* FoundRow = CharacterTable->FindRow<FCharacterDefinitionRow>(CharacterId, TEXT("CharacterPanelUIStore::TryLoadCharacterDefinitionFromTable")))
	{
		OutRow = *FoundRow;
		return true;
	}

	return false;
}

void UCharacterPanelUIStore::BindProgressSubsystem()
{
	UHoyoProgressSubsystem* NewProgressSubsystem = nullptr;

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			NewProgressSubsystem = GameInstance->GetSubsystem<UHoyoProgressSubsystem>();
		}
	}

	if (ProgressSubsystem.Get() == NewProgressSubsystem)
	{
		return;
	}

	UnbindProgressSubsystem();
	ProgressSubsystem = NewProgressSubsystem;
	if (ProgressSubsystem.IsValid())
	{
		ProgressChangedHandle = ProgressSubsystem->OnProgressChanged().AddUObject(this, &UCharacterPanelUIStore::HandleProgressChanged);
	}
}

void UCharacterPanelUIStore::UnbindProgressSubsystem()
{
	if (ProgressSubsystem.IsValid() && ProgressChangedHandle.IsValid())
	{
		ProgressSubsystem->OnProgressChanged().Remove(ProgressChangedHandle);
	}

	ProgressChangedHandle.Reset();
	ProgressSubsystem.Reset();
}

void UCharacterPanelUIStore::BindEquipmentComponent(APlayerState* InPlayerState)
{
	UCharacterEquipmentComponent* NewEquipmentComponent = InPlayerState
		? InPlayerState->FindComponentByClass<UCharacterEquipmentComponent>()
		: nullptr;

	if (EquipmentComponent.Get() == NewEquipmentComponent)
	{
		return;
	}

	UnbindEquipmentComponent();
	EquipmentComponent = NewEquipmentComponent;
	if (EquipmentComponent.IsValid())
	{
		EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UCharacterPanelUIStore::HandleEquipmentChanged);
	}
}

void UCharacterPanelUIStore::UnbindEquipmentComponent()
{
	if (EquipmentComponent.IsValid())
	{
		EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UCharacterPanelUIStore::HandleEquipmentChanged);
	}

	EquipmentComponent.Reset();
}

void UCharacterPanelUIStore::AutoGrantRelicDevelopmentLoadoutIfNeeded()
{
#if UE_BUILD_SHIPPING
	return;
#else
	const UCharacterGrowthSettings* Settings = GetDefault<UCharacterGrowthSettings>();
	if (!Settings || !Settings->bEnableRelicDevelopmentActions || !Settings->bAutoGrantRelicDevelopmentLoadoutOnCharacterPanelOpen)
	{
		return;
	}

	if (!EquipmentComponent.IsValid() || CurrentCharacterId.IsNone())
	{
		return;
	}

	// 开发期便捷入口：打开角色面板时自动填充空槽，避免每次都手动拉蓝图节点测试。
	// bReplaceExisting=false，已有遗器不会被覆盖；接存档后可以直接关掉 settings 开关。
	EquipmentComponent->GrantRelicDevelopmentLoadout(CurrentCharacterId, false);
#endif
}

void UCharacterPanelUIStore::HandleProgressChanged()
{
	BroadcastStoreChanged();
}

void UCharacterPanelUIStore::HandleEquipmentChanged(FName CharacterId)
{
	if (CharacterId.IsNone() || CharacterId == CurrentCharacterId)
	{
		BroadcastStoreChanged();
	}
}
