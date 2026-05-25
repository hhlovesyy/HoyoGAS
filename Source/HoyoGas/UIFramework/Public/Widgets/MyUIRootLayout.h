#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIFrameworkTypes.h"
#include "MyUIRootLayout.generated.h"

class UCommonActivatableWidgetStack;
class UMyScreenBase;
class UOverlay;
class UWidget;

UCLASS(Blueprintable)
class HOYOGAS_API UMyUIRootLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool AddScreenToLayer(UMyScreenBase* Screen, EMyUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool RemoveScreen(UMyScreenBase* Screen);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool AddWidgetToOverlayLayer(UWidget* Widget, EMyUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool RemoveWidgetFromLayout(UWidget* Widget);

	UFUNCTION(BlueprintPure, Category = "UIFramework")
	UOverlay* GetLayerRoot(EMyUILayer Layer) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UIFramework")
	UCommonActivatableWidgetStack* GetMenuStack() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UIFramework")
	UCommonActivatableWidgetStack* GetModalStack() const;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> GameLayerRoot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> ModalStack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ToastLayerRoot;

private:
	bool AddWidgetToOverlayLayerInternal(UWidget* Widget, EMyUILayer Layer, bool bFillWidget);
	void EnsureLayerRoots();
	UOverlay* CreateLayerRoot(const FName InName);
	UCommonActivatableWidgetStack* CreateCommonStack(const FName InName);
};
