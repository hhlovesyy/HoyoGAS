#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;

class SJsonWidgetBlueprintImporterWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SJsonWidgetBlueprintImporterWindow)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleBrowseJsonClicked();
	FReply HandleImportClicked();
	FReply HandleImportCharacterPanelTemplateClicked();
	FReply HandleBrowseManifestClicked();
	FReply HandleGenerateBoilerplateClicked();
	ECheckBoxState GetOpenBlueprintCheckState() const;
	void HandleOpenBlueprintCheckChanged(ECheckBoxState NewState);
	ECheckBoxState GetOverwriteFilesCheckState() const;
	void HandleOverwriteFilesCheckChanged(ECheckBoxState NewState);
	void SetStatusMessage(const FString& InMessage);

private:
	TSharedPtr<SEditableTextBox> JsonPathTextBox;
	TSharedPtr<SEditableTextBox> TargetFolderTextBox;
	TSharedPtr<SEditableTextBox> ManifestPathTextBox;
	TSharedPtr<SEditableTextBox> TargetSourceRootTextBox;
	TSharedPtr<SMultiLineEditableTextBox> StatusTextBox;
	bool bOpenBlueprintAfterImport = true;
	bool bOverwriteExistingFiles = false;
};
