// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "HoyoGasPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UCharacterEquipmentComponent;
class UInventoryComponent;
class UScoreProgressionComponent;
class UHoyoProgressEventBridgeComponent;

UCLASS()
class HOYOGAS_API AHoyoGasPlayerState : public APlayerState ,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHoyoGasPlayerState();
	
	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UScoreProgressionComponent* GetScoreProgressionComponent() const { return ScoreProgressionComponent; }
	FORCEINLINE UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UScoreProgressionComponent> ScoreProgressionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterGrowth", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterEquipmentComponent> CharacterEquipmentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Progression, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHoyoProgressEventBridgeComponent> ProgressEventBridgeComponent;
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
