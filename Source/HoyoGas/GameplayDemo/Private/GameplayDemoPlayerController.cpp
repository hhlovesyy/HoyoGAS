#include "GameplayDemoPlayerController.h"

#include "Battle/HoyoBattleTargetMarkerActor.h"
#include "ElementReaction/ElementDummyTarget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameplayDemoUIConstants.h"
#include "HoyoGasCharacter.h"
#include "Interaction/HoyoInteractableInterface.h"
#include "Character/HoyoEnemyInterface.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/HoyoBattleFlowSubsystem.h"
#include "Subsystems/GameplayDemoUIFlowSubsystem.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "Widgets/MyScreenBase.h"

void AGameplayDemoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		CachedBattleFlowSubsystem = LocalPlayer->GetSubsystem<UHoyoBattleFlowSubsystem>();
		if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = CachedBattleFlowSubsystem.Get())
		{
			BattleFlowSubsystem->OnFlowStateChanged().AddUObject(this, &AGameplayDemoPlayerController::HandleBattleFlowStateChanged);
		}
	}

	AddInputMappingContexts();
	SyncBattleInputMode();
	InitializeGameplayDemoUI();
}

void AGameplayDemoPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindGameplayDebugInput();
	RemoveInputMappingContexts();

	if (CachedBattleFlowSubsystem.IsValid())
	{
		CachedBattleFlowSubsystem->OnFlowStateChanged().RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AGameplayDemoPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocalController())
	{
		return;
	}

	UpdateBattleTargeting();
}

void AGameplayDemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	BindEnhancedInputActions();
	BindGameplayDebugInput();
}

void AGameplayDemoPlayerController::OnPossess(APawn* InPawn)
{
	/*
	* 它通常发生在：
	• GameMode 给玩家生成 Pawn 并让 PlayerController possess。
	• 玩家死亡后重生，Controller possess 新 Pawn。
	• 手动切换 Pawn。
	• 服务器调用 Possess()。
	在网络游戏里，OnPossess 主要是服务端权威侧调用。客户端通常靠 Pawn/PlayerState 复制、AcknowledgePossession、OnRep_Pawn 等路径知道自己拥有了 Pawn。 所以如果以后做多人，不能只靠 OnPossess 打开本地 UI。
	 */
	Super::OnPossess(InPawn);
	HandlePossessedPawnChanged();
}

void AGameplayDemoPlayerController::AcknowledgePossession(APawn* P)
{
	//多人客户端本地确认 possession 时刷新 UI 上下文
	Super::AcknowledgePossession(P);
	HandlePossessedPawnChanged();
}

void AGameplayDemoPlayerController::HandlePossessedPawnChanged()
{
	DebugTarget.Reset();
	SetHoveredEnemy(nullptr);
	SetSelectedEnemy(nullptr);
	RefreshGameplayDemoUIContext();
	BindGameplayDebugInput();
}

UEnhancedInputLocalPlayerSubsystem* AGameplayDemoPlayerController::GetEnhancedInputSubsystem() const
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer) : nullptr;
}

void AGameplayDemoPlayerController::AddInputMappingContexts()
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
	if (!EnhancedInputSubsystem)
	{
		return;
	}

	if (GameplayDemoMappingContext)
	{
		EnhancedInputSubsystem->AddMappingContext(GameplayDemoMappingContext, GameplayDemoMappingPriority);
	}

	RefreshBattleTargetingInputMappingContext();
}

void AGameplayDemoPlayerController::RemoveInputMappingContexts()
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
	if (!EnhancedInputSubsystem)
	{
		return;
	}

	if (BattleTargetingMappingContext)
	{
		EnhancedInputSubsystem->RemoveMappingContext(BattleTargetingMappingContext);
	}

	if (GameplayDemoMappingContext)
	{
		EnhancedInputSubsystem->RemoveMappingContext(GameplayDemoMappingContext);
	}
}

void AGameplayDemoPlayerController::RefreshBattleTargetingInputMappingContext()
{
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
	if (!EnhancedInputSubsystem || !BattleTargetingMappingContext)
	{
		return;
	}

	if (IsBattleTargetingActive())
	{
		EnhancedInputSubsystem->AddMappingContext(BattleTargetingMappingContext, BattleTargetingMappingPriority);
	}
	else
	{
		EnhancedInputSubsystem->RemoveMappingContext(BattleTargetingMappingContext);
	}
}

void AGameplayDemoPlayerController::BindEnhancedInputActions()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AGameplayDemoPlayerController requires EnhancedInputComponent. Check DefaultInput.ini DefaultInputComponentClass."));
		return;
	}

	if (ToggleInventoryAction)
	{
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::ToggleInventoryScreen);
	}

	if (InteractAction)
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::TryInteract);
	}

	if (OpenPauseMenuAction)
	{
		EnhancedInputComponent->BindAction(OpenPauseMenuAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::OpenPauseMenu);
	}

	if (ConfirmBattleTargetAction)
	{
		EnhancedInputComponent->BindAction(ConfirmBattleTargetAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::ConfirmBattleTargetSelection);
	}
	
	if (OpenCharacterDetailsAction)
	{
		EnhancedInputComponent->BindAction(OpenCharacterDetailsAction,ETriggerEvent::Started,  this, &AGameplayDemoPlayerController::OpenCharacterDetailPanel);
	}
}

void AGameplayDemoPlayerController::BindGameplayDebugInput()
{
	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	UnbindGameplayDebugInput();
	AHoyoGasCharacter* HoyoGasCharacter = Cast<AHoyoGasCharacter>(GetPawn());
	if (!HoyoGasCharacter)
	{
		return;
	}

	if (UInputAction* TestPyroAction = HoyoGasCharacter->GetTestPyroAction())
	{
		TestPyroBindingHandle = EnhancedInputComponent->BindAction(TestPyroAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::TestPyro).GetHandle();
	}

	if (UInputAction* TestHydroAction = HoyoGasCharacter->GetTestHydroAction())
	{
		TestHydroBindingHandle = EnhancedInputComponent->BindAction(TestHydroAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::TestHydro).GetHandle();
	}

	if (UInputAction* TestCryoAction = HoyoGasCharacter->GetTestCryoAction())
	{
		TestCryoBindingHandle = EnhancedInputComponent->BindAction(TestCryoAction, ETriggerEvent::Started, this, &AGameplayDemoPlayerController::TestCryo).GetHandle();
	}
}

void AGameplayDemoPlayerController::UnbindGameplayDebugInput()
{
	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (TestPyroBindingHandle != 0)
	{
		EnhancedInputComponent->RemoveBindingByHandle(TestPyroBindingHandle);
		TestPyroBindingHandle = 0;
	}

	if (TestHydroBindingHandle != 0)
	{
		EnhancedInputComponent->RemoveBindingByHandle(TestHydroBindingHandle);
		TestHydroBindingHandle = 0;
	}

	if (TestCryoBindingHandle != 0)
	{
		EnhancedInputComponent->RemoveBindingByHandle(TestCryoBindingHandle);
		TestCryoBindingHandle = 0;
	}
}

void AGameplayDemoPlayerController::FindDebugTarget()
{
	DebugTarget.Reset();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AElementDummyTarget::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		DebugTarget = Cast<AElementDummyTarget>(FoundActors[0]);
	}
}

void AGameplayDemoPlayerController::TryInteract()
{
	//这里只做通用逻辑判断，比如GAS中玩家具有不能交互的标签：
	/*
	* // 只检查通用的状态标签，不关心具体是战斗、看剧情还是死亡
	if (HasMatchingGameplayTag(TAG_State_DisableInteraction))
	{
		return;
	}
	 */
	AActor* InteractableActor = FindBestInteractableActor();
	if (!InteractableActor)
	{
		return;
	}

	IHoyoInteractableInterface::Execute_Interact(InteractableActor, this);
}

AActor* AGameplayDemoPlayerController::FindBestInteractableActor() const
{
	const APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return nullptr;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SearchSphere = FCollisionShape::MakeSphere(InteractionSearchRadius);
	//SCENE_QUERY_STAT这部分：这是一个给“物理查询”贴上“专属铭牌”的性能分析（Profiling）宏。当你在代码里写下 SCENE_QUERY_STAT(FindBestInteractableActor) 时，你实际上是在告诉虚幻引擎：
	//“嘿，老兄，接下来要做的这个画圈找物体的操作（Overlap），请帮我记在账上，名字就叫 FindBestInteractableActor！”
	//它会将这个 C++ 的标识符转换成一个 FName（虚幻内置的高效字符串类型），并传递给底层的物理查询参数 FCollisionQueryParams
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FindBestInteractableActor), false, ControlledPawn); //碰撞查询的参数设置。这里的 false 代表不需要复杂碰撞（只用简单的碰撞盒即可，性能更好），传入 ControlledPawn 是为了把自己（玩家）排除在外，防止玩家自己跟自己交互。
	
	//当你在编辑器（或者开发版游戏）中运行时，你可以按下波浪号 ~ 打开控制台，输入以下命令：
	//stat collision 或者 stat scenequery
	if (!World->OverlapMultiByObjectType(
		OverlapResults,
		ControlledPawn->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_WorldDynamic),
		SearchSphere,
		QueryParams))
	{
		return nullptr;
	} //这是 UE 里最常用的 API 之一！ 它的意思是：“在当前世界中，以玩家位置为中心，用刚才画的球体去检测，把所有物理碰撞通道属于 ECC_WorldDynamic（动态物体）的 Actor 全部抓出来，塞进 OverlapResults 这个数组里。”

	AActor* BestActor = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max(); //初始设置为无限大，直接线性遍历来找到最近的

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* CandidateActor = OverlapResult.GetActor();
		if (!CandidateActor || !CandidateActor->GetClass()->ImplementsInterface(UHoyoInteractableInterface::StaticClass()))
		{
			continue;
		}

		if (!IHoyoInteractableInterface::Execute_CanInteract(CandidateActor, const_cast<AGameplayDemoPlayerController*>(this)))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(ControlledPawn->GetActorLocation(), CandidateActor->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestActor = CandidateActor;
		}
	}

	return BestActor;
}

void AGameplayDemoPlayerController::ConfirmBattleTargetSelection()
{
	if (!IsBattleTargetingActive())
	{
		return;
	}

	UpdateHoveredEnemy();
	AActor* TargetEnemy = HoveredEnemyActor.Get();
	if (!TargetEnemy)
	{
		if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = CachedBattleFlowSubsystem.Get())
		{
			TargetEnemy = BattleFlowSubsystem->GetHoveredEnemy();
		}
	}

	if (TargetEnemy)
	{
		SetSelectedEnemy(TargetEnemy);
	}
}

void AGameplayDemoPlayerController::HandleBattleFlowStateChanged(EHoyoBattleFlowState PreviousState, EHoyoBattleFlowState NewState)
{
	SyncBattleInputMode();
	RefreshBattleTargetingInputMappingContext();

	if (NewState != EHoyoBattleFlowState::TargetSelection)
	{
		SetHoveredEnemy(nullptr);
		SetSelectedEnemy(nullptr);
	}
}

void AGameplayDemoPlayerController::SyncBattleInputMode()
{
	if (!IsLocalController())
	{
		return;
	}

	const bool bEnableBattleCursor = IsBattleTargetingActive();
	bShowMouseCursor = bEnableBattleCursor;
	bEnableClickEvents = bEnableBattleCursor;
	bEnableMouseOverEvents = bEnableBattleCursor;

	if (bEnableBattleCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		EnsureBattleMarkerActors();
		RefreshBattleMarkerActors();
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	RefreshBattleMarkerActors();
}

void AGameplayDemoPlayerController::EnsureBattleMarkerActors()
{
	if (!GetWorld())
	{
		return;
	}

	if (!HoveredMarkerClass)
	{
		HoveredMarkerClass = AHoyoBattleTargetMarkerActor::StaticClass();
	}

	if (!SelectedMarkerClass)
	{
		SelectedMarkerClass = AHoyoBattleTargetMarkerActor::StaticClass();
	}

	if (!HoveredTargetMarkerActor.IsValid() && HoveredMarkerClass)
	{
		HoveredTargetMarkerActor = GetWorld()->SpawnActor<AHoyoBattleTargetMarkerActor>(HoveredMarkerClass, FTransform::Identity);
		if (AHoyoBattleTargetMarkerActor* HoveredMarker = HoveredTargetMarkerActor.Get())
		{
			HoveredMarker->ConfigureMarker(INVTEXT("[AIM]"), FColor(128, 220, 255), 26.0f, 24.0f);
		}
	}

	if (!SelectedTargetMarkerActor.IsValid() && SelectedMarkerClass)
	{
		SelectedTargetMarkerActor = GetWorld()->SpawnActor<AHoyoBattleTargetMarkerActor>(SelectedMarkerClass, FTransform::Identity);
		if (AHoyoBattleTargetMarkerActor* SelectedMarker = SelectedTargetMarkerActor.Get())
		{
			SelectedMarker->ConfigureMarker(INVTEXT("[TARGET]"), FColor(255, 64, 128), 34.0f, 38.0f);
		}
	}
}

void AGameplayDemoPlayerController::UpdateBattleTargeting()
{
	if (!IsBattleTargetingActive())
	{
		SetHoveredEnemy(nullptr);
		RefreshBattleMarkerActors();
		return;
	}

	UpdateHoveredEnemy();
	RefreshBattleMarkerActors();
}

void AGameplayDemoPlayerController::UpdateHoveredEnemy()
{
	if (!IsBattleTargetingActive())
	{
		SetHoveredEnemy(nullptr);
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		SetHoveredEnemy(nullptr);
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UHoyoEnemyInterface::StaticClass()))
	{
		SetHoveredEnemy(nullptr);
		return;
	}

	if (!IHoyoEnemyInterface::Execute_CanBeTargeted(HitActor))
	{
		SetHoveredEnemy(nullptr);
		return;
	}

	if (GetPawn())
	{
		const FVector ViewLocation = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : GetFocalLocation();
		const float DistanceToTarget = FVector::Dist(ViewLocation, IHoyoEnemyInterface::Execute_GetTargetLocation(HitActor));
		if (DistanceToTarget > EnemyHoverTraceDistance)
		{
			SetHoveredEnemy(nullptr);
			return;
		}
	}

	SetHoveredEnemy(HitActor);
}

void AGameplayDemoPlayerController::SetHoveredEnemy(AActor* NewHoveredActor)
{
	AActor* CurrentHoveredActor = HoveredEnemyActor.Get();
	if (CurrentHoveredActor == NewHoveredActor)
	{
		return;
	}

	HoveredEnemyActor = NewHoveredActor;
	if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = CachedBattleFlowSubsystem.Get())
	{
		BattleFlowSubsystem->SetHoveredEnemy(NewHoveredActor);
	}

	RefreshEnemyHighlightState(CurrentHoveredActor);
	RefreshEnemyHighlightState(NewHoveredActor);
}

void AGameplayDemoPlayerController::SetSelectedEnemy(AActor* NewSelectedActor)
{
	AActor* CurrentSelectedActor = SelectedEnemyActor.Get();
	if (CurrentSelectedActor == NewSelectedActor)
	{
		return;
	}

	SelectedEnemyActor = NewSelectedActor;
	if (UHoyoBattleFlowSubsystem* BattleFlowSubsystem = CachedBattleFlowSubsystem.Get())
	{
		BattleFlowSubsystem->SetSelectedEnemy(NewSelectedActor);
	}

	RefreshEnemyHighlightState(CurrentSelectedActor);
	RefreshEnemyHighlightState(NewSelectedActor);
}

void AGameplayDemoPlayerController::RefreshEnemyHighlightState(AActor* EnemyActor) const
{
	if (!EnemyActor || !EnemyActor->GetClass()->ImplementsInterface(UHoyoEnemyInterface::StaticClass()))
	{
		return;
	}

	const bool bShouldHighlight = EnemyActor == HoveredEnemyActor.Get() || EnemyActor == SelectedEnemyActor.Get();
	IHoyoEnemyInterface::Execute_SetEnemyHighlighted(EnemyActor, bShouldHighlight);
}

bool AGameplayDemoPlayerController::IsBattleTargetingActive() const
{
	const UHoyoBattleFlowSubsystem* BattleFlowSubsystem = CachedBattleFlowSubsystem.Get();
	return BattleFlowSubsystem && BattleFlowSubsystem->GetFlowState() == EHoyoBattleFlowState::TargetSelection;
}

void AGameplayDemoPlayerController::RefreshBattleMarkerActors()
{
	AHoyoBattleTargetMarkerActor* HoveredMarker = HoveredTargetMarkerActor.Get();
	AHoyoBattleTargetMarkerActor* SelectedMarker = SelectedTargetMarkerActor.Get();

	if (!IsBattleTargetingActive())
	{
		if (HoveredMarker)
		{
			HoveredMarker->ClearTrackedTarget();
		}

		if (SelectedMarker)
		{
			SelectedMarker->ClearTrackedTarget();
		}

		return;
	}

	EnsureBattleMarkerActors();
	HoveredMarker = HoveredTargetMarkerActor.Get();
	SelectedMarker = SelectedTargetMarkerActor.Get();

	if (HoveredMarker)
	{
		if (AActor* HoveredActor = HoveredEnemyActor.Get())
		{
			HoveredMarker->SetTrackedTarget(HoveredActor, IHoyoEnemyInterface::Execute_GetTargetLocation(HoveredActor));
		}
		else
		{
			HoveredMarker->ClearTrackedTarget();
		}
	}

	if (SelectedMarker)
	{
		if (AActor* SelectedActor = SelectedEnemyActor.Get())
		{
			SelectedMarker->SetTrackedTarget(SelectedActor, IHoyoEnemyInterface::Execute_GetTargetLocation(SelectedActor));
		}
		else
		{
			SelectedMarker->ClearTrackedTarget();
		}
	}
}

void AGameplayDemoPlayerController::InitializeGameplayDemoUI()
{
	RefreshGameplayDemoUIContext();
	EnsureGameplayDemoUIRootAndHUD();
}

void AGameplayDemoPlayerController::RefreshGameplayDemoUIContext()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UMyUIStoreSubsystem* UIStoreSubsystem = ULocalPlayer::GetSubsystem<UMyUIStoreSubsystem>(LocalPlayer))
	{
		UIStoreSubsystem->RefreshPlayerContext();
	}

	if (UGameplayDemoUIFlowSubsystem* UIFlowSubsystem = ULocalPlayer::GetSubsystem<UGameplayDemoUIFlowSubsystem>(LocalPlayer))
	{
		UIFlowSubsystem->RefreshFlowBindings();
	}
}

void AGameplayDemoPlayerController::EnsureGameplayDemoUIRootAndHUD()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer || !IsLocalController())
	{
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
	{
		PlayerUISubsystem->EnsureRootLayout();
		if (!PlayerUISubsystem->FindScreenByTag(FGameplayDemoUIConstants::HUDScreenTag()))
		{
			PlayerUISubsystem->OpenScreen(FGameplayDemoUIConstants::HUDScreenTag(), FMyUIPayload());
		}
	}
}

void AGameplayDemoPlayerController::ToggleInventoryScreen()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer);
	if (!PlayerUISubsystem)
	{
		return;
	}

	if (UMyScreenBase* ExistingInventory = PlayerUISubsystem->FindScreenByTag(FGameplayDemoUIConstants::InventoryScreenTag()))
	{
		if (ExistingInventory->IsActivated())
		{
			PlayerUISubsystem->CloseScreen(ExistingInventory);
			return;
		}
	}

	PlayerUISubsystem->OpenScreen(FGameplayDemoUIConstants::InventoryScreenTag(), FMyUIPayload());
}

void AGameplayDemoPlayerController::OpenPauseMenu()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
	{
		PlayerUISubsystem->OpenScreen(TEXT("PauseMenu"), FMyUIPayload());
	}
}

void AGameplayDemoPlayerController::OpenCharacterDetailPanel()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;
	if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
	{
		PlayerUISubsystem->OpenScreen(TEXT("CharacterPanel"), FMyUIPayload());
	}
}

void AGameplayDemoPlayerController::TestPyro(const FInputActionValue& Value)
{
	SendElementToTarget(EGenshinElementType::Pyro, 100.0f);
}

void AGameplayDemoPlayerController::TestHydro(const FInputActionValue& Value)
{
	SendElementToTarget(EGenshinElementType::Hydro, 100.0f);
}

void AGameplayDemoPlayerController::TestCryo(const FInputActionValue& Value)
{
	SendElementToTarget(EGenshinElementType::Cryo, 100.0f);
}

void AGameplayDemoPlayerController::SendElementToTarget(EGenshinElementType Element, float BaseDamage)
{
	AElementDummyTarget* TargetActor = DebugTarget.Get();
	if (!TargetActor)
	{
		FindDebugTarget();
		TargetActor = DebugTarget.Get();
	}

	if (!TargetActor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("No ElementDummyTarget found in level"));
		}

		UE_LOG(LogTemp, Warning, TEXT("GameplayDemoPlayerController: DebugTarget is null"));
		return;
	}

	FElementHitEvent HitEvent;
	HitEvent.IncomingElement = Element;
	HitEvent.BaseDamage = BaseDamage;
	HitEvent.SourceStats.ElementalMastery = 200.0f;
	HitEvent.SourceStats.CharacterLevel = 80;

	TargetActor->ApplyElementHit(HitEvent);

	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("Sent Element: %d, Damage: %.1f"), static_cast<int32>(Element), BaseDamage);
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);
	}
}
