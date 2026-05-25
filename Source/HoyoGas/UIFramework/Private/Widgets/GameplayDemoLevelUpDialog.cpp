#include "Widgets/GameplayDemoLevelUpDialog.h"

#include "Components/Button.h"
#include "Stores/ProgressionUIStore.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "VM_LevelUpDialog.h"

UGameplayDemoLevelUpDialog::UGameplayDemoLevelUpDialog(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Modal;
}

void UGameplayDemoLevelUpDialog::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.RemoveDynamic(this, &UGameplayDemoLevelUpDialog::HandleContinueClicked);
		ContinueButton->OnClicked.AddDynamic(this, &UGameplayDemoLevelUpDialog::HandleContinueClicked);
	}
}

UObject* UGameplayDemoLevelUpDialog::CreateViewModelInstance()
{
	return NewObject<UVM_LevelUpDialog>(this);
}

void UGameplayDemoLevelUpDialog::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_LevelUpDialog* LevelUpViewModel = Cast<UVM_LevelUpDialog>(ViewModel))
	{
		LevelUpViewModel->Initialize(StoreSubsystem ? StoreSubsystem->GetStore<UProgressionUIStore>() : nullptr);
	}
}

UWidget* UGameplayDemoLevelUpDialog::NativeGetDesiredFocusTarget() const
{
	return ContinueButton;
}

void UGameplayDemoLevelUpDialog::HandleContinueClicked()
{
	RequestClose();
}
