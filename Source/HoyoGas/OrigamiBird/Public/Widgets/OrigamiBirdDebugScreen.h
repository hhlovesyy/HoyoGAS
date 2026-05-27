#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyScreenBase.h"
#include "OrigamiBirdDebugScreen.generated.h"

class UButton;
class UListView;
class UOrigamiBirdMatchSubsystem;
class UTextBlock;

// 折纸小鸟活动的开发 GM 窗口。
// 第一版只做“从道具表选择道具 -> 给予当前对局”，后面可以继续加跳关、刷新棋盘、强制胜利等按钮。
UCLASS()
class HOYOGAS_API UOrigamiBirdDebugScreen : public UMyScreenBase
{
	GENERATED_BODY()

public:
	UOrigamiBirdDebugScreen(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual void NativeOnActivated() override;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> PropDefinitionListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> GrantOneButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> GrantFiveButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DebugStatusText;

private:
	UFUNCTION()
	void HandleGrantOneClicked();

	UFUNCTION()
	void HandleGrantFiveClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void HandlePropSelectionChanged(UObject* Item);
	void RefreshPropDefinitionList();
	void GrantSelectedProp(int32 Count);
	void SetDebugStatus(const FText& InText);
	UOrigamiBirdMatchSubsystem* GetMatchSubsystem() const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> PropDefinitionEntries;

	FName SelectedPropId = NAME_None;
};
