#pragma once

#include "CoreMinimal.h"
#include "Battle/HoyoBattleTypes.h"
#include "Stores/MyUIStoreBase.h"
#include "BattleUIStore.generated.h"

class AActor;
class UHoyoBattleEncounterDefinition;
class UHoyoBattleFlowSubsystem;

UCLASS(BlueprintType)
class HOYOGAS_API UBattleUIStore : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	void BindBattleSources();
	void UnbindBattleSources();

	const FText& GetEncounterTitleText() const;
	const FText& GetTargetStateText() const;
	const FText& GetTargetNameText() const;
	const FText& GetTargetLevelText() const;
	const FText& GetTargetAffinityText() const;
	const FText& GetTargetHintText() const;
	const FText& GetCurrentActorText() const;
	const FText& GetCommandHintText() const;
	const FText& GetReticleDebugText() const;

	AActor* GetHoveredEnemyActor() const;
	AActor* GetSelectedEnemyActor() const;

private:
	void RefreshFromBattleFlow();
	void ResetPresentationState();
	FText BuildEncounterTitleText(const UHoyoBattleEncounterDefinition* EncounterDefinition) const;
	AActor* ResolvePrimaryTargetActor() const;

	void HandleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState);
	void HandleEncounterChanged(const UHoyoBattleEncounterDefinition* EncounterDefinition);
	void HandleHoveredEnemyChanged(AActor* PreviousEnemyActor, AActor* NewEnemyActor);
	void HandleSelectedEnemyChanged(AActor* PreviousEnemyActor, AActor* NewEnemyActor);

	UPROPERTY(Transient)
	TWeakObjectPtr<UHoyoBattleFlowSubsystem> BattleFlowSubsystem;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> HoveredEnemyActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SelectedEnemyActor;

	UPROPERTY(Transient)
	FText EncounterTitleText;

	UPROPERTY(Transient)
	FText TargetStateText;

	UPROPERTY(Transient)
	FText TargetNameText;

	UPROPERTY(Transient)
	FText TargetLevelText;

	UPROPERTY(Transient)
	FText TargetAffinityText;

	UPROPERTY(Transient)
	FText TargetHintText;

	UPROPERTY(Transient)
	FText CurrentActorText;

	UPROPERTY(Transient)
	FText CommandHintText;

	UPROPERTY(Transient)
	FText ReticleDebugText;
};
