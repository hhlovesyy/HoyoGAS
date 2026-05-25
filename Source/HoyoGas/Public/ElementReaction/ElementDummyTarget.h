#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElementTypes.h"
#include "Engine/DataTable.h"
#include "ElementDummyTarget.generated.h"

class UAuraBarWidget;
class UWidgetComponent;
class UStaticMeshComponent;
class UDamageTextWidget;
class UMaterialInstanceDynamic;
class UParticleSystemComponent;

UCLASS()
class HOYOGAS_API AElementDummyTarget : public AActor
{
	GENERATED_BODY()

public:
	AElementDummyTarget();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element")
	FElementAura CurrentAura;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element")
	float Health = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Resistance = 0.1f; //基础抗性值 默认有 10% 抗性

	//冻结的情况需要单独建模
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Frozen")
	bool bIsFrozen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Frozen")
	float FrozenRemainingTime = 0.f;
	
	UFUNCTION(BlueprintCallable)
	void ApplyElementHit(const FElementHitEvent& HitEvent);
	
	UFUNCTION(BlueprintPure, Category = "Element|UI")
	float GetAuraPercent() const;

	UFUNCTION(BlueprintPure, Category = "Element|UI")
	FLinearColor GetAuraColor() const;

	UFUNCTION(BlueprintPure, Category = "Element|UI")
	FText GetAuraText() const;

	UFUNCTION(BlueprintPure, Category = "Element|UI")
	bool HasAura() const;

	UFUNCTION(BlueprintCallable, Category = "Element")
	void ClearAura();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element|Config")
	TObjectPtr<UDataTable> ReactionRuleDataTable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element|Config")
	TObjectPtr<UDataTable> ReactionVisualDataTable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element|Config")
	TObjectPtr<UDataTable> ReactionFXDataTable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element|Config")
	TObjectPtr<UDataTable> AuraConfigDataTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDamageTextWidget> DamageTextClass;
	
	
	UFUNCTION()
	void RefreshAuraWidget();
	
	void InitAuraWidget();
	
	void SpawnDamageText(const FReactionResult& Result);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void PrintDebug(const FString& Message) const;
	
	void PlayReactionVisual(const FReactionResult& Result);
	void ResetReactionVisual();
	void UpdateMaterialVisualState(const FReactionResult& Result);
	void TriggerReactionFX(const FReactionResult& Result);
	void ClearPersistentFrozenFX();
	
	void ApplyFrozenState(float Duration);
	void ClearFrozenState();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> AuraWidgetComponent;
	
	UPROPERTY()
	TObjectPtr<UAuraBarWidget> CachedAuraWidget;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ActiveFrozenFXComponent;
};
