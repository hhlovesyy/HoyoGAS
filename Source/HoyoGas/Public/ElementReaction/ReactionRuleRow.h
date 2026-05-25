#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ElementTypes.h"
#include "ReactionRuleRow.generated.h"

USTRUCT(BlueprintType)
struct FReactionRuleRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGenshinElementType OldAura = EGenshinElementType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGenshinElementType IncomingElement = EGenshinElementType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApplyFrozen = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AuraConsumeValue = 1.0f; // 这条反应会消耗多少旧 Aura，单位要和 DefaultAuraValue 保持一致
};
