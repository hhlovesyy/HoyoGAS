#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MyGameplayTagEventBusSubsystem.generated.h"

USTRUCT(BlueprintType)
struct HOYOGAS_API FMyGameplayTagEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EventBus")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EventBus")
	FInstancedStruct Payload;

	template <typename PayloadType>
	const PayloadType* GetPayload() const
	{
		const UScriptStruct* ExpectedStruct = PayloadType::StaticStruct();
		const UScriptStruct* ActualStruct = Payload.GetScriptStruct();
		if (!ExpectedStruct || !ActualStruct || !ActualStruct->IsChildOf(ExpectedStruct))
		{
			return nullptr;
		}

		return reinterpret_cast<const PayloadType*>(Payload.GetMemory());
	}

	template <typename PayloadType>
	bool HasPayload() const
	{
		return GetPayload<PayloadType>() != nullptr;
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FMyGameplayTagEventDelegate, const FMyGameplayTagEvent&);

UCLASS()
class HOYOGAS_API UMyGameplayTagEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Publish(FGameplayTag EventTag);
	void Publish(FGameplayTag EventTag, const FInstancedStruct& Payload);

	template <typename PayloadType>
	void PublishTyped(FGameplayTag EventTag, const PayloadType& Payload)
	{
		FInstancedStruct InstancedPayload;
		InstancedPayload.InitializeAs(PayloadType::StaticStruct(), reinterpret_cast<const uint8*>(&Payload));
		Publish(EventTag, InstancedPayload);
	}

	FDelegateHandle Subscribe(FGameplayTag EventTag, const FMyGameplayTagEventDelegate::FDelegate& Delegate);

	template <typename UserClass, typename PayloadType>
	FDelegateHandle SubscribeTyped(FGameplayTag EventTag, UserClass* Object, void (UserClass::*Function)(const FGameplayTag&, const PayloadType&))
	{
		TWeakObjectPtr<UserClass> WeakObject(Object);
		return Subscribe(EventTag, FMyGameplayTagEventDelegate::FDelegate::CreateLambda(
			[WeakObject, Function](const FMyGameplayTagEvent& Event)
			{
				UserClass* StrongObject = WeakObject.Get();
				if (!StrongObject)
				{
					return;
				}

				const PayloadType* TypedPayload = Event.GetPayload<PayloadType>();
				if (!TypedPayload)
				{
					UE_LOG(LogTemp, Warning, TEXT("EventBus payload type mismatch. Event=%s ExpectedPayload=%s ActualPayload=%s"),
						*Event.EventTag.ToString(),
						*PayloadType::StaticStruct()->GetName(),
						Event.Payload.GetScriptStruct() ? *Event.Payload.GetScriptStruct()->GetName() : TEXT("None"));
					return;
				}

				(StrongObject->*Function)(Event.EventTag, *TypedPayload);
			}));
	}

	void Unsubscribe(FGameplayTag EventTag, FDelegateHandle Handle);

private:
	TMap<FGameplayTag, FMyGameplayTagEventDelegate> DelegatesByTag;
};
