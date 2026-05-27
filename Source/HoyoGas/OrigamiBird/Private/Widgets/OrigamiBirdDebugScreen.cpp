#include "Widgets/OrigamiBirdDebugScreen.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/OrigamiBirdMatchSubsystem.h"
#include "ViewModels/VM_OrigamiBirdPropEntry.h"

UOrigamiBirdDebugScreen::UOrigamiBirdDebugScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Modal;
}

void UOrigamiBirdDebugScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (PropDefinitionListView)
	{
		PropDefinitionListView->OnItemSelectionChanged().RemoveAll(this);
		PropDefinitionListView->OnItemSelectionChanged().AddUObject(this, &UOrigamiBirdDebugScreen::HandlePropSelectionChanged);
	}

	if (GrantOneButton)
	{
		GrantOneButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdDebugScreen::HandleGrantOneClicked);
		GrantOneButton->OnClicked.AddDynamic(this, &UOrigamiBirdDebugScreen::HandleGrantOneClicked);
	}

	if (GrantFiveButton)
	{
		GrantFiveButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdDebugScreen::HandleGrantFiveClicked);
		GrantFiveButton->OnClicked.AddDynamic(this, &UOrigamiBirdDebugScreen::HandleGrantFiveClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdDebugScreen::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UOrigamiBirdDebugScreen::HandleCloseClicked);
	}
}

void UOrigamiBirdDebugScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshPropDefinitionList();
}

UWidget* UOrigamiBirdDebugScreen::NativeGetDesiredFocusTarget() const
{
	return PropDefinitionListView ? Cast<UWidget>(PropDefinitionListView) : Cast<UWidget>(CloseButton);
}

void UOrigamiBirdDebugScreen::HandleGrantOneClicked()
{
	GrantSelectedProp(1);
}

void UOrigamiBirdDebugScreen::HandleGrantFiveClicked()
{
	GrantSelectedProp(5);
}

void UOrigamiBirdDebugScreen::HandleCloseClicked()
{
	RequestClose();
}

void UOrigamiBirdDebugScreen::HandlePropSelectionChanged(UObject* Item)
{
	const UVM_OrigamiBirdPropEntry* Entry = Cast<UVM_OrigamiBirdPropEntry>(Item);
	SelectedPropId = Entry ? Entry->GetPropId() : NAME_None;

	SetDebugStatus(SelectedPropId.IsNone()
		? NSLOCTEXT("OrigamiBird", "DebugNoPropSelected", "No prop selected.")
		: FText::Format(NSLOCTEXT("OrigamiBird", "DebugSelectedProp", "Selected: {0}"), FText::FromName(SelectedPropId)));
}

void UOrigamiBirdDebugScreen::RefreshPropDefinitionList()
{
	PropDefinitionEntries.Reset();
	SelectedPropId = NAME_None;

	if (PropDefinitionListView)
	{
		PropDefinitionListView->ClearListItems();
	}

	UOrigamiBirdMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!MatchSubsystem)
	{
		SetDebugStatus(NSLOCTEXT("OrigamiBird", "DebugNoSubsystem", "Match subsystem is unavailable."));
		return;
	}

	TArray<FName> PropIds;
	MatchSubsystem->GetAllPropIds(PropIds);
	if (PropIds.IsEmpty())
	{
		SetDebugStatus(NSLOCTEXT("OrigamiBird", "DebugNoPropDefinitions", "No prop definitions. Check PropDefinitionTable."));
		return;
	}

	for (const FName PropId : PropIds)
	{
		FOrigamiBirdPropDefinitionRow Definition;
		if (!MatchSubsystem->FindPropDefinition(PropId, Definition))
		{
			continue;
		}

		UVM_OrigamiBirdPropEntry* Entry = NewObject<UVM_OrigamiBirdPropEntry>(this);
		Entry->InitializeFromDefinition(PropId, Definition, Definition.InitialCount);
		PropDefinitionEntries.Add(Entry);

		if (PropDefinitionListView)
		{
			PropDefinitionListView->AddItem(Entry);
		}
	}

	SetDebugStatus(NSLOCTEXT("OrigamiBird", "DebugSelectProp", "Select a prop, then grant it to the active match."));
}

void UOrigamiBirdDebugScreen::GrantSelectedProp(int32 Count)
{
	if (SelectedPropId.IsNone())
	{
		SetDebugStatus(NSLOCTEXT("OrigamiBird", "DebugGrantNoSelection", "Select a prop first."));
		return;
	}

	UOrigamiBirdMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!MatchSubsystem)
	{
		SetDebugStatus(NSLOCTEXT("OrigamiBird", "DebugGrantNoSubsystem", "Match subsystem is unavailable."));
		return;
	}

	if (MatchSubsystem->GrantPropToActiveMatch(SelectedPropId, Count))
	{
		SetDebugStatus(FText::Format(
			NSLOCTEXT("OrigamiBird", "DebugGrantSucceeded", "Granted {0} x{1}."),
			FText::FromName(SelectedPropId),
			FText::AsNumber(Count)));
	}
	else
	{
		SetDebugStatus(FText::Format(
			NSLOCTEXT("OrigamiBird", "DebugGrantFailed", "Failed to grant {0}. Start a match and check the prop table."),
			FText::FromName(SelectedPropId)));
	}
}

void UOrigamiBirdDebugScreen::SetDebugStatus(const FText& InText)
{
	if (DebugStatusText)
	{
		DebugStatusText->SetText(InText);
	}
}

UOrigamiBirdMatchSubsystem* UOrigamiBirdDebugScreen::GetMatchSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return ULocalPlayer::GetSubsystem<UOrigamiBirdMatchSubsystem>(LocalPlayer);
	}

	return nullptr;
}
