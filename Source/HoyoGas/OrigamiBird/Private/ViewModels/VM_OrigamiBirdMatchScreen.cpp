#include "ViewModels/VM_OrigamiBirdMatchScreen.h"

#include "Core/OrigamiBirdMatchGameObject.h"
#include "Subsystems/OrigamiBirdMatchSubsystem.h"
#include "ViewModels/VM_OrigamiBirdPropEntry.h"

void UVM_OrigamiBirdMatchScreen::Initialize(UOrigamiBirdMatchSubsystem* InMatchSubsystem)
{
	MatchSubsystem = InMatchSubsystem;
	StartDefaultMatch();
}

void UVM_OrigamiBirdMatchScreen::Teardown()
{
	UnbindActiveMatch();

	MatchSubsystem = nullptr;
	CurrentSnapshot = FOrigamiBirdBoardSnapshot();
	LastMoveResult = FOrigamiBirdMoveResult();
	bHasSelectedPosition = false;
	SetCanInteract(false);
	SetPropEntries({});
}

void UVM_OrigamiBirdMatchScreen::StartDefaultMatch()
{
	if (!MatchSubsystem.IsValid())
	{
		SetStatusText(NSLOCTEXT("OrigamiBird", "NoMatchSubsystem", "Match subsystem is unavailable."));
		SetCanInteract(false);
		return;
	}

	UnbindActiveMatch();

	UOrigamiBirdMatchGameObject* NewMatch = MatchSubsystem->StartDefaultDevelopmentMatch();
	if (!NewMatch)
	{
		SetStatusText(NSLOCTEXT("OrigamiBird", "StartMatchFailed", "Failed to start match."));
		SetCanInteract(false);
		SetPropEntries({});
		return;
	}

	BindActiveMatch(NewMatch);
	RefreshFromSnapshot(NewMatch->GetSnapshot());
	RefreshPropEntries();

	SetStatusText(NSLOCTEXT("OrigamiBird", "MatchReady", "Ready"));
	SetCanInteract(true);
}

void UVM_OrigamiBirdMatchScreen::RestartMatch()
{
	StartDefaultMatch();
}

bool UVM_OrigamiBirdMatchScreen::SelectOrSwapTile(FIntPoint BoardPosition)
{
	LastMoveResult = FOrigamiBirdMoveResult();

	if (!bCanInteract || !ActiveMatch.IsValid())
	{
		return false;
	}

	if (!bHasSelectedPosition)
	{
		bHasSelectedPosition = true;
		SelectedBoardPosition = BoardPosition;
		ActiveMatch->SelectTile(BoardPosition);
		return false;
	}

	if (SelectedBoardPosition == BoardPosition)
	{
		ActiveMatch->SelectTile(BoardPosition);
		return false;
	}

	if (!IsAdjacentToSelected(BoardPosition))
	{
		SelectedBoardPosition = BoardPosition;
		ActiveMatch->SelectTile(BoardPosition);
		return false;
	}

	const FIntPoint From = SelectedBoardPosition;
	bHasSelectedPosition = false;
	SelectedBoardPosition = FIntPoint(INDEX_NONE, INDEX_NONE);

	const bool bSwapped = ActiveMatch->TrySwapTilesWithResult(From, BoardPosition, LastMoveResult);
	SetStatusText(bSwapped
		? NSLOCTEXT("OrigamiBird", "SwapAccepted", "Matched")
		: NSLOCTEXT("OrigamiBird", "SwapRejected", "Invalid swap"));

	return bSwapped;
}

FText UVM_OrigamiBirdMatchScreen::GetScoreText() const
{
	return ScoreText;
}

void UVM_OrigamiBirdMatchScreen::SetScoreText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ScoreText, InValue);
}

FText UVM_OrigamiBirdMatchScreen::GetMovesText() const
{
	return MovesText;
}

void UVM_OrigamiBirdMatchScreen::SetMovesText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(MovesText, InValue);
}

FText UVM_OrigamiBirdMatchScreen::GetStatusText() const
{
	return StatusText;
}

void UVM_OrigamiBirdMatchScreen::SetStatusText(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(StatusText, InValue);
}

bool UVM_OrigamiBirdMatchScreen::GetCanInteract() const
{
	return bCanInteract;
}

void UVM_OrigamiBirdMatchScreen::SetCanInteract(bool bInValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bCanInteract, bInValue);
}

const FOrigamiBirdBoardSnapshot& UVM_OrigamiBirdMatchScreen::GetCurrentSnapshot() const
{
	return CurrentSnapshot;
}

const FOrigamiBirdMoveResult& UVM_OrigamiBirdMatchScreen::GetLastMoveResult() const
{
	return LastMoveResult;
}

const TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>>& UVM_OrigamiBirdMatchScreen::GetPropEntries() const
{
	return PropEntries;
}

void UVM_OrigamiBirdMatchScreen::SetPropEntries(const TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>>& InEntries)
{
	PropEntries = InEntries;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(PropEntries);
}

void UVM_OrigamiBirdMatchScreen::HandleBoardChanged(const FOrigamiBirdBoardSnapshot& Snapshot)
{
	RefreshFromSnapshot(Snapshot);

	if (Snapshot.Phase == EOrigamiBirdMatchPhase::GameEnded)
	{
		SetStatusText(NSLOCTEXT("OrigamiBird", "GameEnded", "Game ended"));
		SetCanInteract(false);
	}
}

void UVM_OrigamiBirdMatchScreen::HandlePropStacksChanged(const TArray<FOrigamiBirdPropStack>& PropStacks)
{
	(void)PropStacks;
	RefreshPropEntries();
}

void UVM_OrigamiBirdMatchScreen::BindActiveMatch(UOrigamiBirdMatchGameObject* InMatch)
{
	ActiveMatch = InMatch;
	if (InMatch)
	{
		InMatch->OnBoardChanged.RemoveDynamic(this, &UVM_OrigamiBirdMatchScreen::HandleBoardChanged);
		InMatch->OnBoardChanged.AddDynamic(this, &UVM_OrigamiBirdMatchScreen::HandleBoardChanged);
		InMatch->OnPropStacksChanged.RemoveDynamic(this, &UVM_OrigamiBirdMatchScreen::HandlePropStacksChanged);
		InMatch->OnPropStacksChanged.AddDynamic(this, &UVM_OrigamiBirdMatchScreen::HandlePropStacksChanged);
	}
}

void UVM_OrigamiBirdMatchScreen::UnbindActiveMatch()
{
	if (ActiveMatch.IsValid())
	{
		ActiveMatch->OnBoardChanged.RemoveDynamic(this, &UVM_OrigamiBirdMatchScreen::HandleBoardChanged);
		ActiveMatch->OnPropStacksChanged.RemoveDynamic(this, &UVM_OrigamiBirdMatchScreen::HandlePropStacksChanged);
	}
	ActiveMatch = nullptr;
}

void UVM_OrigamiBirdMatchScreen::RefreshFromSnapshot(const FOrigamiBirdBoardSnapshot& Snapshot)
{
	CurrentSnapshot = Snapshot;

	SetScoreText(FText::Format(
		NSLOCTEXT("OrigamiBird", "ScoreFormat", "Score: {0}"),
		FText::AsNumber(Snapshot.Score)));

	SetMovesText(FText::Format(
		NSLOCTEXT("OrigamiBird", "MovesFormat", "Moves: {0}"),
		FText::AsNumber(Snapshot.MovesRemaining)));
}

void UVM_OrigamiBirdMatchScreen::RefreshPropEntries()
{
	TArray<TObjectPtr<UVM_OrigamiBirdPropEntry>> NewEntries;

	if (!ActiveMatch.IsValid() || !MatchSubsystem.IsValid())
	{
		SetPropEntries(NewEntries);
		return;
	}

	TArray<FOrigamiBirdPropStack> PropStacks;
	ActiveMatch->GetPropStacks(PropStacks);

	NewEntries.Reserve(PropStacks.Num());
	for (const FOrigamiBirdPropStack& PropStack : PropStacks)
	{
		if (PropStack.PropId.IsNone() || PropStack.Count <= 0)
		{
			continue;
		}

		FOrigamiBirdPropDefinitionRow Definition;
		if (!MatchSubsystem->FindPropDefinition(PropStack.PropId, Definition))
		{
			continue;
		}

		UVM_OrigamiBirdPropEntry* Entry = NewObject<UVM_OrigamiBirdPropEntry>(this);
		Entry->InitializeFromDefinition(PropStack.PropId, Definition, PropStack.Count);
		NewEntries.Add(Entry);
	}

	SetPropEntries(NewEntries);
}

bool UVM_OrigamiBirdMatchScreen::IsAdjacentToSelected(FIntPoint BoardPosition) const
{
	if (!bHasSelectedPosition)
	{
		return false;
	}

	const int32 Distance =
		FMath::Abs(BoardPosition.X - SelectedBoardPosition.X)
		+ FMath::Abs(BoardPosition.Y - SelectedBoardPosition.Y);

	return Distance == 1;
}
