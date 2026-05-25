#include "SJsonWidgetBlueprintImporterWindow.h"

#include "JsonWidgetBlueprintBoilerplateGenerator.h"
#include "JsonWidgetBlueprintImporter.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "IDesktopPlatform.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SJsonWidgetBlueprintImporterWindow"

void SJsonWidgetBlueprintImporterWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 12.0f, 12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IntroText", "Import a JSON UI schema and generate a static Widget Blueprint tree."))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ImportSectionLabel", "Import Layout"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("JsonPathLabel", "JSON File"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SAssignNew(JsonPathTextBox, SEditableTextBox)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseJsonLabel", "Browse..."))
				.OnClicked(this, &SJsonWidgetBlueprintImporterWindow::HandleBrowseJsonClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetFolderLabel", "Target Content Folder"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SAssignNew(TargetFolderTextBox, SEditableTextBox)
			.Text(FText::FromString(TEXT("/Game/UI/Generated")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SJsonWidgetBlueprintImporterWindow::GetOpenBlueprintCheckState)
			.OnCheckStateChanged(this, &SJsonWidgetBlueprintImporterWindow::HandleOpenBlueprintCheckChanged)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OpenBlueprintLabel", "Open Blueprint after import"))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ImportButtonLabel", "Import JSON as Widget Blueprint"))
				.OnClicked(this, &SJsonWidgetBlueprintImporterWindow::HandleImportClicked)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ImportCharacterPanelTemplateLabel", "Import Character Panel Template"))
			.OnClicked(this, &SJsonWidgetBlueprintImporterWindow::HandleImportCharacterPanelTemplateClicked)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 16.0f, 12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BoilerplateSectionLabel", "Generate Boilerplate from Manifest"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ManifestPathLabel", "Manifest File"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SAssignNew(ManifestPathTextBox, SEditableTextBox)
				.Text(FText::FromString(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UIManifest/WBP_BattleScreen.ui.manifest.json"))))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseManifestLabel", "Browse..."))
				.OnClicked(this, &SJsonWidgetBlueprintImporterWindow::HandleBrowseManifestClicked)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetSourceRootLabel", "Target Source Root"))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 0.0f, 12.0f, 6.0f)
		[
			SAssignNew(TargetSourceRootTextBox, SEditableTextBox)
			.Text(FText::FromString(TEXT("Source/HoyoGas")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SJsonWidgetBlueprintImporterWindow::GetOverwriteFilesCheckState)
			.OnCheckStateChanged(this, &SJsonWidgetBlueprintImporterWindow::HandleOverwriteFilesCheckChanged)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OverwriteFilesLabel", "Overwrite existing generated files"))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(12.0f, 6.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("GenerateBoilerplateButtonLabel", "Generate Screen / ViewModel / Store Boilerplate"))
			.OnClicked(this, &SJsonWidgetBlueprintImporterWindow::HandleGenerateBoilerplateClicked)
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(12.0f, 6.0f, 12.0f, 12.0f)
		[
			SAssignNew(StatusTextBox, SMultiLineEditableTextBox)
			.IsReadOnly(true)
			.Text(FText::FromString(TEXT("Ready.")))
		]
	];
}

FReply SJsonWidgetBlueprintImporterWindow::HandleBrowseJsonClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		SetStatusMessage(TEXT("DesktopPlatform is unavailable."));
		return FReply::Handled();
	}

	TArray<FString> SelectedFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Choose JSON UI Schema"),
		FPaths::ProjectDir(),
		TEXT(""),
		TEXT("JSON files (*.json)|*.json|All files (*.*)|*.*"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (bOpened && SelectedFiles.Num() > 0)
	{
		JsonPathTextBox->SetText(FText::FromString(SelectedFiles[0]));
	}

	return FReply::Handled();
}

FReply SJsonWidgetBlueprintImporterWindow::HandleBrowseManifestClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		SetStatusMessage(TEXT("DesktopPlatform is unavailable."));
		return FReply::Handled();
	}

	TArray<FString> SelectedFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Choose UI Manifest"),
		FPaths::ProjectSavedDir(),
		TEXT(""),
		TEXT("JSON files (*.json)|*.json|All files (*.*)|*.*"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (bOpened && SelectedFiles.Num() > 0 && ManifestPathTextBox.IsValid())
	{
		ManifestPathTextBox->SetText(FText::FromString(SelectedFiles[0]));
	}

	return FReply::Handled();
}

FReply SJsonWidgetBlueprintImporterWindow::HandleImportClicked()
{
	FJsonWidgetBlueprintImportRequest Request;
	Request.JsonFilePath = JsonPathTextBox.IsValid() ? JsonPathTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	Request.TargetContentFolder = TargetFolderTextBox.IsValid() ? TargetFolderTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	Request.bOpenBlueprintAfterImport = bOpenBlueprintAfterImport;

	const FJsonWidgetBlueprintImportResult Result = FJsonWidgetBlueprintImporter::ImportFromFile(Request);

	FString Message = Result.bSuccess
		? FString::Printf(TEXT("Import succeeded.\nAsset: %s"), *Result.AssetPath)
		: TEXT("Import failed.");

	if (Result.Errors.Num() > 0)
	{
		Message += TEXT("\n\nErrors:");
		for (const FString& Error : Result.Errors)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Error);
		}
	}

	if (Result.Warnings.Num() > 0)
	{
		Message += TEXT("\n\nWarnings:");
		for (const FString& Warning : Result.Warnings)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Warning);
		}
	}

	SetStatusMessage(Message);
	return FReply::Handled();
}

FReply SJsonWidgetBlueprintImporterWindow::HandleImportCharacterPanelTemplateClicked()
{
	FJsonWidgetBlueprintImportRequest Request;
	Request.JsonFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/HoyoGas/CharacterPanel/UI/WBP_CharacterPanelScreen.schema.json"));
	Request.TargetContentFolder = TEXT("/Game/UI/CharacterPanel");
	Request.bOpenBlueprintAfterImport = bOpenBlueprintAfterImport;

	const FJsonWidgetBlueprintImportResult Result = FJsonWidgetBlueprintImporter::ImportFromFile(Request);

	FString Message = Result.bSuccess
		? FString::Printf(TEXT("Character panel template import succeeded.\nAsset: %s"), *Result.AssetPath)
		: TEXT("Character panel template import failed.");

	if (Result.Errors.Num() > 0)
	{
		Message += TEXT("\n\nErrors:");
		for (const FString& Error : Result.Errors)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Error);
		}
	}

	if (Result.Warnings.Num() > 0)
	{
		Message += TEXT("\n\nWarnings:");
		for (const FString& Warning : Result.Warnings)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Warning);
		}
	}

	SetStatusMessage(Message);
	return FReply::Handled();
}

FReply SJsonWidgetBlueprintImporterWindow::HandleGenerateBoilerplateClicked()
{
	FJsonWidgetBlueprintBoilerplateGenerationRequest Request;
	Request.ManifestFilePath = ManifestPathTextBox.IsValid() ? ManifestPathTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	Request.TargetSourceRoot = TargetSourceRootTextBox.IsValid() ? TargetSourceRootTextBox->GetText().ToString().TrimStartAndEnd() : FString();
	Request.bOverwriteExistingFiles = bOverwriteExistingFiles;

	const FJsonWidgetBlueprintBoilerplateGenerationResult Result =
		FJsonWidgetBlueprintBoilerplateGenerator::GenerateFromManifestFile(Request);

	FString Message = Result.bSuccess
		? TEXT("Boilerplate generation succeeded.")
		: TEXT("Boilerplate generation failed.");

	if (Result.GeneratedFiles.Num() > 0)
	{
		Message += TEXT("\n\nGenerated Files:");
		for (const FString& GeneratedFile : Result.GeneratedFiles)
		{
			Message += FString::Printf(TEXT("\n- %s"), *GeneratedFile);
		}
	}

	if (Result.Errors.Num() > 0)
	{
		Message += TEXT("\n\nErrors:");
		for (const FString& Error : Result.Errors)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Error);
		}
	}

	if (Result.Warnings.Num() > 0)
	{
		Message += TEXT("\n\nWarnings:");
		for (const FString& Warning : Result.Warnings)
		{
			Message += FString::Printf(TEXT("\n- %s"), *Warning);
		}
	}

	SetStatusMessage(Message);
	return FReply::Handled();
}

ECheckBoxState SJsonWidgetBlueprintImporterWindow::GetOpenBlueprintCheckState() const
{
	return bOpenBlueprintAfterImport ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SJsonWidgetBlueprintImporterWindow::HandleOpenBlueprintCheckChanged(ECheckBoxState NewState)
{
	bOpenBlueprintAfterImport = (NewState == ECheckBoxState::Checked);
}

ECheckBoxState SJsonWidgetBlueprintImporterWindow::GetOverwriteFilesCheckState() const
{
	return bOverwriteExistingFiles ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SJsonWidgetBlueprintImporterWindow::HandleOverwriteFilesCheckChanged(ECheckBoxState NewState)
{
	bOverwriteExistingFiles = (NewState == ECheckBoxState::Checked);
}

void SJsonWidgetBlueprintImporterWindow::SetStatusMessage(const FString& InMessage)
{
	if (StatusTextBox.IsValid())
	{
		StatusTextBox->SetText(FText::FromString(InMessage));
	}
}

#undef LOCTEXT_NAMESPACE
