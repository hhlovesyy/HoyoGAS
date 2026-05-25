#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "GameplayDemoInventoryScreen.generated.h"

namespace UE::FieldNotification
{
	struct FFieldId;
}

class UButton;
class UGameplayDemoListView;
class UTextBlock;

UCLASS()
class HOYOGAS_API UGameplayDemoInventoryScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	UGameplayDemoInventoryScreen(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UFUNCTION()
	void HandleCloseClicked();

	void RefreshListItemsFromViewModel();
	void RefreshSelectionFromViewModel();
	void HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);
	void HandleListSelectionChanged(UObject* Item);
	class UVM_InventoryScreen* GetInventoryViewModel() const;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, class UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;
	virtual void HandlePostViewModelAttached() override;
	virtual void HandlePreViewModelDetached(UObject* ViewModel) override;
	virtual void HandlePostViewModelDetached() override;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayDemoListView> ItemListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TotalItemsText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TotalValueText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailCountText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> DetailValueText;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	bool bSynchronizingSelectionFromViewModel = false;
};
