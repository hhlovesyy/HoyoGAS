#include "UI/SurvivorHUDScreen.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "UI/SurvivorHUDStore.h"
#include "UI/VM_SurvivorHUD.h"

USurvivorHUDScreen::USurvivorHUDScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Game;
}

UWidget* USurvivorHUDScreen::NativeGetDesiredFocusTarget() const
{
	return nullptr;
}

UObject* USurvivorHUDScreen::CreateViewModelInstance()
{
	return NewObject<UVM_SurvivorHUD>(this);
}

void USurvivorHUDScreen::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(ViewModel))
	{
		HUDViewModel->Initialize(StoreSubsystem ? StoreSubsystem->GetStore<USurvivorHUDStore>() : nullptr);
	}
}

void USurvivorHUDScreen::TeardownViewModel(UObject* ViewModel)
{
	if (UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(ViewModel))
	{
		HUDViewModel->Teardown();
	}
}

void USurvivorHUDScreen::HandlePostViewModelAttached()
{
	if (UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(GetViewModelObject()))
	{
		auto BindField = [this, HUDViewModel](UE::FieldNotification::FFieldId FieldId)
		{
			FieldChangedHandles.Add(HUDViewModel->AddFieldValueChangedDelegate(
				FieldId,
				INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(
					this,
					&USurvivorHUDScreen::HandleAnyHUDFieldChanged)));
		};

		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::HealthText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::HealthPercent);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::LevelText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::WeaponCountText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardSlotsText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::EquippedCardsText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardTagsText);
	}

	RefreshFromViewModel();
}

void USurvivorHUDScreen::HandlePreViewModelDetached(UObject* ViewModel)
{
	if (UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(ViewModel))
	{
		const UE::FieldNotification::FFieldId Fields[] =
		{
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::HealthText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::HealthPercent,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::LevelText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::WeaponCountText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardSlotsText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::EquippedCardsText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardTagsText
		};

		const int32 HandleCount = FMath::Min(FieldChangedHandles.Num(), static_cast<int32>(UE_ARRAY_COUNT(Fields)));
		for (int32 Index = 0; Index < HandleCount; ++Index)
		{
			if (FieldChangedHandles[Index].IsValid())
			{
				HUDViewModel->RemoveFieldValueChangedDelegate(Fields[Index], FieldChangedHandles[Index]);
			}
		}
	}

	FieldChangedHandles.Reset();
}

void USurvivorHUDScreen::RefreshFromViewModel()
{
	const UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(GetViewModelObject());
	if (!HUDViewModel)
	{
		return;
	}

	if (HealthText)
	{
		HealthText->SetText(HUDViewModel->GetHealthText());
	}

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HUDViewModel->GetHealthPercent());
	}

	if (LevelText)
	{
		LevelText->SetText(HUDViewModel->GetLevelText());
	}

	if (WeaponCountText)
	{
		WeaponCountText->SetText(HUDViewModel->GetWeaponCountText());
	}

	if (GoldText)
	{
		GoldText->SetText(HUDViewModel->GetGoldText());
	}

	if (ExperienceText)
	{
		ExperienceText->SetText(HUDViewModel->GetExperienceText());
	}

	if (CardSlotsText)
	{
		CardSlotsText->SetText(HUDViewModel->GetCardSlotsText());
	}

	if (EquippedCardsText)
	{
		EquippedCardsText->SetText(HUDViewModel->GetEquippedCardsText());
	}

	if (CardTagsText)
	{
		CardTagsText->SetText(HUDViewModel->GetCardTagsText());
	}
}

void USurvivorHUDScreen::HandleAnyHUDFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;
	(void)FieldId;
	RefreshFromViewModel();
}
