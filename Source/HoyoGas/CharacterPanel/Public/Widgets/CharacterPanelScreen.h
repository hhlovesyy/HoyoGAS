#pragma once

#include "CoreMinimal.h"
#include "CharacterPanelTypes.h"
#include "Data/HoyoRelicTypes.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "CharacterPanelScreen.generated.h"

namespace UE::FieldNotification
{
	struct FFieldId;
}

class UButton;
class UCharacterPanelUIStore;
class UImage;
class UListView;
class UMyUIStoreSubsystem;
class UTextBlock;
class UVM_CharacterPanelScreen;
class UVM_CharacterRelicSlotEntry;
class UWidgetSwitcher;

UCLASS()
class HOYOGAS_API UCharacterPanelScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	UCharacterPanelScreen(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UFUNCTION(BlueprintCallable, Category = "CharacterPanel")
	void SetActiveTab(EHoyoCharacterPanelTab Tab);

	UFUNCTION(BlueprintPure, Category = "CharacterPanel")
	EHoyoCharacterPanelTab GetActiveTab() const;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;
	virtual void HandlePostViewModelAttached() override;
	virtual void HandlePreViewModelDetached(UObject* ViewModel) override;
	virtual void HandlePostViewModelDetached() override;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetSwitcher> PageSwitcher;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> DetailsTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> LightConeTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> TracesTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicsTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> EidolonsTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> InfoTabButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> StoryListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> RelicSlotListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> RelicSetBonusListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> GrantRelicDevelopmentLoadoutButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicHeadSlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicHandsSlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicBodySlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicFeetSlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicPlanarSphereSlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicLinkRopeSlotButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicHeadIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicHandsIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicBodyIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicFeetIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicPlanarSphereIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> RelicLinkRopeIconImage;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicHeadLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicHandsLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicBodyLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicFeetLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicPlanarSphereLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicLinkRopeLevelText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicHeadNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicHandsNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicBodyNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicFeetNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicPlanarSphereNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicLinkRopeNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicCharacterStatsText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> SelectedRelicNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> SelectedRelicMetaText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> SelectedRelicMainAffixText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> SelectedRelicDetailText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicSetSummaryText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RelicDetailToggleButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> RelicDetailToggleButtonText;

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleDetailsTabClicked();

	UFUNCTION()
	void HandleLightConeTabClicked();

	UFUNCTION()
	void HandleTracesTabClicked();

	UFUNCTION()
	void HandleRelicsTabClicked();

	UFUNCTION()
	void HandleEidolonsTabClicked();

	UFUNCTION()
	void HandleInfoTabClicked();

	UFUNCTION()
	void HandleGrantRelicDevelopmentLoadoutClicked();

	UFUNCTION()
	void HandleRelicHeadSlotClicked();

	UFUNCTION()
	void HandleRelicHandsSlotClicked();

	UFUNCTION()
	void HandleRelicBodySlotClicked();

	UFUNCTION()
	void HandleRelicFeetSlotClicked();

	UFUNCTION()
	void HandleRelicPlanarSphereSlotClicked();

	UFUNCTION()
	void HandleRelicLinkRopeSlotClicked();

	UFUNCTION()
	void HandleRelicDetailToggleClicked();

	void RefreshStoryListFromViewModel();
	void RefreshStorySelectionFromViewModel();
	void RefreshRelicListsFromViewModel();
	void RefreshRelicOrbitFromViewModel();
	void RefreshRelicDetailFromViewModel();
	void ApplyRelicSlotEntryToWidgets(UVM_CharacterRelicSlotEntry* Entry, UImage* IconImage, UTextBlock* LevelText, UTextBlock* NameText);
	UVM_CharacterRelicSlotEntry* FindRelicSlotEntry(EHoyoRelicSlot RelicSlot) const;
	void HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);
	void HandleStorySelectionChanged(UObject* Item);

	UVM_CharacterPanelScreen* GetCharacterPanelViewModel() const;
	UCharacterPanelUIStore* ResolveCharacterPanelStore(UMyUIStoreSubsystem* StoreSubsystem);
	void ApplyActiveTabToWidgets();
	int32 GetTabIndex(EHoyoCharacterPanelTab Tab) const;
	FText GetTabLabel(EHoyoCharacterPanelTab Tab) const;

	UPROPERTY(Transient)
	EHoyoCharacterPanelTab ActiveTab = EHoyoCharacterPanelTab::Details;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterPanelUIStore> FallbackCharacterPanelStore;

	UPROPERTY(Transient)
	bool bSynchronizingStorySelectionFromViewModel = false;
};
