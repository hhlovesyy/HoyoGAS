#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimNode_SkirtCorrection.h"
#include "AnimGraphNode_SkirtCorrection.generated.h"

UCLASS()
class SKIRTCORRECTIONEDITOR_API UAnimGraphNode_SkirtCorrection : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SkirtCorrection Node;

public:
	virtual FText GetControllerDescription() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

protected:
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override
	{
		return &Node;
	}
};