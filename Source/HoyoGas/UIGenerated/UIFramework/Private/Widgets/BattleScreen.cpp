#include "Widgets/BattleScreen.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Widget.h"
#include "CommonInputModeTypes.h"
#include "CommonTextBlock.h"
#include "Stores/BattleUIStore.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "VM_BattleScreen.h"

UBattleScreen::UBattleScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Game;
}

void UBattleScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AttackButton)
	{
		AttackButton->OnClicked.RemoveDynamic(this, &UBattleScreen::HandleAttackClicked);
		AttackButton->OnClicked.AddDynamic(this, &UBattleScreen::HandleAttackClicked);
	}

	if (GuardButton)
	{
		GuardButton->OnClicked.RemoveDynamic(this, &UBattleScreen::HandleGuardClicked);
		GuardButton->OnClicked.AddDynamic(this, &UBattleScreen::HandleGuardClicked);
	}

	if (PersonaButton)
	{
		PersonaButton->OnClicked.RemoveDynamic(this, &UBattleScreen::HandlePersonaClicked);
		PersonaButton->OnClicked.AddDynamic(this, &UBattleScreen::HandlePersonaClicked);
	}

	if (SkillButton)
	{
		SkillButton->OnClicked.RemoveDynamic(this, &UBattleScreen::HandleSkillClicked);
		SkillButton->OnClicked.AddDynamic(this, &UBattleScreen::HandleSkillClicked);
	}

	if (RootCanvas)
	{
		RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (CommandPanel)
	{
		CommandPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (TargetInfoPanel)
	{
		TargetInfoPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	UWidget* NonInteractiveWidgets[] =
	{
		CommandHintText,
		CurrentActorText,
		EncounterTitleText,
		ReticleDebugText,
		TargetAffinityText,
		TargetHintText,
		TargetLevelText,
		TargetNameText,
		TargetStateText
	};

	for (UWidget* Widget : NonInteractiveWidgets)
	{
		if (Widget)
		{
			Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

}

UObject* UBattleScreen::CreateViewModelInstance()
{
	return NewObject<UVM_BattleScreen>(this);
}

void UBattleScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_BattleScreen* ScreenViewModel = Cast<UVM_BattleScreen>(ViewModel))
	{
		ScreenViewModel->Initialize(StoreSubsystem ? StoreSubsystem->GetStore<UBattleUIStore>() : nullptr);
	}
}

void UBattleScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_BattleScreen* ScreenViewModel = Cast<UVM_BattleScreen>(ViewModel))
	{
		ScreenViewModel->Teardown();
	}
}

TOptional<FUIInputConfig> UBattleScreen::GetDesiredInputConfig() const
{
	FUIInputConfig InputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock, false);
	InputConfig.bIgnoreMoveInput = false;
	InputConfig.bIgnoreLookInput = false;
	return InputConfig;
}

UWidget* UBattleScreen::NativeGetDesiredFocusTarget() const
{
	return nullptr;
}

UVM_BattleScreen* UBattleScreen::GetScreenViewModel() const
{
	return Cast<UVM_BattleScreen>(GetViewModelObject());
}

void UBattleScreen::HandleAttackClicked()
{
	// TODO: Implement 'HandleAttackClicked' for widget 'AttackButton'.
}

void UBattleScreen::HandleGuardClicked()
{
	// TODO: Implement 'HandleGuardClicked' for widget 'GuardButton'.
}

void UBattleScreen::HandlePersonaClicked()
{
	// TODO: Implement 'HandlePersonaClicked' for widget 'PersonaButton'.
}

void UBattleScreen::HandleSkillClicked()
{
	// TODO: Implement 'HandleSkillClicked' for widget 'SkillButton'.
}
