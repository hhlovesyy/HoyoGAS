#include "Subsystems/UIFlowRuleBase.h"

#include "Engine/LocalPlayer.h"
#include "Subsystems/GameplayDemoUIFlowSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"

void UUIFlowRuleBase::Initialize(UGameplayDemoUIFlowSubsystem* InOwningFlowSubsystem)
{
	OwningFlowSubsystem = InOwningFlowSubsystem;
}

void UUIFlowRuleBase::Deinitialize()
{
	OwningFlowSubsystem = nullptr;
}

void UUIFlowRuleBase::RefreshBindings()
{
}

UGameplayDemoUIFlowSubsystem* UUIFlowRuleBase::GetOwningFlowSubsystem() const
{
	return OwningFlowSubsystem;
}

ULocalPlayer* UUIFlowRuleBase::GetOwningLocalPlayer() const
{
	return OwningFlowSubsystem ? OwningFlowSubsystem->GetLocalPlayer() : nullptr;
}

UMyPlayerUISubsystem* UUIFlowRuleBase::GetPlayerUISubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer);
	}

	return nullptr;
}
