#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class FWidgetBlueprintEditor;

class FJsonWidgetBlueprintSemanticsTabSummoner : public FWorkflowTabFactory
{
public:
	static const FName TabID;

	explicit FJsonWidgetBlueprintSemanticsTabSummoner(TSharedPtr<FWidgetBlueprintEditor> InWidgetBlueprintEditor);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

private:
	TWeakPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor;
};
