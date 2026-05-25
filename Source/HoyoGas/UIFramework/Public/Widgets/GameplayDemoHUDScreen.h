#pragma once

#include "CoreMinimal.h"
#include "FieldNotificationId.h"
#include "TimerManager.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "GameplayDemoHUDScreen.generated.h"

class UBorder;
class UProgressBar;
class UTextBlock;

UCLASS()
class HOYOGAS_API UGameplayDemoHUDScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	UGameplayDemoHUDScreen(const FObjectInitializer& ObjectInitializer);

	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, class UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;
	virtual void HandlePostViewModelAttached() override;
	virtual void HandlePreViewModelDetached(UObject* ViewModel) override;

	void ResetHealthTween(float InHealthPercent);
	void ApplyHealthPercent(float InHealthPercent);
	void StartHealthTweenTimer();
	void StopHealthTweenTimer();
	void HandleHealthTweenTimer();
	void UpdateHealthTween(float InDeltaTime);
	void HandleHealthPercentChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RemainingText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TotalValueText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthTrailProgressBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Health")
	float HealthIncreaseInterpSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Health")
	float HealthTrailDecreaseInterpSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Health")
	float HealthTrailDelay = 0.35f;

	bool bHasHealthTweenState = false;
	float HealthTargetPercent = 0.0f;
	float HealthFillPercent = 0.0f;
	float HealthTrailPercent = 0.0f;
	float HealthTrailDelayRemaining = 0.0f;

	FDelegateHandle HealthPercentChangedHandle;
	FTimerHandle HealthTweenTimerHandle;
};
