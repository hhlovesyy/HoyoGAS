#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIFrameworkTypes.h"
#include "MyPlayerUISubsystem.generated.h"

class UCommonActivatableWidgetStack;
class UHoyoObjectPoolSubsystem;
class UMyDialogBase;
class UMyScreenBase;
class UMyUIRegistrySubsystem;
class UMyUIRootLayout;
class UUserWidget;
class UWidget;
class UCommonActivatableWidget;

UCLASS(Config = Game, DefaultConfig)
class HOYOGAS_API UMyPlayerUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool EnsureRootLayout();

	UFUNCTION(BlueprintPure, Category = "UIFramework")
	UMyUIRootLayout* GetRootLayout() const;

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	UMyScreenBase* OpenScreen(FName ScreenTag, const FMyUIPayload& Payload);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool CloseScreen(UMyScreenBase* Screen);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool CloseTopScreen(EMyUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	UMyDialogBase* ShowConfirmDialog(FMyConfirmDialogConfig Config, FMyDialogResultDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	UMyScreenBase* ShowDialog(FName ScreenTag, const FMyUIPayload& Payload);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	UMyScreenBase* ShowToast(const FText& Message, FGameplayTag ToastTag);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool AddWidgetInstanceToOverlayLayer(UUserWidget* Widget, EMyUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	bool RemoveOverlayWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void SetRootLayoutClass(TSoftClassPtr<UMyUIRootLayout> InRootLayoutClass);

	UFUNCTION(BlueprintCallable, Category = "UIFramework")
	void SetConfirmDialogScreenTag(FName InScreenTag);

	UFUNCTION(BlueprintPure, Category = "UIFramework")
	UMyScreenBase* FindScreenByTag(FName ScreenTag) const;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework")
	TSoftClassPtr<UMyUIRootLayout> RootLayoutClass;

	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework")
	FName ConfirmDialogScreenTag = TEXT("ConfirmDialog");

	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework")
	FName DefaultToastScreenTag = NAME_None;

	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework|Toast", meta = (ClampMin = "1"))
	int32 MaxVisibleToastCount = 3;

	UPROPERTY(Config, EditDefaultsOnly, Category = "UIFramework|Toast", meta = (ClampMin = "0"))
	int32 MaxIdleToastPoolCount = 4;

	UPROPERTY(Transient)
	TObjectPtr<UMyUIRootLayout> RootLayout;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UMyScreenBase>> ScreenInstancesByTag;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMyScreenBase>> ActiveToastScreens;

	TMap<EMyUILayer, TArray<TObjectPtr<UMyScreenBase>>> LayerScreens;

	UMyUIRegistrySubsystem* GetRegistrySubsystem() const;
	UHoyoObjectPoolSubsystem* GetObjectPoolSubsystem() const;
	TSubclassOf<UMyUIRootLayout> ResolveRootLayoutClass() const;
	UCommonActivatableWidgetStack* GetCommonStackForLayer(EMyUILayer Layer) const;
	bool IsCommonStackLayer(EMyUILayer Layer) const;
	UMyScreenBase* CreateScreenInstance(TSubclassOf<UMyScreenBase> ScreenClass) const;
	void RestoreGameplayInputConfigIfNeeded() const;
	void RequestDeferredUIStateRefresh(float DelaySeconds) const;
	void DebugLogLayerState(const TCHAR* Context) const;
	void BindCommonStackDebugHooks();
	void HandleMenuStackDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget);
	void HandleModalStackDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget);
	void ApplyFocusToDisplayedWidget(UCommonActivatableWidget* DisplayedWidget, EMyUILayer Layer);
	void ApplyFocusToWidget(UWidget* FocusWidget, UCommonActivatableWidget* OwnerWidget, EMyUILayer Layer, const TCHAR* Phase);
	UMyScreenBase* FindScreenInstanceByTag(FName ScreenTag) const;
	UMyScreenBase* PushScreenToCommonLayer(EMyUILayer Layer, TSubclassOf<UMyScreenBase> ScreenClass, const FMyUIPayload& Payload, FName ScreenTag);
	bool PopTopScreenFromCommonLayer(EMyUILayer Layer);
	void RemoveScreenFromTrackedLayers(UMyScreenBase* Screen);
	bool IsManagedToastScreen(const UMyScreenBase* Screen) const;
	UMyScreenBase* ShowToastFromPool(const FMyUIPayload& Payload);
	bool CloseManagedToastScreen(UMyScreenBase* Screen);
	void ReflowToastStack();
};
