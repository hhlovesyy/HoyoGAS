#include "Subsystems/MyGameplayTagEventBusSubsystem.h"

void UMyGameplayTagEventBusSubsystem::Publish(FGameplayTag EventTag)
{
	Publish(EventTag, FInstancedStruct());
}

void UMyGameplayTagEventBusSubsystem::Publish(FGameplayTag EventTag, const FInstancedStruct& Payload)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	if (FMyGameplayTagEventDelegate* Delegate = DelegatesByTag.Find(EventTag))
	{
		FMyGameplayTagEvent Event;
		Event.EventTag = EventTag;
		Event.Payload = Payload;
		Delegate->Broadcast(Event);
	}
}

FDelegateHandle UMyGameplayTagEventBusSubsystem::Subscribe(FGameplayTag EventTag, const FMyGameplayTagEventDelegate::FDelegate& Delegate)
{
	if (!EventTag.IsValid())
	{
		return FDelegateHandle();
	}

	return DelegatesByTag.FindOrAdd(EventTag).Add(Delegate);
}

void UMyGameplayTagEventBusSubsystem::Unsubscribe(FGameplayTag EventTag, FDelegateHandle Handle)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	if (FMyGameplayTagEventDelegate* Delegate = DelegatesByTag.Find(EventTag))
	{
		Delegate->Remove(Handle);
	}
}
