#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElementReaction/ElementTypes.h"
#include "DamageTextWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
class UCanvasPanelSlot;

UCLASS()
class HOYOGAS_API UDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitDamage(float Damage, const FText& ReactionText, const FLinearColor& Color, EDamageTextStyleType AnimType);

	UFUNCTION(BlueprintCallable)
	void SetScreenPosition(FVector2D InScreenPosition);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleAnimFinished();

	void RequestClose();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Damage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Reaction;

	// 在蓝图里建同名动画
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Normal;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Vaporize;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Melt;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Frozen;

	EDamageTextStyleType CachedAnimType = EDamageTextStyleType::Normal;
};
