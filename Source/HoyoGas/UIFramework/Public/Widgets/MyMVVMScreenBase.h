#pragma once

#include "CoreMinimal.h"
#include "Widgets/MyScreenBase.h"
#include "MyMVVMScreenBase.generated.h"

class UMyUIStoreSubsystem;

UCLASS(Abstract, Blueprintable)
class HOYOGAS_API UMyMVVMScreenBase : public UMyScreenBase
{
	GENERATED_BODY()

public:
	// CommonUI activation is centralized here so MVVM screens follow one lifecycle shape.
	// The current workflow assumes one screen owns one primary view model.
	// This lightweight abstraction is intended for standard pages only; multi-view-model
	// composition is intentionally out of scope for this pass and future generated pages
	// should keep following the single primary view model convention by default.
	virtual void NativeOnActivated() final override;
	virtual void NativeOnDeactivated() final override;

protected:
	// To create a new MVVM screen:
	// 1. Derive from UMyMVVMScreenBase.
	// 2. Override CreateViewModelInstance and InitializeViewModel.
	// 3. Override TeardownViewModel if the view model subscribes to stores or other sources.
	// 4. Override the optional bound/unbound hooks for widget-side glue such as list refreshes.
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
