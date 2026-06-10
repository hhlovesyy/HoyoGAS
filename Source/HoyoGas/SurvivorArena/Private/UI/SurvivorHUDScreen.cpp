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
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::SurvivorLevelValue);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::WeaponCountText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldDelta);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceDelta);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldChangeEventId);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceChangeEventId);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceLevelUpClearedAmount);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceLevelUpEventId);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardSlotsText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::EquippedCardsText);
		BindField(UVM_SurvivorHUD::FFieldNotificationClassDescriptor::CardTagsText);
	}

	RefreshFromViewModel();
	RefreshDeltaAnimationState();
	bCanPlayDeltaAnimations = true;
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
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::SurvivorLevelValue,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::WeaponCountText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceText,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldDelta,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceDelta,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::GoldChangeEventId,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceChangeEventId,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceLevelUpClearedAmount,
			UVM_SurvivorHUD::FFieldNotificationClassDescriptor::ExperienceLevelUpEventId,
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
	bCanPlayDeltaAnimations = false;
	LastProcessedGoldChangeEventId = 0;
	LastProcessedExperienceChangeEventId = 0;
	LastProcessedExperienceLevelUpEventId = 0;
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

void USurvivorHUDScreen::RefreshDeltaAnimationState()
{
	const UVM_SurvivorHUD* HUDViewModel = Cast<UVM_SurvivorHUD>(GetViewModelObject());
	if (!HUDViewModel)
	{
		return;
	}

	const int32 NewGoldChangeEventId = HUDViewModel->GetGoldChangeEventId();
	if (NewGoldChangeEventId != LastProcessedGoldChangeEventId)
	{
		LastProcessedGoldChangeEventId = NewGoldChangeEventId;
		if (bCanPlayDeltaAnimations)
		{
			const int32 DeltaGold = HUDViewModel->GetGoldDelta();
			if (DeltaGold > 0)
			{
				BP_OnGoldIncreased(DeltaGold);
				BP_OnGoldChanged(DeltaGold);
			}
			else if (DeltaGold < 0)
			{
				BP_OnGoldDecreased(DeltaGold);
				BP_OnGoldChanged(DeltaGold);
			}
		}
	}

	const int32 NewExperienceChangeEventId = HUDViewModel->GetExperienceChangeEventId();
	if (NewExperienceChangeEventId != LastProcessedExperienceChangeEventId)
	{
		LastProcessedExperienceChangeEventId = NewExperienceChangeEventId;
		if (bCanPlayDeltaAnimations && HUDViewModel->GetExperienceDelta() > 0.0f)
		{
			BP_OnExperienceIncreased(HUDViewModel->GetExperienceDelta());
			BP_OnExperienceChanged(HUDViewModel->GetExperienceDelta());
		}
	}

	const int32 NewExperienceLevelUpEventId = HUDViewModel->GetExperienceLevelUpEventId();
	if (NewExperienceLevelUpEventId != LastProcessedExperienceLevelUpEventId)
	{
		LastProcessedExperienceLevelUpEventId = NewExperienceLevelUpEventId;
		if (bCanPlayDeltaAnimations)
		{
			BP_OnExperienceClearedForLevelUp(
				HUDViewModel->GetExperienceLevelUpClearedAmount(),
				HUDViewModel->GetSurvivorLevelValue());
		}
	}
}

void USurvivorHUDScreen::HandleAnyHUDFieldChanged(UObject* Object, UE::FieldNotification::FFieldId FieldId)
{
	(void)Object;
	(void)FieldId;
	RefreshFromViewModel();
	RefreshDeltaAnimationState();
}
