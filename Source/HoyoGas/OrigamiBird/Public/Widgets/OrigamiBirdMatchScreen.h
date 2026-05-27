#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyMVVMScreenBase.h"
#include "OrigamiBirdMatchScreen.generated.h"

namespace UE::FieldNotification
{
	struct FFieldId;
}

class UButton;
class UListView;
class UMyUIStoreSubsystem;
class UOrigamiBirdBoardWidget;
class UOrigamiBirdMatchSubsystem;
class UVM_OrigamiBirdMatchScreen;

// Origami bird match main screen.
// Screen owns page lifecycle and user routing. Board presentation lives in UOrigamiBirdBoardWidget.
UCLASS()
class HOYOGAS_API UOrigamiBirdMatchScreen : public UMyMVVMScreenBase
{
	GENERATED_BODY()

public:
	UOrigamiBirdMatchScreen(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;
	virtual void HandlePostViewModelAttached() override;
	virtual void HandlePreViewModelDetached(UObject* ViewModel) override;
	virtual void HandlePostViewModelDetached() override;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UOrigamiBirdBoardWidget> BoardWidget;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> PropListView;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleTileClicked(FIntPoint BoardPosition);

	void RefreshBoardFromViewModel();
	void RefreshPropListFromViewModel();
	void HandleViewModelFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId);
	bool OpenDebugScreen();
	UOrigamiBirdMatchSubsystem* GetMatchSubsystem() const;
	UVM_OrigamiBirdMatchScreen* GetMatchViewModel() const;
};
