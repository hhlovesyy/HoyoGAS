#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyMVVMDialogBase.h"
#include "GameplayDemoLevelUpDialog.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class HOYOGAS_API UGameplayDemoLevelUpDialog : public UMyMVVMDialogBase
{
	GENERATED_BODY()

public:
	UGameplayDemoLevelUpDialog(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UFUNCTION()
	void HandleContinueClicked();

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, class UMyUIStoreSubsystem* StoreSubsystem) override;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ContinueButton;
};
