#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyDialogBase.h"
#include "MyMVVMDialogBase.generated.h"

class UMyUIStoreSubsystem;

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UMyMVVMDialogBase : public UMyDialogBase
{
	GENERATED_BODY()

public:
	// Dialog MVVM lifecycle mirrors the screen variant so future modal widgets stay consistent.
	// The current workflow assumes one dialog owns one primary view model.
	// This lightweight abstraction is intended for standard pages only; multi-view-model
	// composition is intentionally out of scope for this pass and future generated pages
	// should keep following the single primary view model convention by default.
	virtual void NativeOnActivated() final override;
	virtual void NativeOnDeactivated() final override;

protected:
	// To create a new MVVM dialog:
	// 1. Derive from UMyMVVMDialogBase.
	// 2. Override CreateViewModelInstance and InitializeViewModel.
	// 3. Override TeardownViewModel if the view model owns external bindings.
	// 4. Override the optional bound/unbound hooks for widget-side behavior.
	// This base intentionally manages a single primary view model object.
	UObject* GetViewModelObject() const;
	UMyUIStoreSubsystem* GetUIStoreSubsystem() const;

	virtual UObject* CreateViewModelInstance();
	virtual void InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem);
	virtual void TeardownViewModel(UObject* ViewModel);
	virtual void HandlePostViewModelAttached();
	virtual void HandlePreViewModelDetached(UObject* ViewModel);
	virtual void HandlePostViewModelDetached();

private:
	void AttachViewModelObject(UObject* InViewModel);
	void DetachViewModelObject();

	UPROPERTY(Transient)
	TObjectPtr<UObject> ViewModelObject;
};
