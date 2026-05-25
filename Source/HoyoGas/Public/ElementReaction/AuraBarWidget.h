#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraBarWidget.generated.h"

class UTextBlock;
class UProgressBar;
class AElementDummyTarget;

UCLASS()
class HOYOGAS_API UAuraBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetObservedTarget(AElementDummyTarget* InTarget);
	UFUNCTION(BlueprintCallable)
	void RefreshFromTarget();

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_AuraName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_Aura;

	UPROPERTY()
	TObjectPtr<AElementDummyTarget> ObservedTarget;
	
	virtual void NativeConstruct() override;
};