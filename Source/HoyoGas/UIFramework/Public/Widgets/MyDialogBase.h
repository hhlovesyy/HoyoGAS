#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyScreenBase.h"
#include "MyDialogBase.generated.h"

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UMyDialogBase : public UMyScreenBase
{
	GENERATED_BODY()

public:
	UMyDialogBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	virtual void SetupDialog(FMyConfirmDialogConfig InConfig);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void SetResultCallback(FMyDialogResultDelegate InCallback);

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UIFramework")
	FMyConfirmDialogConfig DialogConfig;

	FMyDialogResultDelegate ResultCallback;

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void NotifyDialogResult(bool bConfirmed);

	UFUNCTION(BlueprintImplementableEvent, Category = "UIFramework")
	void BP_OnDialogSetup(const FMyConfirmDialogConfig& InDialogConfig);
};
