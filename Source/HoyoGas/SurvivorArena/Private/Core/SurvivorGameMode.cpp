#include "Core/SurvivorGameMode.h"

#include "Cards/SurvivorCardDefinition.h"
#include "Cards/SurvivorCardLoadoutComponent.h"
#include "Core/SurvivorArenaLog.h"
#include "Core/SurvivorArenaSettings.h"
#include "Core/SurvivorGameState.h"
#include "Core/SurvivorRunLauncherSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GAS/SurvivorAbilitySet.h"
#include "Inventory/SurvivorRunInventoryComponent.h"
#include "Player/SurvivorCharacter.h"
#include "Player/SurvivorPlayerController.h"
#include "Player/SurvivorPlayerState.h"
#include "Weapons/SurvivorWeaponDefinition.h"
#include "Weapons/SurvivorWeaponManagerComponent.h"

/*
*在多人联机游戏中，GameMode 资产和实例只存在于服务器端。任何客户端的电脑内存里，GetGameMode() 返回的永远是 nullptr。
这意味着：
	所有核心的规则决定（如：游戏何时开始、何时结束、胜负判定、刷怪控制）都必须写在 GameMode 里。
	客户端绝对无法通过开挂篡改 GameMode 的逻辑，因为他们根本拿不到这个对象。
 */

ASurvivorGameMode::ASurvivorGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ASurvivorGameState::StaticClass();
	PlayerControllerClass = ASurvivorPlayerController::StaticClass();
	PlayerStateClass = ASurvivorPlayerState::StaticClass();
	DefaultPawnClass = ASurvivorCharacter::StaticClass();
}

void ASurvivorGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorGameMode BeginPlay: %s"), *GetNameSafe(this));

	//在《土豆兄弟》这类游戏中，玩家通常在主菜单关卡选择角色、武器和地图，然后点击“开始游戏”跳转到战斗关卡。
	//痛点：关卡一切换，旧地图的所有 Actor 全被销毁，选好的角色 ID 怎么带过去？
	//解法：这段代码利用了生命周期贯穿全局的 GameInstanceSubsystem。主菜单把配置存在子系统里，新关卡的 GameMode 在 BeginPlay 时通过 ConsumePendingStartConfig 把配置“吃”出来。这是极其标准且解耦的商业游戏数据流传方式。
	FSurvivorRunStartConfig StartConfig;
	bool bHasPendingConfig = false;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USurvivorRunLauncherSubsystem* LauncherSubsystem = GameInstance->GetSubsystem<USurvivorRunLauncherSubsystem>())
		{
			bHasPendingConfig = LauncherSubsystem->ConsumePendingStartConfig(StartConfig);
		}
	}

	if (!bHasPendingConfig)
	{
		const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
		StartConfig.CharacterId = Settings ? Settings->DefaultCharacterId : NAME_None;
		StartConfig.LevelId = Settings ? Settings->DefaultLevelId : NAME_None;
	}

	StartSurvivorRun(StartConfig);
}

void ASurvivorGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	GrantStartingLoadoutToPlayer(NewPlayer);
}

void ASurvivorGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>();
	if (!SurvivorGameState)
	{
		return;
	}
	
	//为什么不让 GameState 自己在 Tick 里给自己算时间？
	//因为 GameMode 才是时间的掌控者。如果暂停游戏、或者进入了局内商店阶段，GameMode 可以通过改变 CurrentRunState 随时让计时器停下来。GameMode 算好正确的时间后，再通过 SetElapsedTime 塞给 GameState，触发属性同步给全场玩家。
	if (CurrentRunState == ESurvivorRunState::InRun || CurrentRunState == ESurvivorRunState::Preparing)
	{
		SurvivorGameState->SetElapsedTime(GetWorld() ? GetWorld()->GetTimeSeconds() - RunStartTimeSeconds : 0.0f);
	}
}

void ASurvivorGameMode::StartSurvivorRun(const FSurvivorRunStartConfig& Config)
{
	CurrentRunConfig = Config;
	RunStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LoadoutGrantedPlayers.Reset();

	UE_LOG(
		LogSurvivorArena,
		Log,
		TEXT("Starting Survivor run. CharacterId=%s LevelId=%s RandomSeed=%d FixedSeed=%s"),
		*CurrentRunConfig.CharacterId.ToString(),
		*CurrentRunConfig.LevelId.ToString(),
		CurrentRunConfig.RandomSeed,
		CurrentRunConfig.bUseFixedSeed ? TEXT("true") : TEXT("false"));

	if (ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>())
	{
		SurvivorGameState->ResetRunMetrics();
	}

	SetRunState(ESurvivorRunState::Preparing);
	SetRunState(ESurvivorRunState::InRun);
	GrantStartingLoadoutToExistingPlayers();
}

void ASurvivorGameMode::EndSurvivorRun(bool bVictory)
{
	SetRunState(bVictory ? ESurvivorRunState::Victory : ESurvivorRunState::Defeat);
	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor run ended. Victory=%s"), bVictory ? TEXT("true") : TEXT("false"));
}

void ASurvivorGameMode::SetRunState(ESurvivorRunState NewRunState)
{
	if (CurrentRunState == NewRunState)
	{
		return;
	}

	const ESurvivorRunState PreviousRunState = CurrentRunState;
	CurrentRunState = NewRunState;

	if (ASurvivorGameState* SurvivorGameState = GetGameState<ASurvivorGameState>())
	{
		SurvivorGameState->SetRunState(NewRunState);
	}

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorGameMode state transition: %d -> %d"), static_cast<int32>(PreviousRunState), static_cast<int32>(CurrentRunState));
}

const FSurvivorCharacterDefinitionRow* ASurvivorGameMode::ResolveCharacterDefinitionRow(FName CharacterId) const
{
	const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
	if (!Settings)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("ResolveCharacterDefinitionRow failed because SurvivorArenaSettings is null."));
		return nullptr;
	}

	UDataTable* CharacterDefinitionTable = Settings->CharacterDefinitionTable.LoadSynchronous();
	if (!CharacterDefinitionTable)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("ResolveCharacterDefinitionRow failed because CharacterDefinitionTable is not configured."));
		return nullptr;
	}

	if (CharacterId.IsNone())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("ResolveCharacterDefinitionRow failed because CharacterId is None."));
		return nullptr;
	}

	const FSurvivorCharacterDefinitionRow* CharacterDefinition = CharacterDefinitionTable->FindRow<FSurvivorCharacterDefinitionRow>(
		CharacterId,
		TEXT("ASurvivorGameMode::ResolveCharacterDefinitionRow"));
	if (!CharacterDefinition)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("ResolveCharacterDefinitionRow failed because CharacterId=%s was not found in CharacterDefinitionTable."),
			*CharacterId.ToString());
		return nullptr;
	}

	return CharacterDefinition;
}

void ASurvivorGameMode::GrantStartingLoadoutToExistingPlayers()
{
	//看起来这个代码是支持多人组队的，由服务器的GameMode给所有人再开局赋予GameAbility和Weapon
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		GrantStartingLoadoutToPlayer(Iterator->Get());
	}
}

bool ASurvivorGameMode::GrantStartingLoadoutToPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return false;
	}

	const TWeakObjectPtr<APlayerController> PlayerControllerKey(PlayerController);
	if (LoadoutGrantedPlayers.Contains(PlayerControllerKey))
	{
		return true;
	}

	const FSurvivorCharacterDefinitionRow* CharacterDefinition = ResolveCharacterDefinitionRow(CurrentRunConfig.CharacterId);
	if (!CharacterDefinition)
	{
		return false;
	}

	ASurvivorPlayerState* SurvivorPlayerState = PlayerController->GetPlayerState<ASurvivorPlayerState>();
	ASurvivorCharacter* SurvivorCharacter = Cast<ASurvivorCharacter>(PlayerController->GetPawn());
	if (!SurvivorPlayerState || !SurvivorCharacter)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed because SurvivorPlayerState or SurvivorCharacter is missing. Controller=%s Pawn=%s PlayerState=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(PlayerController->GetPawn()),
			*GetNameSafe(PlayerController->PlayerState));
		return false;
	}

	UAbilitySystemComponent* PlayerASC = SurvivorPlayerState->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed because Player ASC is null. Controller=%s PlayerState=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(SurvivorPlayerState));
		return false;
	}

	USurvivorCardLoadoutComponent* LoadoutComponent = SurvivorPlayerState->GetLoadoutComponent();
	USurvivorRunInventoryComponent* RunInventoryComponent = SurvivorPlayerState->GetRunInventoryComponent();
	if (!LoadoutComponent || !RunInventoryComponent)
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed because LoadoutComponent or RunInventoryComponent is null. PlayerState=%s Loadout=%s RunInventory=%s"),
			*GetNameSafe(SurvivorPlayerState),
			*GetNameSafe(LoadoutComponent),
			*GetNameSafe(RunInventoryComponent));
		return false;
	}

	const USurvivorArenaSettings* Settings = GetDefault<USurvivorArenaSettings>();
	TArray<USurvivorCardDefinition*> StartingCardDefinitions;
	if (Settings)
	{
		StartingCardDefinitions.Reserve(Settings->DefaultStartingCards.Num());
		for (const TSoftObjectPtr<USurvivorCardDefinition>& CardSoftPtr : Settings->DefaultStartingCards)
		{
			USurvivorCardDefinition* CardDefinition = CardSoftPtr.LoadSynchronous();
			if (!CardDefinition)
			{
				UE_LOG(LogSurvivorArena, Error, TEXT("DefaultStartingCards contains an unloaded or null card asset."));
				return false;
			}

			FString ValidationError;
			if (!CardDefinition->ValidateDefinition(&ValidationError))
			{
				UE_LOG(LogSurvivorArena, Error, TEXT("DefaultStartingCard is invalid. Card=%s Error=%s"),
					*GetNameSafe(CardDefinition),
					*ValidationError);
				return false;
			}

			StartingCardDefinitions.Add(CardDefinition);
		}
	}

	TArray<USurvivorWeaponDefinition*> StartingWeaponDefinitions;
	StartingWeaponDefinitions.Reserve(CharacterDefinition->StartingWeapons.Num());

	for (USurvivorAbilitySet* AbilitySet : CharacterDefinition->StartingAbilitySets)
	{
		if (!AbilitySet)
		{
			UE_LOG(LogSurvivorArena, Error, TEXT("CharacterDefinition contains a null StartingAbilitySet. CharacterId=%s"), *CurrentRunConfig.CharacterId.ToString());
			return false;
		}
	}

	for (const TSoftObjectPtr<USurvivorWeaponDefinition>& WeaponSoftPtr : CharacterDefinition->StartingWeapons)
	{
		USurvivorWeaponDefinition* WeaponDefinition = WeaponSoftPtr.LoadSynchronous();
		if (!WeaponDefinition)
		{
			UE_LOG(LogSurvivorArena, Error, TEXT("CharacterDefinition contains an unloaded or null StartingWeapon. CharacterId=%s"),
				*CurrentRunConfig.CharacterId.ToString());
			return false;
		}

		FString ValidationError;
		if (!WeaponDefinition->ValidateRuntimeConfiguration(&ValidationError))
		{
			UE_LOG(LogSurvivorArena, Error, TEXT("StartingWeapon is invalid. CharacterId=%s Weapon=%s Error=%s"),
				*CurrentRunConfig.CharacterId.ToString(),
				*GetNameSafe(WeaponDefinition),
				*ValidationError);
			return false;
		}

		StartingWeaponDefinitions.Add(WeaponDefinition);
	}

	for (USurvivorAbilitySet* AbilitySet : CharacterDefinition->StartingAbilitySets)
	{
		if (!RunInventoryComponent->GrantStartingAbilitySet(AbilitySet, SurvivorCharacter))
		{
			UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed to grant StartingAbilitySet. Controller=%s CharacterId=%s AbilitySet=%s"),
				*GetNameSafe(PlayerController),
				*CurrentRunConfig.CharacterId.ToString(),
				*GetNameSafe(AbilitySet));
			return false;
		}
	}

	if (!SurvivorCharacter->GetWeaponManagerComponent())
	{
		UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed because WeaponManagerComponent is null. Character=%s"),
			*GetNameSafe(SurvivorCharacter));
		return false;
	}

	for (USurvivorWeaponDefinition* WeaponDefinition : StartingWeaponDefinitions)
	{
		if (!RunInventoryComponent->GrantStartingWeapon(WeaponDefinition))
		{
			UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed to grant StartingWeapon. Controller=%s CharacterId=%s Weapon=%s"),
				*GetNameSafe(PlayerController),
				*CurrentRunConfig.CharacterId.ToString(),
				*GetNameSafe(WeaponDefinition));
			return false;
		}
	}

	if (StartingCardDefinitions.Num() == 0)
	{
		UE_LOG(LogSurvivorArena, Log, TEXT("No DefaultStartingCards configured. PlayerState=%s"), *GetNameSafe(SurvivorPlayerState));
	}
	else
	{
		for (USurvivorCardDefinition* CardDefinition : StartingCardDefinitions)
		{
			if (!LoadoutComponent->EquipCard(CardDefinition))
			{
				UE_LOG(LogSurvivorArena, Error, TEXT("GrantStartingLoadoutToPlayer failed to equip DefaultStartingCard. Controller=%s Card=%s"),
					*GetNameSafe(PlayerController),
					*GetNameSafe(CardDefinition));
				return false;
			}
		}
	}

	LoadoutComponent->NotifyRunStarted();

	LoadoutGrantedPlayers.Add(PlayerControllerKey);

	UE_LOG(LogSurvivorArena, Log, TEXT("Granted starting loadout. Controller=%s CharacterId=%s WeaponCount=%d AbilitySetCount=%d StartingCardCount=%d"),
		*GetNameSafe(PlayerController),
		*CurrentRunConfig.CharacterId.ToString(),
		CharacterDefinition->StartingWeapons.Num(),
		CharacterDefinition->StartingAbilitySets.Num(),
		StartingCardDefinitions.Num());

	return true;
}
