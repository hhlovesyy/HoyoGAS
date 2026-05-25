#include "SJsonWidgetBlueprintSemanticsPanel.h"

#include "JsonWidgetBlueprintSemantics.h"

#include "Components/Widget.h"
#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"
#include "WidgetReference.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "JsonWidgetBlueprintSemanticsPanel"

namespace
{
	TSharedRef<SWidget> MakeFieldLabel(const FText& Label)
	{
		return SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10));
	}

	TSharedRef<SWidget> MakeFieldContainer(const FText& Label, TSharedRef<SWidget> FieldWidget)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 4.0f)
			[
				MakeFieldLabel(Label)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				FieldWidget
			];
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SJsonWidgetBlueprintSemanticsPanel::Construct(const FArguments& InArgs)
{
	WidgetBlueprintEditor = InArgs._WidgetBlueprintEditor;

	if (TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		WidgetBlueprintEditorPinned->OnSelectedWidgetsChanged.AddSP(this, &SJsonWidgetBlueprintSemanticsPanel::HandleSelectionChanged);
		WidgetBlueprintEditorPinned->OnSelectedWidgetsChanging.AddSP(this, &SJsonWidgetBlueprintSemanticsPanel::HandleSelectionChanged);
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(10.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelTitle", "UI Semantics"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetBlueprintNameText)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetSelectedWidgetSummaryText)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ExportManifestButton", "Export UI Manifest"))
					.OnClicked(this, &SJsonWidgetBlueprintSemanticsPanel::HandleExportManifestClicked)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f, 0.0f, 0.0f)
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex_Lambda([this]() { return IsEditingEnabled() ? 1 : 0; })
					+ SWidgetSwitcher::Slot()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SelectionHint", "Select a single widget in the Widget Blueprint designer to edit BindWidget, callback, ViewModel field and semantic role."))
						.AutoWrapText(true)
					]
					+ SWidgetSwitcher::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeFieldContainer(
								LOCTEXT("BindWidgetLabel", "BindWidget"),
								SNew(SCheckBox)
								.IsChecked(this, &SJsonWidgetBlueprintSemanticsPanel::GetBindWidgetCheckState)
								.OnCheckStateChanged(this, &SJsonWidgetBlueprintSemanticsPanel::HandleBindWidgetCheckChanged)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BindWidgetHint", "Expose this widget for BindWidget / code access"))
								])
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeFieldContainer(
								LOCTEXT("CallbackNameLabel", "Callback Name"),
								SNew(SEditableTextBox)
								.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetCallbackNameText)
								.HintText(LOCTEXT("CallbackNameHint", "HandleCloseClicked"))
								.OnTextCommitted(this, &SJsonWidgetBlueprintSemanticsPanel::HandleCallbackNameCommitted))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeFieldContainer(
								LOCTEXT("ViewModelFieldLabel", "ViewModel Field"),
								SNew(SEditableTextBox)
								.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetViewModelFieldText)
								.HintText(LOCTEXT("ViewModelFieldHint", "ActivityTitleText"))
								.OnTextCommitted(this, &SJsonWidgetBlueprintSemanticsPanel::HandleViewModelFieldCommitted))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeFieldContainer(
								LOCTEXT("SemanticRoleLabel", "Semantic Role"),
								SNew(SEditableTextBox)
								.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetSemanticRoleText)
								.HintText(LOCTEXT("SemanticRoleHint", "button.close"))
								.OnTextCommitted(this, &SJsonWidgetBlueprintSemanticsPanel::HandleSemanticRoleCommitted))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeFieldContainer(
								LOCTEXT("NotesLabel", "Notes"),
								SNew(SEditableTextBox)
								.Text(this, &SJsonWidgetBlueprintSemanticsPanel::GetNotesText)
								.HintText(LOCTEXT("NotesHint", "Optional editor note for later codegen or review"))
								.OnTextCommitted(this, &SJsonWidgetBlueprintSemanticsPanel::HandleNotesCommitted))
						]
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SJsonWidgetBlueprintSemanticsPanel::HandleExportManifestClicked()
{
	JsonWidgetBlueprintSemantics::ExportManifestWithDialog(WidgetBlueprintEditor.Pin());
	return FReply::Handled();
}

void SJsonWidgetBlueprintSemanticsPanel::HandleBindWidgetCheckChanged(ECheckBoxState NewState)
{
	if (UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		JsonWidgetBlueprintSemantics::SetBindWidget(WidgetBlueprint, GetSelectedWidget(), NewState == ECheckBoxState::Checked);
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}
}

void SJsonWidgetBlueprintSemanticsPanel::HandleCallbackNameCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	SaveMetadataValue(JsonWidgetBlueprintSemantics::CallbackNameMetaDataKey, NewText.ToString().TrimStartAndEnd());
}

void SJsonWidgetBlueprintSemanticsPanel::HandleNotesCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	SaveMetadataValue(JsonWidgetBlueprintSemantics::NotesMetaDataKey, NewText.ToString());
}

void SJsonWidgetBlueprintSemanticsPanel::HandleSemanticRoleCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	SaveMetadataValue(JsonWidgetBlueprintSemantics::SemanticRoleMetaDataKey, NewText.ToString().TrimStartAndEnd());
}

void SJsonWidgetBlueprintSemanticsPanel::HandleSelectionChanged()
{
	Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SJsonWidgetBlueprintSemanticsPanel::HandleViewModelFieldCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	SaveMetadataValue(JsonWidgetBlueprintSemantics::ViewModelFieldMetaDataKey, NewText.ToString().TrimStartAndEnd());
}

ECheckBoxState SJsonWidgetBlueprintSemanticsPanel::GetBindWidgetCheckState() const
{
	return JsonWidgetBlueprintSemantics::GetBindWidget(GetSelectedWidget())
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

FText SJsonWidgetBlueprintSemanticsPanel::GetBlueprintNameText() const
{
	if (const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		return FText::FromString(FString::Printf(
			TEXT("Widget Blueprint: %s | Parent: %s | Kind: %s"),
			*WidgetBlueprint->GetName(),
			WidgetBlueprint->ParentClass != nullptr ? *WidgetBlueprint->ParentClass->GetAuthoredName() : TEXT("UserWidget"),
			*JsonWidgetBlueprintSemantics::GetWidgetKind(WidgetBlueprint)));
	}

	return LOCTEXT("NoBlueprintLoaded", "Widget Blueprint: None");
}

FText SJsonWidgetBlueprintSemanticsPanel::GetCallbackNameText() const
{
	FString Value;
	JsonWidgetBlueprintSemantics::GetWidgetMetadataValue(GetSelectedWidget(), JsonWidgetBlueprintSemantics::CallbackNameMetaDataKey, Value);
	return FText::FromString(Value);
}

FText SJsonWidgetBlueprintSemanticsPanel::GetNotesText() const
{
	FString Value;
	JsonWidgetBlueprintSemantics::GetWidgetMetadataValue(GetSelectedWidget(), JsonWidgetBlueprintSemantics::NotesMetaDataKey, Value);
	return FText::FromString(Value);
}

FText SJsonWidgetBlueprintSemanticsPanel::GetSelectedWidgetSummaryText() const
{
	if (const TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		const TSet<FWidgetReference>& SelectedWidgets = WidgetBlueprintEditorPinned->GetSelectedWidgets();
		if (SelectedWidgets.Num() == 0)
		{
			return LOCTEXT("NoWidgetSelected", "Selected Widget: None");
		}

		if (SelectedWidgets.Num() > 1)
		{
			return FText::FromString(FString::Printf(TEXT("Selected Widget: %d widgets selected"), SelectedWidgets.Num()));
		}
	}

	if (const UWidget* Widget = GetSelectedWidget())
	{
		return FText::FromString(FString::Printf(
			TEXT("Selected Widget: %s (%s)"),
			*Widget->GetName(),
			*Widget->GetClass()->GetAuthoredName()));
	}

	return LOCTEXT("SelectedWidgetUnavailable", "Selected Widget: Unavailable");
}

FText SJsonWidgetBlueprintSemanticsPanel::GetSemanticRoleText() const
{
	FString Value;
	JsonWidgetBlueprintSemantics::GetWidgetMetadataValue(GetSelectedWidget(), JsonWidgetBlueprintSemantics::SemanticRoleMetaDataKey, Value);
	return FText::FromString(Value);
}

FText SJsonWidgetBlueprintSemanticsPanel::GetViewModelFieldText() const
{
	FString Value;
	JsonWidgetBlueprintSemantics::GetWidgetMetadataValue(GetSelectedWidget(), JsonWidgetBlueprintSemantics::ViewModelFieldMetaDataKey, Value);
	return FText::FromString(Value);
}

bool SJsonWidgetBlueprintSemanticsPanel::IsEditingEnabled() const
{
	if (const TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		return WidgetBlueprintEditorPinned->GetSelectedWidgets().Num() == 1 && GetSelectedWidget() != nullptr;
	}

	return false;
}

UWidget* SJsonWidgetBlueprintSemanticsPanel::GetSelectedWidget() const
{
	if (const TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		const TSet<FWidgetReference>& SelectedWidgets = WidgetBlueprintEditorPinned->GetSelectedWidgets();
		if (SelectedWidgets.Num() != 1)
		{
			return nullptr;
		}

		for (const FWidgetReference& WidgetReference : SelectedWidgets)
		{
			return WidgetReference.IsValid() ? WidgetReference.GetTemplate() : nullptr;
		}
	}

	return nullptr;
}

UWidgetBlueprint* SJsonWidgetBlueprintSemanticsPanel::GetWidgetBlueprint() const
{
	if (const TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		return WidgetBlueprintEditorPinned->GetWidgetBlueprintObj();
	}

	return nullptr;
}

void SJsonWidgetBlueprintSemanticsPanel::SaveMetadataValue(FName Key, const FString& Value)
{
	if (UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		JsonWidgetBlueprintSemantics::SetWidgetMetadataValue(WidgetBlueprint, GetSelectedWidget(), Key, Value);
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}
}

#undef LOCTEXT_NAMESPACE
