#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/Function.h"
#include "HoyoWidgetTweenLibrary.generated.h"

class UWidget;

UENUM(BlueprintType)
enum class EHoyoWidgetTweenEasing : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHoyoWidgetTweenEvent);

// 一个正在播放的 Widget Tween。
// 调用方可以保存它，用于取消动画或监听结束事件。
UCLASS(BlueprintType)
class HOYOGAS_API UHoyoWidgetTweenHandle : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hoyo|UI|Tween")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "Hoyo|UI|Tween")
	bool IsActive() const;

	UPROPERTY(BlueprintAssignable, Category = "Hoyo|UI|Tween")
	FHoyoWidgetTweenEvent OnFinished;

	UPROPERTY(BlueprintAssignable, Category = "Hoyo|UI|Tween")
	FHoyoWidgetTweenEvent OnCancelled;

private:
	friend class UHoyoWidgetTweenLibrary;

	void Start(
		UWidget* InTargetWidget,
		float InDurationSeconds,
		EHoyoWidgetTweenEasing InEasing,
		float InEaseExponent,
		TFunction<void(float)> InTickFunction,
		TFunction<void()> InFinishFunction);

	void Tick();
	void Finish(bool bCancelled);
	float EvaluateEasing(float Alpha) const;

	TWeakObjectPtr<UWidget> TargetWidget;
	FTimerHandle TimerHandle;
	TFunction<void(float)> TickFunction;
	TFunction<void()> FinishFunction;
	double StartTimeSeconds = 0.0;
	float DurationSeconds = 0.0f;
	float EaseExponent = 2.0f;
	EHoyoWidgetTweenEasing Easing = EHoyoWidgetTweenEasing::EaseOut;
	bool bActive = false;
};

// 通用 Widget Tween 工具。
// 这是表现层工具，不应该包含任何具体玩法逻辑。
UCLASS()
class HOYOGAS_API UHoyoWidgetTweenLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hoyo|UI|Tween")
	static UHoyoWidgetTweenHandle* TweenRenderTranslation(
		UWidget* Widget,
		FVector2D From,
		FVector2D To,
		float DurationSeconds,
		EHoyoWidgetTweenEasing Easing = EHoyoWidgetTweenEasing::EaseOut,
		float EaseExponent = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Hoyo|UI|Tween")
	static UHoyoWidgetTweenHandle* TweenRenderScale(
		UWidget* Widget,
		FVector2D From,
		FVector2D To,
		float DurationSeconds,
		EHoyoWidgetTweenEasing Easing = EHoyoWidgetTweenEasing::EaseOut,
		float EaseExponent = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Hoyo|UI|Tween")
	static UHoyoWidgetTweenHandle* TweenRenderOpacity(
		UWidget* Widget,
		float From,
		float To,
		float DurationSeconds,
		EHoyoWidgetTweenEasing Easing = EHoyoWidgetTweenEasing::EaseOut,
		float EaseExponent = 2.0f);
};
