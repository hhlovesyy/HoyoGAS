#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "OrigamiBirdBoardWidget.generated.h"

class UOrigamiBirdMatchSubsystem;
class UOrigamiBirdTileVisualWidget;
class UUniformGridPanel;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrigamiBirdBoardTileClickedEvent, FIntPoint, BoardPosition);

// 三消棋盘表现层。
// 它负责把逻辑层的 Snapshot / ActionResult 转成格子 Widget 的创建、移动、消失和生成表现。
UCLASS(BlueprintType)
class HOYOGAS_API UOrigamiBirdBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void BuildFromSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot, UOrigamiBirdMatchSubsystem* InMatchSubsystem);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void PlayActionResult(const FOrigamiBirdActionResult& ActionResult);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void PlayPresentationTimeline(const FOrigamiBirdPresentationTimeline& Timeline, const FOrigamiBirdBoardSnapshot& InitialSnapshot);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void ClearBoard();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void SetBoardInputEnabled(bool bInEnabled);

	UPROPERTY(BlueprintAssignable, Category = "OrigamiBird")
	FOrigamiBirdBoardTileClickedEvent OnTileClicked;

protected:
	// JSON/UMG 里可以放一个同名 UniformGridPanel。没有的话，C++ 会生成一个默认棋盘根节点。
	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UUniformGridPanel> BoardGrid;

	// 后续如果你用 JSON/UMG 做更漂亮的单格子布局，可以在这里换成对应 Widget Class。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OrigamiBird")
	TSubclassOf<UOrigamiBirdTileVisualWidget> TileVisualWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OrigamiBird", meta = (ClampMin = "16.0"))
	float TileCellSize = 96.0f;

private:
	UFUNCTION()
	void HandleTileVisualClicked(FIntPoint BoardPosition);

	UFUNCTION()
	void HandlePresentationEventDelayFinished();

	void EnsureDefaultBoardTree();
	void RebuildPositionMap();
	void ReconcileWithSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot);
	void PlayNextPresentationEvent();
	void PlayPresentationEvent(const FOrigamiBirdPresentationEvent& Event);
	void FinishPresentationTimeline();

	UOrigamiBirdTileVisualWidget* CreateTileVisual(const FOrigamiBirdTile& Tile);
	UOrigamiBirdTileVisualWidget* FindTileVisualById(int32 TileId) const;
	void AddTileVisualToGrid(UOrigamiBirdTileVisualWidget* TileVisual, FIntPoint BoardPosition);
	void MoveTileVisualToGridPosition(UOrigamiBirdTileVisualWidget* TileVisual, FIntPoint BoardPosition);
	void RemoveTileVisual(int32 TileId);

	void ResolveTileVisualData(EOrigamiBirdTileType TileType, UTexture2D*& OutIconTexture, FLinearColor& OutDebugColor) const;
	FVector2D GetRenderOffsetBetween(FIntPoint FromPosition, FIntPoint ToPosition) const;

	UPROPERTY(Transient)
	TObjectPtr<UOrigamiBirdMatchSubsystem> MatchSubsystem;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UOrigamiBirdTileVisualWidget>> TileWidgetsById;

	TMap<FIntPoint, int32> TileIdByPosition;
	FOrigamiBirdPresentationTimeline PendingPresentationTimeline;
	TArray<int32> TileIdsPendingRemoval;
	FTimerHandle PresentationEventTimerHandle;
	int32 PendingPresentationEventIndex = 0;
	bool bBoardInputEnabled = true;
};
