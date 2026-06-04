#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SurvivorFirePattern.generated.h"

USTRUCT(BlueprintType)
struct FSurvivorFirePatternContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Survivor|Weapon")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Survivor|Weapon")
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Survivor|Weapon")
	FVector ForwardDirection = FVector::ForwardVector;
};

UENUM(BlueprintType)
enum class ESurvivorRadialAlignment : uint8
{
	CharacterForward,
	WorldAxis
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class HOYOGAS_API USurvivorFirePattern : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Survivor|Weapon")
	void GenerateDirections(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class HOYOGAS_API USurvivorForwardFirePattern : public USurvivorFirePattern
{
	GENERATED_BODY()

public:
	virtual void GenerateDirections_Implementation(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const override;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class HOYOGAS_API USurvivorRadialFirePattern : public USurvivorFirePattern
{
	GENERATED_BODY()

public:
	virtual void GenerateDirections_Implementation(const FSurvivorFirePatternContext& Context, TArray<FVector>& OutDirections) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Weapon")
	int32 DirectionCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survivor|Weapon")
	ESurvivorRadialAlignment RadialAlignment = ESurvivorRadialAlignment::CharacterForward;
};
