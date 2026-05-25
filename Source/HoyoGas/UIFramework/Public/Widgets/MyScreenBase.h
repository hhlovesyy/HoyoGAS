#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UIFrameworkTypes.h"
#include "MyScreenBase.generated.h"

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UMyScreenBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMyScreenBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	virtual void SetPayload(const FMyUIPayload& InPayload);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	virtual void RequestClose();

	UFUNCTION(BlueprintPure, Category = "UIFramework")
	FName GetScreenTag() const;

	UFUNCTION(BlueprintPure, Category = "UIFramework")
	EMyUILayer GetPreferredLayer() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UIFramework")
	bool CanBeClosed() const;
	virtual bool CanBeClosed_Implementation() const;

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;

	void ConfigureScreen(FName InScreenTag, EMyUILayer InPreferredLayer);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework")
	FName ScreenTag = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UIFramework")
	EMyUILayer PreferredLayer = EMyUILayer::Menu;

	UPROPERTY(BlueprintReadOnly, Category = "UIFramework")
	FMyUIPayload CachedPayload;

	UFUNCTION(BlueprintImplementableEvent, Category = "UIFramework")
	void BP_OnScreenActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "UIFramework")
	void BP_OnScreenDeactivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "UIFramework")
	void BP_OnPayloadSet(const FMyUIPayload& Payload);
};
