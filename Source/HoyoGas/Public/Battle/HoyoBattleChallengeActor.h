// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HoyoInteractableInterface.h"
#include "HoyoBattleChallengeActor.generated.h"

class UHoyoBattleEncounterDefinition;
class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UMaterialInstanceDynamic;

UCLASS()
class HOYOGAS_API AHoyoBattleChallengeActor : public AActor, public IHoyoInteractableInterface
{
	GENERATED_BODY()

public:
	AHoyoBattleChallengeActor();

	virtual bool CanInteract_Implementation(APlayerController* InteractingController) const override;
	virtual FText GetInteractionText_Implementation() const override;
	virtual void Interact_Implementation(APlayerController* InteractingController) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void ShowInteractionPrompt() const;
	void HideInteractionPrompt() const;

	UFUNCTION()
	void HandleInteractionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHoyoBattleEncounterDefinition> EncounterDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	FText InteractionText = INVTEXT("Enter battle");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true", ClampMin = "50.0"))
	float InteractionRadius = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	FLinearColor DisplayColor = FLinearColor(0.42f, 0.12f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	FLinearColor InRangeDisplayColor = FLinearColor(0.9f, 0.45f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	bool bDebugLogRangeChanges = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	bool bShowOnScreenInteractionPrompt = true;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	bool bPlayerInRange = false;
};
