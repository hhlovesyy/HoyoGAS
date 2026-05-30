#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "OrigamiBirdTileVisualWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class USizeBox;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdTileVisualClickedEvent, FIntPoint, BoardPosition);

// 单个三消方块的视觉 Widget。
// 它把“点击输入”和“图标/底色/选中态表现”封装在一个 UserWidget 里，
// 避免继续把 UButton 背景色当成方块表现层。
UCLASS(BlueprintType)
class HOYOGAS_API UOrigamiBirdTileVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	// 用逻辑层 Tile 和 UI 已解析好的图标/颜色刷新表现。
	// 图标资源加载仍由 VM/BoardWidget 做，这个 Widget 只负责表现。
	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void ApplyTileVisual(const FOrigamiBirdTile& InTile, UTexture2D* InIconTexture, FLinearColor InDebugColor);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	bool IsSelected() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void SetInputEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void SetTileSize(float InTileSize);

	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	int32 GetTileId() const;

	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	FIntPoint GetBoardPosition() const;

	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	EOrigamiBirdTileType GetTileType() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void ResetVisualTransform();

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdTileVisualClickedEvent OnTileClicked;

protected:
	// 可选绑定：后续 JSON/UMG 里按这些名字创建即可自动接上。
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> HitButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UBorder> SelectionBorder;

private:
	UFUNCTION()
	void HandleHitButtonClicked();

	void RefreshVisualState();

	UPROPERTY(Transient)
	FOrigamiBirdTile Tile;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(Transient)
	FLinearColor DebugColor = FLinearColor::White;

	UPROPERTY(Transient)
	bool bSelected = false;
};
