#pragma once

#include "Battle/HoyoBattleTypes.h"
#include "CoreMinimal.h"
#include "ElementReaction/ElementTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameplayDemoPlayerController.generated.h"

class AElementDummyTarget;
class AHoyoBattleTargetMarkerActor;
class AHoyoGasCharacter;
class UEnhancedInputLocalPlayerSubsystem;
class UHoyoEnemyInterface;
class UHoyoBattleFlowSubsystem;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class HOYOGAS_API AGameplayDemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
	void AddInputMappingContexts();
	void RemoveInputMappingContexts();
	void RefreshBattleTargetingInputMappingContext();
	void BindEnhancedInputActions();
	void BindGameplayDebugInput();
	void UnbindGameplayDebugInput();
	void FindDebugTarget();
	void TryInteract();
	AActor* FindBestInteractableActor() const;
	void ConfirmBattleTargetSelection();
	void HandleBattleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState);
	void SyncBattleInputMode();
	void EnsureBattleMarkerActors();
	void UpdateBattleTargeting();
	void UpdateHoveredEnemy();
	void SetHoveredEnemy(AActor* NewHoveredActor);
	void SetSelectedEnemy(AActor* NewSelectedActor);
	void RefreshEnemyHighlightState(AActor* EnemyActor) const;
	bool IsBattleTargetingActive() const;
	void RefreshBattleMarkerActors();
	void InitializeGameplayDemoUI();
	void EnsureGameplayDemoUIRootAndHUD();
	void RefreshGameplayDemoUIContext();
	void HandlePossessedPawnChanged();
	void ToggleInventoryScreen();
	void OpenPauseMenu();
	void OpenCharacterDetailPanel();
	void TestPyro(const FInputActionValue& Value);
	void TestHydro(const FInputActionValue& Value);
	void TestCryo(const FInputActionValue& Value);
	void SendElementToTarget(EGenshinElementType Element, float BaseDamage = 100.0f);

	TWeakObjectPtr<AElementDummyTarget> DebugTarget;
	TWeakObjectPtr<AActor> HoveredEnemyActor;
	TWeakObjectPtr<AActor> SelectedEnemyActor;
	TWeakObjectPtr<UHoyoBattleFlowSubsystem> CachedBattleFlowSubsystem;
	TWeakObjectPtr<AHoyoBattleTargetMarkerActor> HoveredTargetMarkerActor;
	TWeakObjectPtr<AHoyoBattleTargetMarkerActor> SelectedTargetMarkerActor;
	float InteractionSearchRadius = 250.0f;
	float EnemyHoverTraceDistance = 5000.0f;
	TSubclassOf<AHoyoBattleTargetMarkerActor> HoveredMarkerClass;
	TSubclassOf<AHoyoBattleTargetMarkerActor> SelectedMarkerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> GameplayDemoMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	int32 GameplayDemoMappingPriority = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> BattleTargetingMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mapping Contexts", meta = (AllowPrivateAccess = "true"))
	int32 BattleTargetingMappingPriority = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ToggleInventoryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OpenPauseMenuAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ConfirmBattleTargetAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OpenCharacterDetailsAction; 
	
	uint32 TestPyroBindingHandle = 0;
	uint32 TestHydroBindingHandle = 0;
	uint32 TestCryoBindingHandle = 0;
};
