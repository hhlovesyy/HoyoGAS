#include "JsonWidgetBlueprintSemantics.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/MetaData.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ContentWidget.h"
#include "Components/NamedSlotInterface.h"
#include "Components/PanelWidget.h"
#include "Editor.h"
#include "IDesktopPlatform.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"

namespace JsonWidgetBlueprintSemantics
{
	const FName CallbackNameMetaDataKey(TEXT("JsonWidgetImporter.CallbackName"));
	const FName NotesMetaDataKey(TEXT("JsonWidgetImporter.Notes"));
	const FName SemanticRoleMetaDataKey(TEXT("JsonWidgetImporter.SemanticRole"));
	const FName ViewModelFieldMetaDataKey(TEXT("JsonWidgetImporter.ViewModelField"));

	namespace
	{
		struct FAnnotatedWidgetRecord
		{
			FString CallbackName;
			FString Notes;
			FString SemanticRole;
			FString ViewModelField;
			FString WidgetClass;
			FString WidgetName;
			FString WidgetPath;
			bool bBindWidget = false;
		};

		void ShowNotification(const FText& Message, SNotificationItem::ECompletionState CompletionState)
		{
			FNotificationInfo Info(Message);
			Info.bFireAndForget = true;
			Info.FadeOutDuration = 0.2f;
			Info.ExpireDuration = 3.0f;

			if (TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info))
			{
				Notification->SetCompletionState(CompletionState);
			}
		}

		bool TryGetMetaData(const UWidget* Widget, UMetaData*& OutMetaData)
		{
			OutMetaData = nullptr;
			if (Widget == nullptr)
			{
				return false;
			}

			if (UPackage* Package = Widget->GetOutermost())
			{
				OutMetaData = Package->GetMetaData();
			}

			return OutMetaData != nullptr;
		}

		void TraverseWidgetRecursive(UWidget* Widget, const FString& ParentPath, TSet<const UWidget*>& VisitedWidgets, TFunctionRef<void(UWidget*, const FString&)> Visitor)
		{
			if (Widget == nullptr || VisitedWidgets.Contains(Widget))
			{
				return;
			}

			VisitedWidgets.Add(Widget);

			const FString WidgetPath = ParentPath.IsEmpty() ? Widget->GetName() : ParentPath / Widget->GetName();
			Visitor(Widget, WidgetPath);

			if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
			{
				for (int32 ChildIndex = 0; ChildIndex < PanelWidget->GetChildrenCount(); ++ChildIndex)
				{
					TraverseWidgetRecursive(PanelWidget->GetChildAt(ChildIndex), WidgetPath, VisitedWidgets, Visitor);
				}
			}
			else if (UContentWidget* ContentWidget = Cast<UContentWidget>(Widget))
			{
				TraverseWidgetRecursive(ContentWidget->GetContent(), WidgetPath, VisitedWidgets, Visitor);
			}

			if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(Widget))
			{
				TArray<FName> SlotNames;
				NamedSlotHost->GetSlotNames(SlotNames);

				for (const FName SlotName : SlotNames)
				{
					if (UWidget* SlotContent = NamedSlotHost->GetContentForSlot(SlotName))
					{
						TraverseWidgetRecursive(SlotContent, WidgetPath / SlotName.ToString(), VisitedWidgets, Visitor);
					}
				}
			}
		}

		void ForEachWidgetWithPath(const UWidgetBlueprint* WidgetBlueprint, TFunctionRef<void(UWidget*, const FString&)> Visitor)
		{
			if (WidgetBlueprint == nullptr || WidgetBlueprint->WidgetTree == nullptr)
			{
				return;
			}

			TSet<const UWidget*> VisitedWidgets;
			TraverseWidgetRecursive(WidgetBlueprint->WidgetTree->RootWidget, FString(), VisitedWidgets, Visitor);

			for (const TPair<FName, TObjectPtr<UWidget>>& NamedSlotBinding : WidgetBlueprint->WidgetTree->NamedSlotBindings)
			{
				TraverseWidgetRecursive(NamedSlotBinding.Value.Get(), FString(TEXT("NamedSlots")) / NamedSlotBinding.Key.ToString(), VisitedWidgets, Visitor);
			}
		}

		FString GetParentClassDisplayName(const UWidgetBlueprint* WidgetBlueprint)
		{
			return (WidgetBlueprint != nullptr && WidgetBlueprint->ParentClass != nullptr)
				? WidgetBlueprint->ParentClass->GetAuthoredName()
				: FString(TEXT("UserWidget"));
		}

		bool ClassChainContainsAuthoredName(const UClass* InClass, const FString& AuthoredName)
		{
			for (const UClass* CurrentClass = InClass; CurrentClass != nullptr; CurrentClass = CurrentClass->GetSuperClass())
			{
				if (CurrentClass->GetAuthoredName().Equals(AuthoredName, ESearchCase::CaseSensitive) ||
					CurrentClass->GetName().Equals(AuthoredName, ESearchCase::CaseSensitive) ||
					CurrentClass->GetName().Equals(FString(TEXT("U")) + AuthoredName, ESearchCase::CaseSensitive))
				{
					return true;
				}
			}

			return false;
		}

		FString BuildManifestJson(const UWidgetBlueprint* WidgetBlueprint)
		{
			TArray<FAnnotatedWidgetRecord> Records;
			ForEachWidgetWithPath(WidgetBlueprint, [&Records](UWidget* Widget, const FString& WidgetPath)
			{
				FAnnotatedWidgetRecord Record;
				Record.bBindWidget = GetBindWidget(Widget);
				GetWidgetMetadataValue(Widget, CallbackNameMetaDataKey, Record.CallbackName);
				GetWidgetMetadataValue(Widget, NotesMetaDataKey, Record.Notes);
				GetWidgetMetadataValue(Widget, SemanticRoleMetaDataKey, Record.SemanticRole);
				GetWidgetMetadataValue(Widget, ViewModelFieldMetaDataKey, Record.ViewModelField);
				Record.WidgetClass = Widget->GetClass()->GetAuthoredName();
				Record.WidgetName = Widget->GetName();
				Record.WidgetPath = WidgetPath;

				Records.Add(MoveTemp(Record));
			});

			Records.Sort([](const FAnnotatedWidgetRecord& Left, const FAnnotatedWidgetRecord& Right)
			{
				return Left.WidgetPath < Right.WidgetPath;
			});

			TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
			RootObject->SetStringField(TEXT("widgetBlueprint"), WidgetBlueprint != nullptr ? WidgetBlueprint->GetName() : FString());
			RootObject->SetStringField(TEXT("parentClass"), GetParentClassDisplayName(WidgetBlueprint));
			RootObject->SetStringField(TEXT("widgetKind"), GetWidgetKind(WidgetBlueprint));

			TSharedRef<FJsonObject> UIFrameworkObject = MakeShared<FJsonObject>();
			UIFrameworkObject->SetBoolField(TEXT("singleMainViewModel"), true);
			RootObject->SetObjectField(TEXT("uiFramework"), UIFrameworkObject);

			TArray<TSharedPtr<FJsonValue>> BindWidgetArray;
			TArray<TSharedPtr<FJsonValue>> CallbackArray;
			TArray<TSharedPtr<FJsonValue>> ViewModelFieldArray;
			TArray<TSharedPtr<FJsonValue>> SemanticRoleArray;
			TArray<TSharedPtr<FJsonValue>> AnnotatedWidgetArray;

			for (const FAnnotatedWidgetRecord& Record : Records)
			{
				if (Record.bBindWidget)
				{
					BindWidgetArray.Add(MakeShared<FJsonValueString>(Record.WidgetName));
				}

				if (!Record.CallbackName.IsEmpty())
				{
					TSharedRef<FJsonObject> CallbackObject = MakeShared<FJsonObject>();
					CallbackObject->SetStringField(TEXT("widget"), Record.WidgetName);
					CallbackObject->SetStringField(TEXT("function"), Record.CallbackName);
					CallbackObject->SetStringField(TEXT("widgetPath"), Record.WidgetPath);
					CallbackArray.Add(MakeShared<FJsonValueObject>(CallbackObject));
				}

				if (!Record.ViewModelField.IsEmpty())
				{
					TSharedRef<FJsonObject> ViewModelFieldObject = MakeShared<FJsonObject>();
					ViewModelFieldObject->SetStringField(TEXT("widget"), Record.WidgetName);
					ViewModelFieldObject->SetStringField(TEXT("field"), Record.ViewModelField);
					ViewModelFieldObject->SetStringField(TEXT("widgetPath"), Record.WidgetPath);
					ViewModelFieldArray.Add(MakeShared<FJsonValueObject>(ViewModelFieldObject));
				}

				if (!Record.SemanticRole.IsEmpty())
				{
					TSharedRef<FJsonObject> SemanticRoleObject = MakeShared<FJsonObject>();
					SemanticRoleObject->SetStringField(TEXT("widget"), Record.WidgetName);
					SemanticRoleObject->SetStringField(TEXT("role"), Record.SemanticRole);
					SemanticRoleObject->SetStringField(TEXT("widgetPath"), Record.WidgetPath);
					SemanticRoleArray.Add(MakeShared<FJsonValueObject>(SemanticRoleObject));
				}

				const bool bIsAnnotated = Record.bBindWidget ||
					!Record.CallbackName.IsEmpty() ||
					!Record.ViewModelField.IsEmpty() ||
					!Record.SemanticRole.IsEmpty() ||
					!Record.Notes.IsEmpty();

				if (bIsAnnotated)
				{
					TSharedRef<FJsonObject> WidgetObject = MakeShared<FJsonObject>();
					WidgetObject->SetStringField(TEXT("widget"), Record.WidgetName);
					WidgetObject->SetStringField(TEXT("widgetClass"), Record.WidgetClass);
					WidgetObject->SetStringField(TEXT("widgetPath"), Record.WidgetPath);
					WidgetObject->SetBoolField(TEXT("bindWidget"), Record.bBindWidget);

					if (!Record.CallbackName.IsEmpty())
					{
						WidgetObject->SetStringField(TEXT("callback"), Record.CallbackName);
					}

					if (!Record.ViewModelField.IsEmpty())
					{
						WidgetObject->SetStringField(TEXT("viewModelField"), Record.ViewModelField);
					}

					if (!Record.SemanticRole.IsEmpty())
					{
						WidgetObject->SetStringField(TEXT("semanticRole"), Record.SemanticRole);
					}

					if (!Record.Notes.IsEmpty())
					{
						WidgetObject->SetStringField(TEXT("notes"), Record.Notes);
					}

					AnnotatedWidgetArray.Add(MakeShared<FJsonValueObject>(WidgetObject));
				}
			}

			RootObject->SetArrayField(TEXT("bindWidgets"), BindWidgetArray);
			RootObject->SetArrayField(TEXT("callbacks"), CallbackArray);
			RootObject->SetArrayField(TEXT("viewModelFields"), ViewModelFieldArray);
			RootObject->SetArrayField(TEXT("semanticRoles"), SemanticRoleArray);
			RootObject->SetArrayField(TEXT("annotatedWidgets"), AnnotatedWidgetArray);

			FString ManifestJson;
			TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
				TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&ManifestJson);
			FJsonSerializer::Serialize(RootObject, Writer);
			return ManifestJson;
		}

		bool PromptForOutputPath(const UWidgetBlueprint* WidgetBlueprint, FString& OutOutputPath)
		{
			OutOutputPath.Reset();

			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
			if (DesktopPlatform == nullptr)
			{
				return false;
			}

			const FString DefaultPath = GetSuggestedManifestOutputPath(WidgetBlueprint);
			const FString DefaultDirectory = FPaths::GetPath(DefaultPath);
			const FString DefaultFileName = FPaths::GetCleanFilename(DefaultPath);

			IFileManager::Get().MakeDirectory(*DefaultDirectory, true);

			const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
			TArray<FString> SaveFileNames;
			const bool bOpenedDialog = DesktopPlatform->SaveFileDialog(
				ParentWindowHandle,
				TEXT("Export UI Manifest"),
				DefaultDirectory,
				DefaultFileName,
				TEXT("JSON Files (*.json)|*.json"),
				EFileDialogFlags::None,
				SaveFileNames);

			if (!bOpenedDialog || SaveFileNames.IsEmpty())
			{
				return false;
			}

			OutOutputPath = SaveFileNames[0];
			return true;
		}
	}

	bool GetWidgetMetadataValue(const UWidget* Widget, FName Key, FString& OutValue)
	{
		OutValue.Reset();

		UMetaData* MetaData = nullptr;
		if (!TryGetMetaData(Widget, MetaData))
		{
			return false;
		}

		const FString& Value = MetaData->GetValue(Widget, Key);
		if (Value.IsEmpty())
		{
			return false;
		}

		OutValue = Value;
		return true;
	}

	bool HasWidgetMetadataValue(const UWidget* Widget, FName Key)
	{
		UMetaData* MetaData = nullptr;
		return TryGetMetaData(Widget, MetaData) && MetaData->HasValue(Widget, Key);
	}

	void SetWidgetMetadataValue(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, FName Key, const FString& Value)
	{
		if (WidgetBlueprint == nullptr || Widget == nullptr)
		{
			return;
		}

		UMetaData* MetaData = nullptr;
		if (!TryGetMetaData(Widget, MetaData))
		{
			return;
		}

		WidgetBlueprint->Modify();
		if (WidgetBlueprint->WidgetTree != nullptr)
		{
			WidgetBlueprint->WidgetTree->Modify();
		}
		Widget->Modify();

		if (Value.TrimStartAndEnd().IsEmpty())
		{
			MetaData->RemoveValue(Widget, Key);
		}
		else
		{
			MetaData->SetValue(Widget, Key, *Value);
		}

		FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBlueprint);
	}

	bool GetBindWidget(const UWidget* Widget)
	{
		return Widget != nullptr && Widget->bIsVariable;
	}

	void SetBindWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, bool bBindWidget)
	{
		if (WidgetBlueprint == nullptr || Widget == nullptr || Widget->bIsVariable == bBindWidget)
		{
			return;
		}

		WidgetBlueprint->Modify();
		if (WidgetBlueprint->WidgetTree != nullptr)
		{
			WidgetBlueprint->WidgetTree->Modify();
		}
		Widget->Modify();
		Widget->bIsVariable = bBindWidget;

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	}

	FString GetWidgetKind(const UWidgetBlueprint* WidgetBlueprint)
	{
		if (WidgetBlueprint == nullptr || WidgetBlueprint->ParentClass == nullptr)
		{
			return TEXT("Widget");
		}

		if (ClassChainContainsAuthoredName(WidgetBlueprint->ParentClass, TEXT("MyMVVMScreenBase")))
		{
			return TEXT("Screen");
		}

		if (ClassChainContainsAuthoredName(WidgetBlueprint->ParentClass, TEXT("MyMVVMDialogBase")))
		{
			return TEXT("Dialog");
		}

		return TEXT("Widget");
	}

	FString GetSuggestedManifestOutputPath(const UWidgetBlueprint* WidgetBlueprint)
	{
		const FString SavedDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UIManifest"));
		const FString FileName = FString::Printf(
			TEXT("%s.ui.manifest.json"),
			WidgetBlueprint != nullptr ? *WidgetBlueprint->GetName() : TEXT("WidgetBlueprint"));
		return FPaths::Combine(SavedDirectory, FileName);
	}

	bool ExportManifestWithDialog(const TSharedPtr<FWidgetBlueprintEditor>& WidgetBlueprintEditor)
	{
		if (!WidgetBlueprintEditor.IsValid())
		{
			ShowNotification(FText::FromString(TEXT("UI manifest export failed: no active Widget Blueprint editor.")), SNotificationItem::CS_Fail);
			return false;
		}

		UWidgetBlueprint* WidgetBlueprint = WidgetBlueprintEditor->GetWidgetBlueprintObj();
		if (WidgetBlueprint == nullptr)
		{
			ShowNotification(FText::FromString(TEXT("UI manifest export failed: no active Widget Blueprint asset.")), SNotificationItem::CS_Fail);
			return false;
		}

		FString OutputPath;
		if (!PromptForOutputPath(WidgetBlueprint, OutputPath))
		{
			return false;
		}

		IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);

		const FString ManifestJson = BuildManifestJson(WidgetBlueprint);
		if (!FFileHelper::SaveStringToFile(ManifestJson, *OutputPath))
		{
			ShowNotification(
				FText::FromString(FString::Printf(TEXT("Failed to write UI manifest: %s"), *OutputPath)),
				SNotificationItem::CS_Fail);
			return false;
		}

		ShowNotification(
			FText::FromString(FString::Printf(TEXT("Exported UI manifest: %s"), *OutputPath)),
			SNotificationItem::CS_Success);
		return true;
	}
}
