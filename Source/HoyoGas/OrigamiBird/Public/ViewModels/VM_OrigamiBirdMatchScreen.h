#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "MVVMViewModelBase.h"
#include "VM_OrigamiBirdMatchScreen.generated.h"

class UOrigamiBirdMatchGameObject;
class UOrigamiBirdMatchSubsystem;
class UVM_OrigamiBirdPropEntry;

// 折纸小鸟对对碰主界面的 ViewModel。
// 棋盘本体由 GameObject 负责，VM 只把分数、步数、道具列表等状态整理给 UI。
UCLASS(BlueprintType)
class HOYOGAS_API UVM_OrigamiBirdMatchScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UOrigamiBirdMatchSubsystem* InMatchSubsystem);
	void Teardown();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void StartDefaultMatch();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void RestartMatch();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool SelectOrSwapTile(FIntPoint BoardPosition);

	FText GetScoreText() const;
	void SetScoreText(const FText& InValue);

	FText GetMovesText() const;
	void SetMovesText(const FText& InValue);

	FText GetStatusText() const;
	void SetStatusText(const FText& InValue);

	bool GetCanInteract() const;
	void SetCanInteract(bool bInValue);

	const FOrigamiBirdBoardSnapshot& GetCurrentSnapshot() const;
	const FOrigamiBirdMoveResult& GetLastMoveResult() const;

	const TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>>& GetPropEntries() const;
	void SetPropEntries(const TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>>& InEntries);

	void SelectPropEntry(UVM_OrigamiBirdPropEntry* Entry);
	void ClearSelectedProp();
	bool HasSelectedProp() const;
	bool TryUseSelectedPropOnTile(FIntPoint BoardPosition, FOrigamiBirdPropUseResult& OutResult);
	bool TryUseSelectedPropWithoutTarget(FOrigamiBirdPropUseResult& OutResult);

private:
	UFUNCTION()
	void HandleBoardChanged(const FOrigamiBirdBoardSnapshot& Snapshot);

	UFUNCTION()
	void HandlePropStacksChanged(const TArray<FOrigamiBirdPropStack>& PropStacks);

	void BindActiveMatch(UOrigamiBirdMatchGameObject* InMatch);
	void UnbindActiveMatch();
	void RefreshFromSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot);
	void RefreshPropEntries();
	bool IsAdjacentToSelected(FIntPoint BoardPosition) const;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText ScoreText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText MovesText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText StatusText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetCanInteract", Setter = "SetCanInteract", meta = (AllowPrivateAccess = "true"))
	bool bCanInteract = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>> PropEntries;

	TWeakObjectPtr<UOrigamiBirdMatchSubsystem> MatchSubsystem;
	TWeakObjectPtr<UOrigamiBirdMatchGameObject> ActiveMatch;
	FOrigamiBirdBoardSnapshot CurrentSnapshot;
	FOrigamiBirdMoveResult LastMoveResult;

	FName SelectedPropId = NAME_None;
	EOrigamiBirdPropTargetType SelectedPropTargetType = EOrigamiBirdPropTargetType::None;
	TArray<int32> PendingPropTargetColumns;
	FIntPoint SelectedBoardPosition = FIntPoint(INDEX_NONE, INDEX_NONE);
	bool bHasSelectedPosition = false;
};
