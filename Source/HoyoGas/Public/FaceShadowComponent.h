// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FaceShadowComponent.generated.h"

class ADirectionalLight;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EFaceShadowAxis : uint8
{
	PositiveX,
	NegativeX,
	PositiveY,
	NegativeY,
	PositiveZ,
	NegativeZ
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HOYOGAS_API UFaceShadowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaceShadowComponent();

	UFUNCTION(BlueprintCallable, Category = "Face Shadow")
	void SetCharacterMeshComponent(USkeletalMeshComponent* InCharacterMesh);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	ADirectionalLight* MainLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	FName HeadBoneName = FName("Head");

	// Optional socket or bone used as the face reference frame. If empty, the mesh component transform is used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	FName FaceReferenceName = FName("HeadSocket");
	//FName FaceReferenceName = NAME_None;

	// Head bone axes are often unreliable on imported rigs. Disable this to use the mesh component orientation instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	bool bUseFaceReference = true;

	// Which axis of the face reference points out of the face.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	EFaceShadowAxis FaceForwardAxis = EFaceShadowAxis::PositiveZ;

	// Which axis of the face reference points to the character's right cheek.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	EFaceShadowAxis FaceRightAxis = EFaceShadowAxis::NegativeX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	bool bInvertHeadForward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	bool bInvertHeadRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow")
	int32 MaterialIndex = 3;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face Shadow", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> CharacterMesh = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* FaceMID = nullptr;
};
