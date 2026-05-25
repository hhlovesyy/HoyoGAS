#include "Widgets/GameplayDemoInventoryScreen.h"

#include "Components/Button.h"
#include "INotifyFieldValueChanged.h"
#include "Stores/InventoryUIStore.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "VM_InventoryItemEntry.h"
#include "VM_InventoryScreen.h"
#include "Widgets/GameplayDemoListView.h"

UGameplayDemoInventoryScreen::UGameplayDemoInventoryScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Menu;
}

void UGameplayDemoInventoryScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ItemListView)
	{
		ItemListView->OnItemSelectionChanged().RemoveAll(this);
		ItemListView->OnItemSelectionChanged().AddUObject(this, &UGameplayDemoInventoryScreen::HandleListSelectionChanged);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UGameplayDemoInventoryScreen::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UGameplayDemoInventoryScreen::HandleCloseClicked);
	}
}

UObject* UGameplayDemoInventoryScreen::CreateViewModelInstance()
{
	return NewObject<UVM_InventoryScreen>(this);
}

void UGameplayDemoInventoryScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_InventoryScreen* InventoryViewModel = Cast<UVM_InventoryScreen>(ViewModel))
	{
		InventoryViewModel->Initialize(StoreSubsystem ? StoreSubsystem->GetStore<UInventoryUIStore>() : nullptr);
	}
}

UWidget* UGameplayDemoInventoryScreen::NativeGetDesiredFocusTarget() const
{
	return ItemListView ? Cast<UWidget>(ItemListView) : Cast<UWidget>(CloseButton);
}

void UGameplayDemoInventoryScreen::HandleCloseClicked()
{
	RequestClose();
}

UVM_InventoryScreen* UGameplayDemoInventoryScreen::GetInventoryViewModel() const
{
	return Cast<UVM_InventoryScreen>(GetViewModelObject());
}

void UGameplayDemoInventoryScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_InventoryScreen* InventoryViewModel = Cast<UVM_InventoryScreen>(ViewModel))
	{
		InventoryViewModel->Teardown();
	}
}

void UGameplayDemoInventoryScreen::HandlePostViewModelAttached()
{
	if (UVM_InventoryScreen* InventoryViewModel = GetInventoryViewModel())
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UGameplayDemoInventoryScreen::HandleViewModelFieldChanged);
		InventoryViewModel->AddFieldValueChangedDelegate(UVM_InventoryScreen::FFieldNotificationClassDescriptor::ItemEntries, Delegate);
		InventoryViewModel->AddFieldValueChangedDelegate(UVM_InventoryScreen::FFieldNotificationClassDescriptor::SelectedIndex, Delegate);
	}

	RefreshListItemsFromViewModel();
	RefreshSelectionFromViewModel();
}

void UGameplayDemoInventoryScreen::HandlePreViewModelDetached(UObject* ViewModel)
{
	if (UVM_InventoryScreen* InventoryViewModel = Cast<UVM_InventoryScreen>(ViewModel))
	{
		InventoryViewModel->RemoveAllFieldValueChangedDelegates(this);
	}
}

void UGameplayDemoInventoryScreen::HandlePostViewModelDetached()
{
	RefreshListItemsFromViewModel();
	RefreshSelectionFromViewModel();
}

void UGameplayDemoInventoryScreen::RefreshListItemsFromViewModel()
{
	if (!ItemListView)
	{
		return;
	}

	if (!GetInventoryViewModel())
	{
		ItemListView->ClearListItems();
		return;
	}

	TArray<UObject*> ListItems;
	for (UVM_InventoryItemEntry* Entry : GetInventoryViewModel()->GetItemEntries())
	{
		ListItems.Add(Entry);
	}

	ItemListView->SetListItemsPublic(ListItems);
}

void UGameplayDemoInventoryScreen::RefreshSelectionFromViewModel()
{
	if (!ItemListView)
	{
		return;
	}

	UVM_InventoryScreen* InventoryViewModel = GetInventoryViewModel();
	if (!InventoryViewModel)
	{
		bSynchronizingSelectionFromViewModel = true;
		ItemListView->ClearSelection();
		bSynchronizingSelectionFromViewModel = false;
		return;
	}

	const int32 SelectedIndex = InventoryViewModel->GetSelectedIndex();
	const int32 CurrentSelectedIndex = ItemListView->GetIndexForItem(ItemListView->GetSelectedItem());

	if (CurrentSelectedIndex == SelectedIndex)
	{
		return;
	}

	bSynchronizingSelectionFromViewModel = true;
	if (SelectedIndex != INDEX_NONE && SelectedIndex < ItemListView->GetNumItems())
	{
		ItemListView->SetSelectedIndex(SelectedIndex);
	}
	else
	{
		ItemListView->ClearSelection();
	}
	bSynchronizingSelectionFromViewModel = false;
}

void UGameplayDemoInventoryScreen::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;

	if (FieldId == UVM_InventoryScreen::FFieldNotificationClassDescriptor::ItemEntries)
	{
		RefreshListItemsFromViewModel();
		RefreshSelectionFromViewModel();
	}
	else if (FieldId == UVM_InventoryScreen::FFieldNotificationClassDescriptor::SelectedIndex)
	{
		RefreshSelectionFromViewModel();
	}
}

void UGameplayDemoInventoryScreen::HandleListSelectionChanged(UObject* Item)
{
	if (bSynchronizingSelectionFromViewModel)
	{
		return;
	}

	if (UVM_InventoryScreen* InventoryViewModel = GetInventoryViewModel())
	{
		const int32 SelectedIndex = ItemListView ? ItemListView->GetIndexForItem(Item) : INDEX_NONE;
		if (InventoryViewModel->GetSelectedIndex() != SelectedIndex)
		{
			InventoryViewModel->SetSelectedIndex(SelectedIndex);
		}
	}
}
