#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FWidgetBlueprintEditor;
class UWidget;
class UWidgetBlueprint;

class SJsonWidgetBlueprintSemanticsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SJsonWidgetBlueprintSemanticsPanel)
	{
	}
		SLATE_ARGUMENT(TWeakPtr<FWidgetBlueprintEditor>, WidgetBlueprintEditor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleExportManifestClicked();
	void HandleBindWidgetCheckChanged(ECheckBoxState NewState);
	void HandleCallbackNameCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void HandleNotesCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void HandleSemanticRoleCommitted(const FText& NewText, ETextCommit::Type CommitType);
	void HandleSelectionChanged();
	void HandleViewModelFieldCommitted(const FText& NewText, ETextCommit::Type CommitType);

	ECheckBoxState GetBindWidgetCheckState() const;
	FText GetBlueprintNameText() const;
	FText GetCallbackNameText() const;
	FText GetNotesText() const;
	FText GetSelectedWidgetSummaryText() const;
	FText GetSemanticRoleText() const;
	FText GetViewModelFieldText() const;
	bool IsEditingEnabled() const;

	UWidget* GetSelectedWidget() const;
	UWidgetBlueprint* GetWidgetBlueprint() const;
	void SaveMetadataValue(FName Key, const FString& Value);

private:
	TWeakPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor;
};
