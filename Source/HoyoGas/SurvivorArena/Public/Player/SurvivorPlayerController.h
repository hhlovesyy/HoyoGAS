#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SurvivorPlayerController.generated.h"

class ASurvivorCharacter;
class ASurvivorPlayerCameraManager;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class HOYOGAS_API ASurvivorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASurvivorPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

protected:
	void AddSurvivorInputMappingContext();
	void RemoveSurvivorInputMappingContext();
	void BindSurvivorInputActions();
	void HandleMove(const FInputActionValue& Value);
	void HandleAim(const FInputActionValue& Value);
	void HandlePrimaryFire(const FInputActionValue& Value);
	void HandleDash(const FInputActionValue& Value);
	void OpenSurvivorPauseMenu();
	void InitializeSurvivorUI();
	void RefreshPossessedPawnState();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputMappingContext> SurvivorMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	int32 SurvivorMappingPriority = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputAction> PrimaryFireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SurvivorArena|Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(Transient)
	TWeakObjectPtr<ASurvivorCharacter> CachedSurvivorCharacter;

	FVector2D LastAimInput = FVector2D::ZeroVector;
};
