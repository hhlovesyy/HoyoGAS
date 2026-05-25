// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoyoBattleTargetMarkerActor.generated.h"

class APlayerController;
class USceneComponent;
class UTextRenderComponent;
class UHoyoEnemyInterface;

UCLASS()
class HOYOGAS_API AHoyoBattleTargetMarkerActor : public AActor
{
	GENERATED_BODY()

public:
	AHoyoBattleTargetMarkerActor();

	virtual void Tick(float DeltaSeconds) override;

	void SetTrackedTarget(AActor* InTargetActor, const FVector& InWorldLocation);
	void ClearTrackedTarget();
	bool IsTrackingTarget() const;
	void ConfigureMarker(const FText& InMarkerText, const FColor& InMarkerColor, float InTextWorldSize, float InVerticalOffset);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	FText MarkerText = INVTEXT("[TARGET]");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	FColor MarkerColor = FColor(255, 64, 128);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker", meta = (ClampMin = "10.0"))
	float VerticalOffset = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker", meta = (ClampMin = "0.1"))
	float TextWorldSize = 36.0f;

private:
	void RefreshMarkerVisuals();
	void UpdateTransformFromTrackedTarget();
	void FaceOwningPlayerCamera();

	TWeakObjectPtr<AActor> TargetActor;
	FVector TrackedWorldLocation = FVector::ZeroVector;
	TWeakObjectPtr<APlayerController> OwningPlayerController;
};
