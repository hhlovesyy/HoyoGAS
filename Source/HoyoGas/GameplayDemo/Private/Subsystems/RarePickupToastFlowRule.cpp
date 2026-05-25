#include "Subsystems/RarePickupToastFlowRule.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Events/GameplayDemoEventPayloads.h"
#include "MyUITags.h"
#include "Subsystems/MyGameplayTagEventBusSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"

void URarePickupToastFlowRule::Deinitialize()
{
	UnbindEventBus();
	Super::Deinitialize();
}

void URarePickupToastFlowRule::RefreshBindings()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UMyGameplayTagEventBusSubsystem* NewEventBus = nullptr;

	if (LocalPlayer)
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			NewEventBus = GameInstance->GetSubsystem<UMyGameplayTagEventBusSubsystem>();
		}
	}

	if (EventBus.Get() == NewEventBus)
	{
		return;
	}

	UnbindEventBus();
	EventBus = NewEventBus;

	if (EventBus.IsValid())
	{
		RarePickupEventHandle = EventBus->SubscribeTyped<URarePickupToastFlowRule, FGameplayDemoRarePickupEventPayload>(
			MyUITags::Gameplay_Event_RarePickup,
			this,
			&URarePickupToastFlowRule::HandleRarePickupEvent);
	}
}

void URarePickupToastFlowRule::HandleRarePickupEvent(const FGameplayTag& EventTag, const FGameplayDemoRarePickupEventPayload& Payload)
{
	if (EventTag != MyUITags::Gameplay_Event_RarePickup)
	{
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = GetPlayerUISubsystem())
	{
		PlayerUISubsystem->ShowToast(
			Payload.ToastMessage.IsEmpty() ? FText::Format(INVTEXT("Rare pickup: {0}"), Payload.DisplayName) : Payload.ToastMessage,
			MyUITags::UI_Request_Show_RarePickupToast);
	}
}

void URarePickupToastFlowRule::UnbindEventBus()
{
	if (EventBus.IsValid() && RarePickupEventHandle.IsValid())
	{
		EventBus->Unsubscribe(MyUITags::Gameplay_Event_RarePickup, RarePickupEventHandle);
		RarePickupEventHandle.Reset();
	}

	EventBus.Reset();
}
