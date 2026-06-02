#include "Player/SurvivorPlayerController.h"

#include "Core/SurvivorArenaLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/SurvivorCharacter.h"
#include "Player/SurvivorPlayerCameraManager.h"
#include "Subsystems/MyPlayerUISubsystem.h"
#include "UI/SurvivorUIConstants.h"
#include "UIFrameworkTypes.h"
//PlayerController（私密的）：只存在于服务器和控制该角色的本地客户端。假设房间里有 4 个玩家，你的电脑里只有 1 个 PlayerController（你自己的）。你绝对拿不到其他 3 个玩家的 Controller，因为那是他们的“私密大脑”，里面包含输入按键和 UI。
ASurvivorPlayerController::ASurvivorPlayerController()
{
	bShowMouseCursor = true;
	PlayerCameraManagerClass = ASurvivorPlayerCameraManager::StaticClass(); //只要写这个，就会自动把相机和角色关联起来，非常牛了
}

void ASurvivorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorPlayerController BeginPlay: %s"), *GetNameSafe(this));
	AddSurvivorInputMappingContext();
	InitializeSurvivorUI();
}

void ASurvivorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveSurvivorInputMappingContext();
	Super::EndPlay(EndPlayReason);
}

void ASurvivorPlayerController::SetupInputComponent()
{
	//这个函数是由引擎在后台自动、极早期调用的。
	/*
	它的生命周期大概是这样的：
	玩家加入游戏。
	引擎生成 PlayerController。
	引擎为这个 Controller 创建一个 InputComponent（用来监听键盘鼠标的组件）。
	立刻触发 SetupInputComponent()。
	之后再去执行 BeginPlay 或者 OnPossess（附身角色）。
	所以，在这个函数里把按键映射（如 MoveAction、PrimaryFireAction）用 BindAction 绑定到具体的 C++ 函数上是最安全的，它保证了只要 Controller 活着，按键监听就一直有效。
	 */
	Super::SetupInputComponent();
	BindSurvivorInputActions();
}

void ASurvivorPlayerController::OnPossess(APawn* InPawn)
{
	//这只在服务器上执行。服务器说：“好，你现在控制这个角色了。”
	Super::OnPossess(InPawn);
	RefreshPossessedPawnState();
}

void ASurvivorPlayerController::AcknowledgePossession(APawn* P)
{
	//AcknowledgePossession：这只在客户端上执行。客户端收到网络包后确认：“收到，我知道我控制这个角色了。”
	//通过在这两个地方都调用 RefreshPossessedPawnState() 去缓存当前的 Pawn 和初始化 UI，完美避免了网络延迟导致的“UI 出来了但拿不到血量”的经典时序 Bug。
	Super::AcknowledgePossession(P);
	RefreshPossessedPawnState();
}

void ASurvivorPlayerController::AddSurvivorInputMappingContext()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer) : nullptr;
	if (!InputSubsystem)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorPlayerController %s could not add input mapping because EnhancedInput subsystem is unavailable."), *GetNameSafe(this));
		return;
	}

	if (!SurvivorMappingContext)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorPlayerController %s has no SurvivorMappingContext configured."), *GetNameSafe(this));
		return;
	}

	InputSubsystem->AddMappingContext(SurvivorMappingContext, SurvivorMappingPriority);
	UE_LOG(LogSurvivorArena, Log, TEXT("Added Survivor input mapping context for %s."), *GetNameSafe(this));
}

void ASurvivorPlayerController::RemoveSurvivorInputMappingContext()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer) : nullptr;
	if (!InputSubsystem || !SurvivorMappingContext)
	{
		return;
	}

	InputSubsystem->RemoveMappingContext(SurvivorMappingContext);
	UE_LOG(LogSurvivorArena, Log, TEXT("Removed Survivor input mapping context for %s."), *GetNameSafe(this));
}

void ASurvivorPlayerController::BindSurvivorInputActions()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("SurvivorPlayerController %s requires an EnhancedInputComponent."), *GetNameSafe(this));
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurvivorPlayerController::HandleMove);
	}

	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ASurvivorPlayerController::HandleAim);
	}

	if (PrimaryFireAction)
	{
		EnhancedInputComponent->BindAction(PrimaryFireAction, ETriggerEvent::Started, this, &ASurvivorPlayerController::HandlePrimaryFire);
	}

	if (DashAction)
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ASurvivorPlayerController::HandleDash);
	}

	if (PauseAction)
	{
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ASurvivorPlayerController::OpenSurvivorPauseMenu);
	}
}

void ASurvivorPlayerController::HandleMove(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentControlRotation = GetControlRotation(); //牢记 UE 里最核心的哲学：Controller（大脑）和 Pawn（肉体）是分离的。GetActorRotation() 得到的是“肉体”当前的朝向（比如角色正看着左边）。GetControlRotation() 得到的是“大脑”或者说“玩家的视线/意图”的朝向。
	//在《土豆兄弟》这种俯视角游戏里，如果加入了“鼠标瞄准射击”的机制（类似于你在 PlayerController 里写的 HandleAim），GetControlRotation() 通常会被用来追踪玩家鼠标悬停在 3D 世界里的那个点。
	const FRotator YawRotation(0.0f, CurrentControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
}

void ASurvivorPlayerController::HandleAim(const FInputActionValue& Value)
{
	LastAimInput = Value.Get<FVector2D>();
}

void ASurvivorPlayerController::HandlePrimaryFire(const FInputActionValue& Value)
{
	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor primary fire requested by %s."), *GetNameSafe(this));
}

void ASurvivorPlayerController::HandleDash(const FInputActionValue& Value)
{
	UE_LOG(LogSurvivorArena, Log, TEXT("Survivor dash requested by %s."), *GetNameSafe(this));
}

void ASurvivorPlayerController::OpenSurvivorPauseMenu()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UMyPlayerUISubsystem* PlayerUISubsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer) : nullptr;
	if (!PlayerUISubsystem)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Pause menu request failed because MyPlayerUISubsystem is unavailable."));
		return;
	}

	if (!PlayerUISubsystem->OpenScreen(SurvivorUIConstants::SurvivorPauseScreenTag, FMyUIPayload()))
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Pause menu screen tag '%s' is not registered."), *SurvivorUIConstants::SurvivorPauseScreenTag.ToString());
	}
}

void ASurvivorPlayerController::InitializeSurvivorUI()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UMyPlayerUISubsystem* PlayerUISubsystem = LocalPlayer ? ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer) : nullptr;
	if (!PlayerUISubsystem)
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Survivor UI initialization skipped because MyPlayerUISubsystem is unavailable."));
		return;
	}

	PlayerUISubsystem->EnsureRootLayout();
	if (!PlayerUISubsystem->OpenScreen(SurvivorUIConstants::SurvivorHUDScreenTag, FMyUIPayload()))
	{
		UE_LOG(LogSurvivorArena, Warning, TEXT("Survivor HUD screen tag '%s' is not registered."), *SurvivorUIConstants::SurvivorHUDScreenTag.ToString());
	}
}

void ASurvivorPlayerController::RefreshPossessedPawnState()
{
	CachedSurvivorCharacter = Cast<ASurvivorCharacter>(GetPawn());
	UE_LOG(LogSurvivorArena, Log, TEXT("SurvivorPlayerController refreshed possessed pawn. Pawn=%s"), *GetNameSafe(GetPawn()));

	if (IsLocalController())  //在你的电脑上，只有你的 Controller 会返回 true，其他玩家的Controller不会进这里，避免多次打开UI之类的；
	{
		InitializeSurvivorUI();
	}
}
