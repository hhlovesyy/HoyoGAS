#include "JsonWidgetBlueprintSemanticsTabSummoner.h"

#include "SJsonWidgetBlueprintSemanticsPanel.h"

#include "Styling/AppStyle.h"
#include "WidgetBlueprintEditor.h"

#define LOCTEXT_NAMESPACE "JsonWidgetBlueprintSemanticsTabSummoner"

const FName FJsonWidgetBlueprintSemanticsTabSummoner::TabID(TEXT("JsonWidgetBlueprintSemantics"));

FJsonWidgetBlueprintSemanticsTabSummoner::FJsonWidgetBlueprintSemanticsTabSummoner(TSharedPtr<FWidgetBlueprintEditor> InWidgetBlueprintEditor)
	: FWorkflowTabFactory(TabID, InWidgetBlueprintEditor)
	, WidgetBlueprintEditor(InWidgetBlueprintEditor)
{
	TabLabel = LOCTEXT("TabLabel", "UI Semantics");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details");
	bIsSingleton = true;
	ViewMenuDescription = LOCTEXT("ViewMenuDescription", "UI Semantics");
	ViewMenuTooltip = LOCTEXT("ViewMenuTooltip", "Show UI semantic annotations for the selected widget.");
}

TSharedRef<SWidget> FJsonWidgetBlueprintSemanticsTabSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SJsonWidgetBlueprintSemanticsPanel)
		.WidgetBlueprintEditor(WidgetBlueprintEditor);
}

#undef LOCTEXT_NAMESPACE
