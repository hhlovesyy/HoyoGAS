#include "Widgets/OrigamiBirdMatchScreen.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Engine/LocalPlayer.h"
#include "Input/Reply.h"
#include "INotifyFieldValueChanged.h"
#include "InputCoreTypes.h"
#include "Subsystems/OrigamiBirdMatchSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "ViewModels/VM_OrigamiBirdMatchScreen.h"
#include "ViewModels/VM_OrigamiBirdPropEntry.h"
#include "Widgets/OrigamiBirdBoardWidget.h"

UOrigamiBirdMatchScreen::UOrigamiBirdMatchScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Menu;
	SetIsFocusable(true);
}

void UOrigamiBirdMatchScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdMatchScreen::HandleRestartClicked);
		RestartButton->OnClicked.AddDynamic(this, &UOrigamiBirdMatchScreen::HandleRestartClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdMatchScreen::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UOrigamiBirdMatchScreen::HandleCloseClicked);
	}
}

UWidget* UOrigamiBirdMatchScreen::NativeGetDesiredFocusTarget() const
{
	return const_cast<UOrigamiBirdMatchScreen*>(this);
}

FReply UOrigamiBirdMatchScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::G)
	{
		return OpenDebugScreen() ? FReply::Handled() : FReply::Unhandled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

UObject* UOrigamiBirdMatchScreen::CreateViewModelInstance()
{
	return NewObject<UVM_OrigamiBirdMatchScreen>(this);
}

void UOrigamiBirdMatchScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	(void)StoreSubsystem;

	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = Cast<UVM_OrigamiBirdMatchScreen>(ViewModel))
	{
		MatchViewModel->Initialize(GetMatchSubsystem());
	}
}

void UOrigamiBirdMatchScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = Cast<UVM_OrigamiBirdMatchScreen>(ViewModel))
	{
		MatchViewModel->Teardown();
	}
}

void UOrigamiBirdMatchScreen::HandlePostViewModelAttached()
{
	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = GetMatchViewModel())
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UOrigamiBirdMatchScreen::HandleViewModelFieldChanged);
		MatchViewModel->AddFieldValueChangedDelegate(UVM_OrigamiBirdMatchScreen::FFieldNotificationClassDescriptor::PropEntries, Delegate);
	}

	if (BoardWidget)
	{
		BoardWidget->OnTileClicked.RemoveDynamic(this, &UOrigamiBirdMatchScreen::HandleTileClicked);
		BoardWidget->OnTileClicked.AddDynamic(this, &UOrigamiBirdMatchScreen::HandleTileClicked);
	}

	RefreshBoardFromViewModel();
	RefreshPropListFromViewModel();
}

void UOrigamiBirdMatchScreen::HandlePreViewModelDetached(UObject* ViewModel)
{
	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = Cast<UVM_OrigamiBirdMatchScreen>(ViewModel))
	{
		MatchViewModel->RemoveAllFieldValueChangedDelegates(this);
	}

	if (BoardWidget)
	{
		BoardWidget->OnTileClicked.RemoveDynamic(this, &UOrigamiBirdMatchScreen::HandleTileClicked);
	}
}

void UOrigamiBirdMatchScreen::HandlePostViewModelDetached()
{
	if (BoardWidget)
	{
		BoardWidget->ClearBoard();
	}
	if (PropListView)
	{
		PropListView->ClearListItems();
	}
}

void UOrigamiBirdMatchScreen::HandleRestartClicked()
{
	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = GetMatchViewModel())
	{
		MatchViewModel->RestartMatch();
		RefreshBoardFromViewModel();
		RefreshPropListFromViewModel();
	}
}

void UOrigamiBirdMatchScreen::HandleCloseClicked()
{
	RequestClose();
}

void UOrigamiBirdMatchScreen::HandleTileClicked(FIntPoint BoardPosition)
{
	UVM_OrigamiBirdMatchScreen* MatchViewModel = GetMatchViewModel();
	if (!MatchViewModel)
	{
		return;
	}

	const bool bResolvedMove = MatchViewModel->SelectOrSwapTile(BoardPosition);
	const FOrigamiBirdMoveResult& MoveResult = MatchViewModel->GetLastMoveResult();
	const bool bAttemptedSwap =
		MoveResult.From.X != INDEX_NONE
		&& MoveResult.From.Y != INDEX_NONE
		&& MoveResult.To.X != INDEX_NONE
		&& MoveResult.To.Y != INDEX_NONE;

	if (bAttemptedSwap && (bResolvedMove || MoveResult.FailureReasonId == TEXT("NoMatch")))
	{
		if (BoardWidget)
		{
			BoardWidget->PlayMoveResult(MoveResult);
		}
		return;
	}

	RefreshBoardFromViewModel();
}

void UOrigamiBirdMatchScreen::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;

	if (FieldId == UVM_OrigamiBirdMatchScreen::FFieldNotificationClassDescriptor::PropEntries)
	{
		RefreshPropListFromViewModel();
	}
}

void UOrigamiBirdMatchScreen::RefreshBoardFromViewModel()
{
	if (!BoardWidget)
	{
		return;
	}

	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = GetMatchViewModel())
	{
		BoardWidget->BuildFromSnapshot(MatchViewModel->GetCurrentSnapshot(), GetMatchSubsystem());
	}
}

void UOrigamiBirdMatchScreen::RefreshPropListFromViewModel()
{
	if (!PropListView)
	{
		return;
	}

	PropListView->ClearListItems();

	if (UVM_OrigamiBirdMatchScreen* MatchViewModel = GetMatchViewModel())
	{
		for (UVM_OrigamiBirdPropEntry* Entry : MatchViewModel->GetPropEntries())
		{
			if (Entry)
			{
				PropListView->AddItem(Entry);
			}
		}
	}
}

bool UOrigamiBirdMatchScreen::OpenDebugScreen()
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
		{
			return PlayerUISubsystem->OpenScreen(TEXT("OrigamiBirdDebug"), FMyUIPayload()) != nullptr;
		}
	}

	return false;
}

UOrigamiBirdMatchSubsystem* UOrigamiBirdMatchScreen::GetMatchSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return ULocalPlayer::GetSubsystem<UOrigamiBirdMatchSubsystem>(LocalPlayer);
	}

	return nullptr;
}

UVM_OrigamiBirdMatchScreen* UOrigamiBirdMatchScreen::GetMatchViewModel() const
{
	return Cast<UVM_OrigamiBirdMatchScreen>(GetViewModelObject());
}
