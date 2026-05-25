#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyScreenBase.h"
#include "MyToastScreenBase.generated.h"

class UTextBlock;

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UMyToastScreenBase : public UMyScreenBase
{
	GENERATED_BODY()

public:
	UMyToastScreenBase(const FObjectInitializer& ObjectInitializer);

	virtual void SetPayload(const FMyUIPayload& InPayload) override;

	UFUNCTION(BlueprintCallable, Category = "UIFramework|Toast")
	void SetToastStackIndex(int32 InStackIndex);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "UIFramework|Toast")
	void RefreshToastFromPayload();

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "UIFramework|Toast")
	TObjectPtr<UTextBlock> ToastMessageText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework|Toast", meta = (ClampMin = "0.0"))
	float AutoCloseDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework|Toast")
	float StackTopOffset = 32.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework|Toast")
	float StackRightOffset = 32.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework|Toast")
	float StackItemSpacing = 84.0f;

private:
	void StartAutoCloseTimer();
	void ClearAutoCloseTimer();
	void ApplyToastStackPosition();

	FTimerHandle AutoCloseTimerHandle;
	int32 ToastStackIndex = 0;
};
