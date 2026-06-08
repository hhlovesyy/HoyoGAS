#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SurvivorCardDefinition.generated.h"

class UGameplayEffect;
class UTexture2D;
class USurvivorAbilitySet;

UCLASS(BlueprintType)
class HOYOGAS_API USurvivorCardDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//卡牌按照entry+stack存
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	FName CardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "1"))
	int32 MaxStack = 1; //每一层stack都单独记录：FActiveGameplayEffectHandle，FGameplayAbilitySpecHandle

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	bool bUnique = true; //阻止重复装备

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	FGameplayTagContainer CardTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	TArray<TObjectPtr<USurvivorAbilitySet>> AbilitySetsToGrant;

	bool ValidateDefinition(FString* OutError = nullptr) const;
};
