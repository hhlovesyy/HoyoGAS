#include "AnimGraphNode_SkirtCorrection.h"

#define LOCTEXT_NAMESPACE "AnimGraphNode_SkirtCorrection"

FText UAnimGraphNode_SkirtCorrection::GetControllerDescription() const
{
	return LOCTEXT("ControllerDescription", "Skirt Correction");
}

FText UAnimGraphNode_SkirtCorrection::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Skirt Correction");
}

FText UAnimGraphNode_SkirtCorrection::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Empty skirt correction skeletal control node.");
}

#undef LOCTEXT_NAMESPACE