#pragma once

#include "CoreMinimal.h"
#include "FieldNotificationId.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "SurvivorHUDScreen.generated.h"

class UProgressBar;
class UTextBlock;
class UVM_SurvivorHUD;

UCLASS()
class HOYOGAS_API USurvivorHUDScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	USurvivorHUDScreen(const FObjectInitializer& ObjectInitializer);

	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, class UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;
	virtual void HandlePostViewModelAttached() override;
	virtual void HandlePreViewModelDetached(UObject* ViewModel) override;

	void RefreshFromViewModel();
	void HandleAnyHUDFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponCountText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ExperienceText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CardSlotsText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EquippedCardsText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CardTagsText;

	TArray<FDelegateHandle> FieldChangedHandles;
};
