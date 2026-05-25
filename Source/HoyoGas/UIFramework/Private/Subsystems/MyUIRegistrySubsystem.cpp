#include "Subsystems/MyUIRegistrySubsystem.h"

#include "Data/MyUIRegistryDataAsset.h"

void UMyUIRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadRegistry();
}

void UMyUIRegistrySubsystem::ReloadRegistry()
{
	ScreenConfigs.Empty();

	if (RegistryAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIRegistrySubsystem::ReloadRegistry skipped because RegistryAsset is not configured."));
		return;
	}

	UMyUIRegistryDataAsset* LoadedRegistryAsset = RegistryAsset.LoadSynchronous();
	if (!LoadedRegistryAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIRegistrySubsystem::ReloadRegistry failed because RegistryAsset could not be loaded."));
		return;
	}

	for (const FMyUIScreenConfig& ScreenConfig : LoadedRegistryAsset->Screens)
	{
		AddScreenConfigInternal(ScreenConfig, true);
	}
}

void UMyUIRegistrySubsystem::RegisterScreenConfig(const FMyUIScreenConfig& ScreenConfig)
{
	AddScreenConfigInternal(ScreenConfig, true);
}

bool UMyUIRegistrySubsystem::FindScreenConfig(FName ScreenTag, FMyUIScreenConfig& OutScreenConfig) const
{
	if (const FMyUIScreenConfig* FoundConfig = ScreenConfigs.Find(ScreenTag))
	{
		OutScreenConfig = *FoundConfig;
		return true;
	}

	return false;
}

void UMyUIRegistrySubsystem::AddScreenConfigInternal(const FMyUIScreenConfig& ScreenConfig, bool bWarnOnOverride)
{
	if (ScreenConfig.ScreenTag.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIRegistrySubsystem::AddScreenConfigInternal failed because ScreenTag is None."));
		return;
	}

	if (bWarnOnOverride && ScreenConfigs.Contains(ScreenConfig.ScreenTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMyUIRegistrySubsystem found duplicate ScreenTag '%s'. The later config will override the previous one."), *ScreenConfig.ScreenTag.ToString());
	}

	ScreenConfigs.Add(ScreenConfig.ScreenTag, ScreenConfig);
}
