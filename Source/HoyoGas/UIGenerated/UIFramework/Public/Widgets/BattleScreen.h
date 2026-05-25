#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "BattleScreen.generated.h"

class UMyUIStoreSubsystem;
class UBorder;
class UButton;
class UCanvasPanel;
class UCommonTextBlock;
class UVM_BattleScreen;

UCLASS()
class HOYOGAS_API UBattleScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	UBattleScreen(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;

private:
	UFUNCTION()
	void HandleAttackClicked();
	UFUNCTION()
	void HandleGuardClicked();
	UFUNCTION()
	void HandlePersonaClicked();
	UFUNCTION()
	void HandleSkillClicked();

	UVM_BattleScreen* GetScreenViewModel() const;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> AttackButton;
	// SemanticRole: battle.command.attack

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> CommandHintText;
	// SemanticRole: battle.commandHint

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> CommandPanel;
	// SemanticRole: battle.commandPanel

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> CurrentActorText;
	// SemanticRole: battle.currentActor

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> EncounterTitleText;
	// SemanticRole: battle.encounterTitle

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> GuardButton;
	// SemanticRole: battle.command.guard

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> PersonaButton;
	// SemanticRole: battle.command.persona

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> ReticleDebugText;
	// SemanticRole: battle.reticleDebug

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> SkillButton;
	// SemanticRole: battle.command.skill

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> TargetAffinityText;
	// SemanticRole: battle.targetAffinity

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> TargetHintText;
	// SemanticRole: battle.targetHint

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> TargetInfoPanel;
	// SemanticRole: battle.targetInfo

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> TargetLevelText;
	// SemanticRole: battle.targetLevel

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> TargetNameText;
	// SemanticRole: battle.targetName

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCommonTextBlock> TargetStateText;
	// SemanticRole: battle.targetState

};
