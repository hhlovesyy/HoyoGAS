#include "Widgets/CharacterPanelScreen.h"

#include "CharacterPanelTags.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "INotifyFieldValueChanged.h"
#include "Stores/CharacterPanelUIStore.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "ViewModels/VM_CharacterRelicSetBonusEntry.h"
#include "ViewModels/VM_CharacterRelicSlotEntry.h"
#include "ViewModels/VM_CharacterPanelScreen.h"
#include "ViewModels/VM_CharacterStoryEntry.h"

UCharacterPanelScreen::UCharacterPanelScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Menu;
}

void UCharacterPanelScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (DetailsTabButton)
	{
		DetailsTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleDetailsTabClicked);
		DetailsTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleDetailsTabClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleCloseClicked);
	}

	if (LightConeTabButton)
	{
		LightConeTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleLightConeTabClicked);
		LightConeTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleLightConeTabClicked);
	}

	if (TracesTabButton)
	{
		TracesTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleTracesTabClicked);
		TracesTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleTracesTabClicked);
	}

	if (RelicsTabButton)
	{
		RelicsTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicsTabClicked);
		RelicsTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicsTabClicked);
	}

	if (EidolonsTabButton)
	{
		EidolonsTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleEidolonsTabClicked);
		EidolonsTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleEidolonsTabClicked);
	}

	if (InfoTabButton)
	{
		InfoTabButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleInfoTabClicked);
		InfoTabButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleInfoTabClicked);
	}

	if (StoryListView)
	{
		StoryListView->OnItemSelectionChanged().RemoveAll(this);
		StoryListView->OnItemSelectionChanged().AddUObject(this, &UCharacterPanelScreen::HandleStorySelectionChanged);
	}

	if (GrantRelicDevelopmentLoadoutButton)
	{
		GrantRelicDevelopmentLoadoutButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleGrantRelicDevelopmentLoadoutClicked);
		GrantRelicDevelopmentLoadoutButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleGrantRelicDevelopmentLoadoutClicked);
	}

	if (RelicHeadSlotButton)
	{
		RelicHeadSlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicHeadSlotClicked);
		RelicHeadSlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicHeadSlotClicked);
	}
	if (RelicHandsSlotButton)
	{
		RelicHandsSlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicHandsSlotClicked);
		RelicHandsSlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicHandsSlotClicked);
	}
	if (RelicBodySlotButton)
	{
		RelicBodySlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicBodySlotClicked);
		RelicBodySlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicBodySlotClicked);
	}
	if (RelicFeetSlotButton)
	{
		RelicFeetSlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicFeetSlotClicked);
		RelicFeetSlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicFeetSlotClicked);
	}
	if (RelicPlanarSphereSlotButton)
	{
		RelicPlanarSphereSlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicPlanarSphereSlotClicked);
		RelicPlanarSphereSlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicPlanarSphereSlotClicked);
	}
	if (RelicLinkRopeSlotButton)
	{
		RelicLinkRopeSlotButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicLinkRopeSlotClicked);
		RelicLinkRopeSlotButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicLinkRopeSlotClicked);
	}
	if (RelicDetailToggleButton)
	{
		RelicDetailToggleButton->OnClicked.RemoveDynamic(this, &UCharacterPanelScreen::HandleRelicDetailToggleClicked);
		RelicDetailToggleButton->OnClicked.AddDynamic(this, &UCharacterPanelScreen::HandleRelicDetailToggleClicked);
	}
}

UWidget* UCharacterPanelScreen::NativeGetDesiredFocusTarget() const
{
	if (DetailsTabButton)
	{
		return DetailsTabButton;
	}

	if (LightConeTabButton)
	{
		return LightConeTabButton;
	}

	return Super::NativeGetDesiredFocusTarget();
}

void UCharacterPanelScreen::SetActiveTab(EHoyoCharacterPanelTab Tab)
{
	if (ActiveTab == Tab)
	{
		ApplyActiveTabToWidgets();
		return;
	}

	ActiveTab = Tab;
	ApplyActiveTabToWidgets();
}

EHoyoCharacterPanelTab UCharacterPanelScreen::GetActiveTab() const
{
	return ActiveTab;
}

UObject* UCharacterPanelScreen::CreateViewModelInstance()
{
	return NewObject<UVM_CharacterPanelScreen>(this);
}

void UCharacterPanelScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = Cast<UVM_CharacterPanelScreen>(ViewModel))
	{
		CharacterPanelViewModel->Initialize(ResolveCharacterPanelStore(StoreSubsystem));
		CharacterPanelViewModel->SetSelectedTabIndex(GetTabIndex(ActiveTab));
		CharacterPanelViewModel->SetSelectedTabLabel(GetTabLabel(ActiveTab));
	}
}

void UCharacterPanelScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = Cast<UVM_CharacterPanelScreen>(ViewModel))
	{
		CharacterPanelViewModel->Teardown();
	}
}

void UCharacterPanelScreen::HandlePostViewModelAttached()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		const INotifyFieldValueChanged::FFieldValueChangedDelegate Delegate =
			INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &UCharacterPanelScreen::HandleViewModelFieldChanged);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::StoryEntries, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedStoryIndex, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSlotEntries, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSetBonusEntries, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicCharacterStatsText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicNameText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicMetaText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicMainAffixText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicDetailText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSetSummaryText, Delegate);
		CharacterPanelViewModel->AddFieldValueChangedDelegate(UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicDetailButtonText, Delegate);
	}

	ApplyActiveTabToWidgets();
	RefreshStoryListFromViewModel();
	RefreshStorySelectionFromViewModel();
	RefreshRelicListsFromViewModel();
	RefreshRelicOrbitFromViewModel();
	RefreshRelicDetailFromViewModel();
}

void UCharacterPanelScreen::HandlePreViewModelDetached(UObject* ViewModel)
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = Cast<UVM_CharacterPanelScreen>(ViewModel))
	{
		CharacterPanelViewModel->RemoveAllFieldValueChangedDelegates(this);
	}
}

void UCharacterPanelScreen::HandlePostViewModelDetached()
{
	RefreshStoryListFromViewModel();
	RefreshStorySelectionFromViewModel();
	RefreshRelicListsFromViewModel();
	RefreshRelicOrbitFromViewModel();
	RefreshRelicDetailFromViewModel();
}

void UCharacterPanelScreen::HandleCloseClicked()
{
	RequestClose();
}

void UCharacterPanelScreen::HandleDetailsTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::Details);
}

void UCharacterPanelScreen::HandleLightConeTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::LightCone);
}

void UCharacterPanelScreen::HandleTracesTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::Traces);
}

void UCharacterPanelScreen::HandleRelicsTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::Relics);
}

void UCharacterPanelScreen::HandleEidolonsTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::Eidolons);
}

void UCharacterPanelScreen::HandleInfoTabClicked()
{
	SetActiveTab(EHoyoCharacterPanelTab::Info);
}

void UCharacterPanelScreen::HandleGrantRelicDevelopmentLoadoutClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->GrantRelicDevelopmentLoadout();
		RefreshRelicListsFromViewModel();
		RefreshRelicOrbitFromViewModel();
		RefreshRelicDetailFromViewModel();
	}
}

void UCharacterPanelScreen::HandleRelicHeadSlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::Head);
	}
}

void UCharacterPanelScreen::HandleRelicHandsSlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::Hands);
	}
}

void UCharacterPanelScreen::HandleRelicBodySlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::Body);
	}
}

void UCharacterPanelScreen::HandleRelicFeetSlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::Feet);
	}
}

void UCharacterPanelScreen::HandleRelicPlanarSphereSlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::PlanarSphere);
	}
}

void UCharacterPanelScreen::HandleRelicLinkRopeSlotClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SelectRelicSlot(EHoyoRelicSlot::LinkRope);
	}
}

void UCharacterPanelScreen::HandleRelicDetailToggleClicked()
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->ToggleSelectedRelicDetail();
	}
}

UVM_CharacterPanelScreen* UCharacterPanelScreen::GetCharacterPanelViewModel() const
{
	return Cast<UVM_CharacterPanelScreen>(GetViewModelObject());
}

void UCharacterPanelScreen::RefreshStoryListFromViewModel()
{
	if (!StoryListView)
	{
		return;
	}

	StoryListView->ClearListItems();

	UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel();
	if (!CharacterPanelViewModel)
	{
		return;
	}

	for (UVM_CharacterStoryEntry* Entry : CharacterPanelViewModel->GetStoryEntries())
	{
		if (Entry)
		{
			StoryListView->AddItem(Entry);
		}
	}
}

void UCharacterPanelScreen::RefreshStorySelectionFromViewModel()
{
	if (!StoryListView)
	{
		return;
	}

	UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel();
	if (!CharacterPanelViewModel)
	{
		bSynchronizingStorySelectionFromViewModel = true;
		StoryListView->ClearSelection();
		bSynchronizingStorySelectionFromViewModel = false;
		return;
	}

	const int32 SelectedIndex = CharacterPanelViewModel->GetSelectedStoryIndex();
	const int32 CurrentSelectedIndex = StoryListView->GetIndexForItem(StoryListView->GetSelectedItem());
	if (CurrentSelectedIndex == SelectedIndex)
	{
		return;
	}

	bSynchronizingStorySelectionFromViewModel = true;
	if (SelectedIndex != INDEX_NONE && SelectedIndex < StoryListView->GetNumItems())
	{
		StoryListView->SetSelectedIndex(SelectedIndex);
	}
	else
	{
		StoryListView->ClearSelection();
	}
	bSynchronizingStorySelectionFromViewModel = false;
}

void UCharacterPanelScreen::RefreshRelicListsFromViewModel()
{
	if (RelicSlotListView)
	{
		RelicSlotListView->ClearListItems();
		if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
		{
			for (UVM_CharacterRelicSlotEntry* Entry : CharacterPanelViewModel->GetRelicSlotEntries())
			{
				if (Entry)
				{
					RelicSlotListView->AddItem(Entry);
				}
			}
		}
	}

	if (RelicSetBonusListView)
	{
		RelicSetBonusListView->ClearListItems();
		if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
		{
			for (UVM_CharacterRelicSetBonusEntry* Entry : CharacterPanelViewModel->GetRelicSetBonusEntries())
			{
				if (Entry)
				{
					RelicSetBonusListView->AddItem(Entry);
				}
			}
		}
	}
}

void UCharacterPanelScreen::RefreshRelicOrbitFromViewModel()
{
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::Head), RelicHeadIconImage, RelicHeadLevelText, RelicHeadNameText);
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::Hands), RelicHandsIconImage, RelicHandsLevelText, RelicHandsNameText);
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::Body), RelicBodyIconImage, RelicBodyLevelText, RelicBodyNameText);
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::Feet), RelicFeetIconImage, RelicFeetLevelText, RelicFeetNameText);
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::PlanarSphere), RelicPlanarSphereIconImage, RelicPlanarSphereLevelText, RelicPlanarSphereNameText);
	ApplyRelicSlotEntryToWidgets(FindRelicSlotEntry(EHoyoRelicSlot::LinkRope), RelicLinkRopeIconImage, RelicLinkRopeLevelText, RelicLinkRopeNameText);
}

void UCharacterPanelScreen::RefreshRelicDetailFromViewModel()
{
	UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel();
	if (!CharacterPanelViewModel)
	{
		return;
	}

	if (RelicCharacterStatsText)
	{
		RelicCharacterStatsText->SetText(CharacterPanelViewModel->GetRelicCharacterStatsText());
	}
	if (SelectedRelicNameText)
	{
		SelectedRelicNameText->SetText(CharacterPanelViewModel->GetSelectedRelicNameText());
	}
	if (SelectedRelicMetaText)
	{
		SelectedRelicMetaText->SetText(CharacterPanelViewModel->GetSelectedRelicMetaText());
	}
	if (SelectedRelicMainAffixText)
	{
		SelectedRelicMainAffixText->SetText(CharacterPanelViewModel->GetSelectedRelicMainAffixText());
	}
	if (SelectedRelicDetailText)
	{
		SelectedRelicDetailText->SetText(CharacterPanelViewModel->GetSelectedRelicDetailText());
	}
	if (RelicSetSummaryText)
	{
		RelicSetSummaryText->SetText(CharacterPanelViewModel->GetRelicSetSummaryText());
	}
	if (RelicDetailToggleButtonText)
	{
		RelicDetailToggleButtonText->SetText(CharacterPanelViewModel->GetRelicDetailButtonText());
	}
}

void UCharacterPanelScreen::ApplyRelicSlotEntryToWidgets(UVM_CharacterRelicSlotEntry* Entry, UImage* IconImage, UTextBlock* LevelText, UTextBlock* NameText)
{
	const bool bEquipped = Entry && Entry->GetIsEquipped();
	if (IconImage)
	{
		if (bEquipped && Entry->GetIconTexture())
		{
			IconImage->SetBrushFromTexture(Entry->GetIconTexture(), false);
			IconImage->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			IconImage->SetBrushFromTexture(nullptr);
			IconImage->SetColorAndOpacity(FLinearColor(0.20f, 0.22f, 0.24f, 1.0f));
		}
	}

	if (LevelText)
	{
		LevelText->SetText(bEquipped ? Entry->GetLevelText() : FText::GetEmpty());
	}
	if (NameText)
	{
		NameText->SetText(FText::GetEmpty());
	}
}

UVM_CharacterRelicSlotEntry* UCharacterPanelScreen::FindRelicSlotEntry(EHoyoRelicSlot RelicSlot) const
{
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		for (UVM_CharacterRelicSlotEntry* Entry : CharacterPanelViewModel->GetRelicSlotEntries())
		{
			if (Entry && Entry->GetSlot() == RelicSlot)
			{
				return Entry;
			}
		}
	}
	return nullptr;
}

void UCharacterPanelScreen::HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;

	if (FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::StoryEntries)
	{
		RefreshStoryListFromViewModel();
		RefreshStorySelectionFromViewModel();
	}
	else if (FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedStoryIndex)
	{
		RefreshStorySelectionFromViewModel();
	}
	else if (FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSlotEntries
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSetBonusEntries)
	{
		RefreshRelicListsFromViewModel();
		RefreshRelicOrbitFromViewModel();
		RefreshRelicDetailFromViewModel();
	}
	else if (FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicCharacterStatsText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicNameText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicMetaText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicMainAffixText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::SelectedRelicDetailText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicSetSummaryText
		|| FieldId == UVM_CharacterPanelScreen::FFieldNotificationClassDescriptor::RelicDetailButtonText)
	{
		RefreshRelicDetailFromViewModel();
	}
}

void UCharacterPanelScreen::HandleStorySelectionChanged(UObject* Item)
{
	if (bSynchronizingStorySelectionFromViewModel)
	{
		return;
	}

	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		const int32 SelectedIndex = StoryListView ? StoryListView->GetIndexForItem(Item) : INDEX_NONE;
		if (CharacterPanelViewModel->GetSelectedStoryIndex() != SelectedIndex)
		{
			CharacterPanelViewModel->SetSelectedStoryIndex(SelectedIndex);
		}
	}
}

UCharacterPanelUIStore* UCharacterPanelScreen::ResolveCharacterPanelStore(UMyUIStoreSubsystem* StoreSubsystem)
{
	if (StoreSubsystem)
	{
		if (UCharacterPanelUIStore* RegisteredStore = StoreSubsystem->GetStore<UCharacterPanelUIStore>())
		{
			return RegisteredStore;
		}
	}

	if (!FallbackCharacterPanelStore)
	{
		FallbackCharacterPanelStore = NewObject<UCharacterPanelUIStore>(this);
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		FallbackCharacterPanelStore->InitializeStore(LocalPlayer, HoyoCharacterPanelTags::UI_Store_CharacterPanel);
	}

	APlayerController* PlayerController = GetOwningPlayer();
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	FallbackCharacterPanelStore->BindToPlayerContext(Pawn, PlayerState);
	return FallbackCharacterPanelStore;
}

void UCharacterPanelScreen::ApplyActiveTabToWidgets()
{
	const int32 ActiveTabIndex = GetTabIndex(ActiveTab);
	if (PageSwitcher && PageSwitcher->GetChildrenCount() > ActiveTabIndex)
	{
		PageSwitcher->SetActiveWidgetIndex(ActiveTabIndex);
	}

	const FText ActiveTabLabel = GetTabLabel(ActiveTab);
	if (UVM_CharacterPanelScreen* CharacterPanelViewModel = GetCharacterPanelViewModel())
	{
		CharacterPanelViewModel->SetSelectedTabIndex(ActiveTabIndex);
		CharacterPanelViewModel->SetSelectedTabLabel(ActiveTabLabel);
	}
}

int32 UCharacterPanelScreen::GetTabIndex(EHoyoCharacterPanelTab Tab) const
{
	return static_cast<int32>(Tab);
}

FText UCharacterPanelScreen::GetTabLabel(EHoyoCharacterPanelTab Tab) const
{
	switch (Tab)
	{
	case EHoyoCharacterPanelTab::Details:
		return NSLOCTEXT("CharacterPanel", "DetailsTab", "Details");
	case EHoyoCharacterPanelTab::LightCone:
		return NSLOCTEXT("CharacterPanel", "LightConeTab", "Light Cone");
	case EHoyoCharacterPanelTab::Traces:
		return NSLOCTEXT("CharacterPanel", "TracesTab", "Traces");
	case EHoyoCharacterPanelTab::Relics:
		return NSLOCTEXT("CharacterPanel", "RelicsTab", "Relics");
	case EHoyoCharacterPanelTab::Eidolons:
		return NSLOCTEXT("CharacterPanel", "EidolonsTab", "Eidolons");
	case EHoyoCharacterPanelTab::Info:
		return NSLOCTEXT("CharacterPanel", "InfoTab", "Info");
	default:
		return FText::GetEmpty();
	}
}
