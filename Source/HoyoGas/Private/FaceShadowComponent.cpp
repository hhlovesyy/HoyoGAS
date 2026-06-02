// Fill out your copyright notice in the Description page of Project Settings.


#include "FaceShadowComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	struct FSDFRemapPoint
	{
		float ShadowCoverage;
		float SDFLocation;
	};

	float RemapShadowCoverageToSDF(float ShadowCoverage, const TArray<FSDFRemapPoint>& Points)
	{
		if (Points.Num() == 0)
		{
			return 0.0f;
		}

		if (ShadowCoverage <= Points[0].ShadowCoverage)
		{
			return Points[0].SDFLocation;
		}

		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			const FSDFRemapPoint& Prev = Points[Index - 1];
			const FSDFRemapPoint& Next = Points[Index];
			if (ShadowCoverage <= Next.ShadowCoverage)
			{
				const float Range = Next.ShadowCoverage - Prev.ShadowCoverage;
				const float Alpha = Range > KINDA_SMALL_NUMBER ? (ShadowCoverage - Prev.ShadowCoverage) / Range : 0.0f;
				return FMath::Lerp(Prev.SDFLocation, Next.SDFLocation, Alpha);
			}
		}

		return Points.Last().SDFLocation;
	}

	FVector GetAxisVector(const FTransform& Transform, EFaceShadowAxis Axis)
	{
		switch (Axis)
		{
		case EFaceShadowAxis::PositiveX:
			return Transform.GetUnitAxis(EAxis::X);
		case EFaceShadowAxis::NegativeX:
			return -Transform.GetUnitAxis(EAxis::X);
		case EFaceShadowAxis::PositiveY:
			return Transform.GetUnitAxis(EAxis::Y);
		case EFaceShadowAxis::NegativeY:
			return -Transform.GetUnitAxis(EAxis::Y);
		case EFaceShadowAxis::PositiveZ:
			return Transform.GetUnitAxis(EAxis::Z);
		case EFaceShadowAxis::NegativeZ:
			return -Transform.GetUnitAxis(EAxis::Z);
		default:
			return Transform.GetUnitAxis(EAxis::X);
		}
	}
}

UFaceShadowComponent::UFaceShadowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UFaceShadowComponent::SetCharacterMeshComponent(USkeletalMeshComponent* InCharacterMesh)
{
	CharacterMesh = InCharacterMesh;
	FaceMID = nullptr;
}

void UFaceShadowComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!CharacterMesh)
	{
		AActor* OwnerActor = GetOwner();
		if (OwnerActor)
		{
			TArray<USkeletalMeshComponent*> AllMeshes;
			OwnerActor->GetComponents<USkeletalMeshComponent>(AllMeshes);
			for (USkeletalMeshComponent* Mesh : AllMeshes)
			{
				if (Mesh->GetName() == "GenshinMesh")
				{
					CharacterMesh = Mesh;
					break;
				}
			}
		}
	}

	if (!CharacterMesh)
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			CharacterMesh = OwnerCharacter->GetMesh();
		}
	}

	if (CharacterMesh)
	{
		UMaterialInterface* OrigMaterial = CharacterMesh->GetMaterial(MaterialIndex);
		if (OrigMaterial)
		{
			FaceMID = CharacterMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(MaterialIndex, OrigMaterial);
		}
	}

	if (!MainLight)
	{
		AActor* FoundLight = UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass());
		MainLight = Cast<ADirectionalLight>(FoundLight);
	}
}

void UFaceShadowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MainLight || !CharacterMesh || !FaceMID)
	{
		return;
	}

	FTransform ReferenceTransform = CharacterMesh->GetComponentTransform();
	if (bUseFaceReference)
	{
		const FName ReferenceName = !FaceReferenceName.IsNone() ? FaceReferenceName : HeadBoneName;
		ReferenceTransform = CharacterMesh->GetSocketTransform(ReferenceName, RTS_World);
	}

	FVector HeadForward = GetAxisVector(ReferenceTransform, FaceForwardAxis);
	FVector HeadRight = GetAxisVector(ReferenceTransform, FaceRightAxis);

	if (bInvertHeadForward)
	{
		HeadForward *= -1.0f;
	}

	if (bInvertHeadRight)
	{
		HeadRight *= -1.0f;
	}

	HeadForward.Z = 0.0f;
	HeadRight.Z = 0.0f;
	if (!HeadForward.Normalize() || !HeadRight.Normalize())
	{
		return;
	}

	FVector LightDir = -MainLight->GetActorForwardVector();
	LightDir.Z = 0.0f;
	LightDir.Normalize();

	const float ForwardDot = FVector::DotProduct(HeadForward, LightDir);
	const float RightDot = FVector::DotProduct(HeadRight, LightDir);

	// 0.0 = front lit (0 deg), 0.5 = side lit (90 deg), 1.0 = back lit (180 deg).
	const float ShadowAngle = FMath::Acos(FMath::Clamp(ForwardDot, -1.0f, 1.0f));
	const float ShadowProgress = FMath::Clamp(ShadowAngle / PI, 0.0f, 1.0f);

	float FinalSDFLocation = 0.0f;

	// SDF_Location is not a linear "shadow ratio".
	// It is the material's own threshold space, and the left/right ranges are strongly asymmetric.
	// The anchor points below are based on in-editor observations of actual face coverage.
	if (RightDot <= 0.0f)
	{
		static const TArray<FSDFRemapPoint> RightShadowMap = {
			{0.0f, 0.0f},
			{1.0f / 3.0f, 0.1f},
			{0.5f, 0.2f},
			{2.0f / 3.0f, 0.3f},
			{0.8f, 0.5f},
			{1.0f, 1.0f}
		};

		FinalSDFLocation = RemapShadowCoverageToSDF(ShadowProgress, RightShadowMap);
	}
	else
	{
		static const TArray<FSDFRemapPoint> LeftShadowMap = {
			{0.0f, 0.0f},
			{1.0f / 6.0f, -0.5f},
			{0.2f, -0.6f},
			{1.0f / 3.0f, -0.7f},
			{0.5f, -0.8f},
			{0.6f, -0.9f},
			{5.0f / 6.0f, -0.95f},
			{1.0f, -1.0f}
		};

		FinalSDFLocation = RemapShadowCoverageToSDF(ShadowProgress, LeftShadowMap);
	}

	FinalSDFLocation = FMath::Clamp(FinalSDFLocation, -1.0f, 1.0f);

	//UE_LOG(LogTemp, Warning, TEXT("ForwardDot: %.6f, RightDot: %.6f, ShadowAngle: %.6f, ShadowProgress: %.6f, FinalSDF: %.6f"),
		//ForwardDot, RightDot, ShadowAngle, ShadowProgress, FinalSDFLocation);

	FaceMID->SetScalarParameterValue(FName("SDF_Location"), FinalSDFLocation);
}
