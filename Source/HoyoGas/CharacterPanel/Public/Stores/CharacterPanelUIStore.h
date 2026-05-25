#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterDefinitionRow.h"
#include "Data/HoyoRelicTypes.h"
#include "Stores/MyUIStoreBase.h"
#include "CharacterPanelUIStore.generated.h"

class UCharacterEquipmentComponent;
class APlayerState;

UCLASS(BlueprintType)
class HOYOGAS_API UCharacterPanelUIStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void InitializeStore(ULocalPlayer* InOwningPlayer, FGameplayTag InStoreTag) override;
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	UFUNCTION(BlueprintCallable, Category = "CharacterPanel")
	void SetCurrentCharacterId(FName InCharacterId);

	UFUNCTION(BlueprintPure, Category = "CharacterPanel")
	FName GetCurrentCharacterId() const;

	const FCharacterDefinitionRow& GetCurrentCharacter() const;
	bool HasCurrentCharacter() const;
	bool IsStoryUnlocked(const FCharacterStoryEntry& Story) const;
	FText GetStoryLockedReason(const FCharacterStoryEntry& Story) const;
	bool IsRelicDevelopmentActionEnabled() const;
	bool GrantRelicDevelopmentLoadoutForCurrentCharacter(FText& OutStatusText);
	bool GetEquippedRelicInSlot(EHoyoRelicSlot Slot, FHoyoRelicInstance& OutRelic) const;
	void GetActiveRelicSetBonuses(TArray<FHoyoRelicSetActivation>& OutActivations) const;
	bool GetRelicDefinition(FName RelicDefinitionId, FHoyoRelicDefinitionRow& OutDefinition) const;
	bool GetRelicSetDefinition(FName SetId, FHoyoRelicSetDefinitionRow& OutDefinition) const;
	bool GetRelicAffixDefinition(FName AffixId, FHoyoRelicAffixDefinitionRow& OutDefinition) const;

	void RebuildFromCharacterDefinitionTable();

private:
	FCharacterDefinitionRow BuildFallbackCharacterDefinition() const;
	bool TryLoadCharacterDefinitionFromTable(FName CharacterId, FCharacterDefinitionRow& OutRow) const;
	void BindProgressSubsystem();
	void UnbindProgressSubsystem();
	void BindEquipmentComponent(APlayerState* InPlayerState);
	void UnbindEquipmentComponent();
	void AutoGrantRelicDevelopmentLoadoutIfNeeded();
	void HandleProgressChanged();

	UFUNCTION()
	void HandleEquipmentChanged(FName CharacterId);

	UPROPERTY(Transient)
	FName CurrentCharacterId = NAME_None;

	UPROPERTY(Transient)
	FCharacterDefinitionRow CurrentCharacter;

	UPROPERTY(Transient)
	bool bHasCurrentCharacter = false;

	TWeakObjectPtr<class UHoyoProgressSubsystem> ProgressSubsystem;
	FDelegateHandle ProgressChangedHandle;

	TWeakObjectPtr<UCharacterEquipmentComponent> EquipmentComponent;
};
