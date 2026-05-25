#include "JsonWidgetBlueprintImporter.h"
#include "JsonWidgetBlueprintSemantics.h"

#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/ListViewBase.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/ContentWidget.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Factories/DataTableFactory.h"
#include "INotifyFieldValueChanged.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Layout/Margin.h"
#include "Materials/MaterialInterface.h"
#include "MVVMBlueprintView.h"
#include "MVVMBlueprintViewBinding.h"
#include "MVVMBlueprintViewModelContext.h"
#include "MVVMEditorSubsystem.h"
#include "MVVMPropertyPath.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonSerializer.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Types/MVVMBindingMode.h"
#include "Types/MVVMFieldVariant.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"
#include "UObject/TopLevelAssetPath.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectIterator.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

DEFINE_LOG_CATEGORY_STATIC(LogJsonWidgetBlueprintImporter, Log, All);

namespace JsonWidgetBlueprintImporter
{
	struct FImportedWidgetNode
	{
		FString Type;
		FString Name;
		TSharedPtr<FJsonObject> SlotObject;
		TSharedPtr<FJsonObject> PropsObject;
		TSharedPtr<FJsonObject> SemanticObject;
		TSharedPtr<FJsonObject> CodegenObject;
		TSharedPtr<FJsonObject> MVVMObject;
		TOptional<FString> SemanticRole;
		TOptional<bool> BindWidget;
		TArray<FImportedWidgetNode> Children;
	};

	struct FImportedViewModelDefinition
	{
		FString Name;
		FString ClassName;
		FString CreationType;
	};

	struct FImportedMVVMBindingDefinition
	{
		FString WidgetName;
		FString Destination;
		FString SourceViewModel;
		FString SourceField;
	};

	struct FImportedMVVMSchema
	{
		TArray<FImportedViewModelDefinition> ViewModels;
	};

	struct FImportedSchemaDefaults
	{
		FString FontObjectPath;
		FString RichTextStyleSet;
		TWeakObjectPtr<UDataTable> GeneratedRichTextStyleSet;
	};

	struct FImportedSchema
	{
		FString Version;
		FString WidgetBlueprintName;
		FString ParentClass;
		FImportedSchemaDefaults Defaults;
		FImportedMVVMSchema MVVM;
		FImportedWidgetNode Root;
	};

	static FString SanitizeName(const FString& InName)
	{
		FString Result = InName.TrimStartAndEnd();
		for (TCHAR& Char : Result)
		{
			if (!(FChar::IsAlnum(Char) || Char == TEXT('_')))
			{
				Char = TEXT('_');
			}
		}

		if (Result.IsEmpty())
		{
			Result = TEXT("WBP_ImportedJson");
		}

		return Result;
	}

	static FString MakeNodePath(const FString& ParentPath, const FString& NodeName, const FString& NodeType)
	{
		const FString Segment = NodeName.IsEmpty() ? NodeType : NodeName;
		return ParentPath.IsEmpty() ? Segment : ParentPath / Segment;
	}

	static void AddWarning(FJsonWidgetBlueprintImportResult& Result, const FString& Message)
	{
		Result.Warnings.Add(Message);
		UE_LOG(LogJsonWidgetBlueprintImporter, Warning, TEXT("%s"), *Message);
	}

	static void AddError(FJsonWidgetBlueprintImportResult& Result, const FString& Message)
	{
		Result.Errors.Add(Message);
		UE_LOG(LogJsonWidgetBlueprintImporter, Error, TEXT("%s"), *Message);
	}

	static bool TryGetObjectField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, TSharedPtr<FJsonObject>& OutObject)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		const TSharedPtr<FJsonObject>* FoundObject = nullptr;
		if (Object->TryGetObjectField(FieldName, FoundObject) && FoundObject != nullptr)
		{
			OutObject = *FoundObject;
			return OutObject.IsValid();
		}

		return false;
	}

	static bool TryGetArrayField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, const TArray<TSharedPtr<FJsonValue>>*& OutArray)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		return Object->TryGetArrayField(FieldName, OutArray);
	}

	static bool TryReadFloatArray(const TArray<TSharedPtr<FJsonValue>>& Values, TArray<float>& OutValues)
	{
		OutValues.Reset();
		for (const TSharedPtr<FJsonValue>& Value : Values)
		{
			double Number = 0.0;
			if (!Value.IsValid() || !Value->TryGetNumber(Number))
			{
				return false;
			}

			OutValues.Add(static_cast<float>(Number));
		}

		return true;
	}

	static bool TryReadVector2D(const TSharedPtr<FJsonValue>& Value, FVector2D& OutVector)
	{
		if (!Value.IsValid())
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Value->TryGetArray(Array) || Array == nullptr || Array->Num() != 2)
		{
			return false;
		}

		TArray<float> Numbers;
		if (!TryReadFloatArray(*Array, Numbers) || Numbers.Num() != 2)
		{
			return false;
		}

		OutVector = FVector2D(Numbers[0], Numbers[1]);
		return true;
	}

	static bool TryReadLinearColor(const TSharedPtr<FJsonValue>& Value, FLinearColor& OutColor)
	{
		if (!Value.IsValid())
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Value->TryGetArray(Array) || Array == nullptr || (Array->Num() != 3 && Array->Num() != 4))
		{
			return false;
		}

		TArray<float> Numbers;
		if (!TryReadFloatArray(*Array, Numbers))
		{
			return false;
		}

		OutColor = FLinearColor(
			Numbers[0],
			Numbers[1],
			Numbers[2],
			Numbers.Num() == 4 ? Numbers[3] : 1.0f);
		return true;
	}

	static bool TryReadMargin(const TSharedPtr<FJsonValue>& Value, FMargin& OutMargin)
	{
		if (!Value.IsValid())
		{
			return false;
		}

		double Scalar = 0.0;
		if (Value->TryGetNumber(Scalar))
		{
			OutMargin = FMargin(static_cast<float>(Scalar));
			return true;
		}

		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Value->TryGetArray(Array) || Array == nullptr)
		{
			return false;
		}

		TArray<float> Numbers;
		if (!TryReadFloatArray(*Array, Numbers))
		{
			return false;
		}

		if (Numbers.Num() == 2)
		{
			OutMargin = FMargin(Numbers[0], Numbers[1]);
			return true;
		}

		if (Numbers.Num() == 4)
		{
			OutMargin = FMargin(Numbers[0], Numbers[1], Numbers[2], Numbers[3]);
			return true;
		}

		return false;
	}

	static bool TryReadAnchors(const TSharedPtr<FJsonValue>& Value, FAnchors& OutAnchors)
	{
		if (!Value.IsValid())
		{
			return false;
		}

		FString AnchorKeyword;
		if (Value->TryGetString(AnchorKeyword))
		{
			if (AnchorKeyword.Equals(TEXT("full"), ESearchCase::IgnoreCase))
			{
				OutAnchors = FAnchors(0.0f, 0.0f, 1.0f, 1.0f);
				return true;
			}

			if (AnchorKeyword.Equals(TEXT("center"), ESearchCase::IgnoreCase))
			{
				OutAnchors = FAnchors(0.5f, 0.5f, 0.5f, 0.5f);
				return true;
			}

			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Value->TryGetArray(Array) || Array == nullptr || Array->Num() != 4)
		{
			return false;
		}

		TArray<float> Numbers;
		if (!TryReadFloatArray(*Array, Numbers) || Numbers.Num() != 4)
		{
			return false;
		}

		OutAnchors = FAnchors(Numbers[0], Numbers[1], Numbers[2], Numbers[3]);
		return true;
	}

	static EHorizontalAlignment ToHorizontalAlignment(float Value)
	{
		if (Value <= 0.25f)
		{
			return HAlign_Left;
		}

		if (Value >= 0.75f)
		{
			return HAlign_Right;
		}

		return HAlign_Center;
	}

	static EVerticalAlignment ToVerticalAlignment(float Value)
	{
		if (Value <= 0.25f)
		{
			return VAlign_Top;
		}

		if (Value >= 0.75f)
		{
			return VAlign_Bottom;
		}

		return VAlign_Center;
	}

	static bool TryReadHorizontalAlignmentKeyword(const TSharedPtr<FJsonValue>& Value, EHorizontalAlignment& OutAlignment)
	{
		FString AlignmentText;
		if (!Value.IsValid() || !Value->TryGetString(AlignmentText))
		{
			return false;
		}

		if (AlignmentText.Equals(TEXT("fill"), ESearchCase::IgnoreCase))
		{
			OutAlignment = HAlign_Fill;
			return true;
		}
		if (AlignmentText.Equals(TEXT("left"), ESearchCase::IgnoreCase))
		{
			OutAlignment = HAlign_Left;
			return true;
		}
		if (AlignmentText.Equals(TEXT("center"), ESearchCase::IgnoreCase))
		{
			OutAlignment = HAlign_Center;
			return true;
		}
		if (AlignmentText.Equals(TEXT("right"), ESearchCase::IgnoreCase))
		{
			OutAlignment = HAlign_Right;
			return true;
		}

		return false;
	}

	static bool TryReadVerticalAlignmentKeyword(const TSharedPtr<FJsonValue>& Value, EVerticalAlignment& OutAlignment)
	{
		FString AlignmentText;
		if (!Value.IsValid() || !Value->TryGetString(AlignmentText))
		{
			return false;
		}

		if (AlignmentText.Equals(TEXT("fill"), ESearchCase::IgnoreCase))
		{
			OutAlignment = VAlign_Fill;
			return true;
		}
		if (AlignmentText.Equals(TEXT("top"), ESearchCase::IgnoreCase))
		{
			OutAlignment = VAlign_Top;
			return true;
		}
		if (AlignmentText.Equals(TEXT("center"), ESearchCase::IgnoreCase))
		{
			OutAlignment = VAlign_Center;
			return true;
		}
		if (AlignmentText.Equals(TEXT("bottom"), ESearchCase::IgnoreCase))
		{
			OutAlignment = VAlign_Bottom;
			return true;
		}

		return false;
	}

	static bool ParseNode(const TSharedPtr<FJsonObject>& NodeObject, FImportedWidgetNode& OutNode, FJsonWidgetBlueprintImportResult& Result, const FString& ParentPath)
	{
		if (!NodeObject.IsValid())
		{
			AddError(Result, TEXT("Node is not a valid JSON object."));
			return false;
		}

		if (!NodeObject->TryGetStringField(TEXT("type"), OutNode.Type) || OutNode.Type.IsEmpty())
		{
			AddError(Result, FString::Printf(TEXT("Node at '%s' is missing a valid 'type'."), *ParentPath));
			return false;
		}

		NodeObject->TryGetStringField(TEXT("name"), OutNode.Name);
		TryGetObjectField(NodeObject, TEXT("slot"), OutNode.SlotObject);
		TryGetObjectField(NodeObject, TEXT("props"), OutNode.PropsObject);
		TryGetObjectField(NodeObject, TEXT("semantic"), OutNode.SemanticObject);
		TryGetObjectField(NodeObject, TEXT("codegen"), OutNode.CodegenObject);
		if (OutNode.CodegenObject.IsValid())
		{
			TryGetObjectField(OutNode.CodegenObject, TEXT("mvvm"), OutNode.MVVMObject);
		}

		if (OutNode.SemanticObject.IsValid())
		{
			FString SemanticRole;
			if (OutNode.SemanticObject->TryGetStringField(TEXT("role"), SemanticRole) && !SemanticRole.IsEmpty())
			{
				OutNode.SemanticRole = SemanticRole;
			}
		}

		if (OutNode.CodegenObject.IsValid() && OutNode.CodegenObject->HasField(TEXT("bindWidget")))
		{
			bool bBindWidget = false;
			if (OutNode.CodegenObject->TryGetBoolField(TEXT("bindWidget"), bBindWidget))
			{
				OutNode.BindWidget = bBindWidget;
			}
			else
			{
				AddWarning(Result, FString::Printf(TEXT("Node '%s' has non-bool codegen.bindWidget; ignoring it."), *MakeNodePath(ParentPath, OutNode.Name, OutNode.Type)));
			}
		}

		const FString NodePath = MakeNodePath(ParentPath, OutNode.Name, OutNode.Type);

		const TArray<TSharedPtr<FJsonValue>>* ChildrenArray = nullptr;
		if (TryGetArrayField(NodeObject, TEXT("children"), ChildrenArray) && ChildrenArray != nullptr)
		{
			for (const TSharedPtr<FJsonValue>& ChildValue : *ChildrenArray)
			{
				const TSharedPtr<FJsonObject>* ChildObjectPtr = nullptr;
				if (!ChildValue.IsValid() || !ChildValue->TryGetObject(ChildObjectPtr) || ChildObjectPtr == nullptr || !ChildObjectPtr->IsValid())
				{
					AddWarning(Result, FString::Printf(TEXT("Skipping non-object child under '%s'."), *NodePath));
					continue;
				}

				TSharedPtr<FJsonObject> ChildObject = *ChildObjectPtr;
				FImportedWidgetNode& ChildNode = OutNode.Children.AddDefaulted_GetRef();
				if (!ParseNode(ChildObject, ChildNode, Result, NodePath))
				{
					return false;
				}
			}
		}

		return true;
	}

	static bool ParseSchema(const FString& JsonText, FImportedSchema& OutSchema, FJsonWidgetBlueprintImportResult& Result)
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			AddError(Result, TEXT("Failed to deserialize JSON schema."));
			return false;
		}

		RootObject->TryGetStringField(TEXT("version"), OutSchema.Version);
		TSharedPtr<FJsonObject> DefaultsObject;
		if (TryGetObjectField(RootObject, TEXT("defaults"), DefaultsObject))
		{
			DefaultsObject->TryGetStringField(TEXT("fontObject"), OutSchema.Defaults.FontObjectPath);
			DefaultsObject->TryGetStringField(TEXT("richTextStyleSet"), OutSchema.Defaults.RichTextStyleSet);
		}

		if (!RootObject->TryGetStringField(TEXT("widgetBlueprintName"), OutSchema.WidgetBlueprintName) || OutSchema.WidgetBlueprintName.IsEmpty())
		{
			AddError(Result, TEXT("Schema is missing 'widgetBlueprintName'."));
			return false;
		}

		RootObject->TryGetStringField(TEXT("parentClass"), OutSchema.ParentClass);
		if (OutSchema.ParentClass.IsEmpty())
		{
			AddWarning(Result, TEXT("Schema does not specify 'parentClass'; falling back to UserWidget."));
		}

		TSharedPtr<FJsonObject> MVVMObject;
		if (TryGetObjectField(RootObject, TEXT("mvvm"), MVVMObject))
		{
			const TArray<TSharedPtr<FJsonValue>>* ViewModelsArray = nullptr;
			if (TryGetArrayField(MVVMObject, TEXT("viewModels"), ViewModelsArray) && ViewModelsArray != nullptr)
			{
				for (const TSharedPtr<FJsonValue>& ViewModelValue : *ViewModelsArray)
				{
					const TSharedPtr<FJsonObject>* ViewModelObjectPtr = nullptr;
					if (!ViewModelValue.IsValid() || !ViewModelValue->TryGetObject(ViewModelObjectPtr) || ViewModelObjectPtr == nullptr || !ViewModelObjectPtr->IsValid())
					{
						AddWarning(Result, TEXT("Skipping non-object mvvm.viewModels entry."));
						continue;
					}

					FImportedViewModelDefinition& ViewModelDefinition = OutSchema.MVVM.ViewModels.AddDefaulted_GetRef();
					(*ViewModelObjectPtr)->TryGetStringField(TEXT("name"), ViewModelDefinition.Name);
					(*ViewModelObjectPtr)->TryGetStringField(TEXT("class"), ViewModelDefinition.ClassName);
					(*ViewModelObjectPtr)->TryGetStringField(TEXT("creationType"), ViewModelDefinition.CreationType);
					if (ViewModelDefinition.Name.IsEmpty() || ViewModelDefinition.ClassName.IsEmpty())
					{
						AddWarning(Result, TEXT("Skipping mvvm.viewModels entry because it is missing 'name' or 'class'."));
						OutSchema.MVVM.ViewModels.Pop(EAllowShrinking::No);
					}
				}
			}
		}

		TSharedPtr<FJsonObject> RootNodeObject;
		if (!TryGetObjectField(RootObject, TEXT("root"), RootNodeObject))
		{
			AddError(Result, TEXT("Schema is missing a valid 'root' object."));
			return false;
		}

		return ParseNode(RootNodeObject, OutSchema.Root, Result, TEXT("root"));
	}

	static FSlateChildSize MakeSlateChildSize(const FString& SizeRuleText, double FillValue)
	{
		FSlateChildSize Size;
		if (SizeRuleText.Equals(TEXT("fill"), ESearchCase::IgnoreCase))
		{
			Size.SizeRule = ESlateSizeRule::Fill;
			Size.Value = static_cast<float>(FillValue);
		}
		else
		{
			Size.SizeRule = ESlateSizeRule::Automatic;
			Size.Value = 1.0f;
		}

		return Size;
	}

	static FName ResolveWidgetObjectName(UWidgetTree* WidgetTree, UClass* WidgetClass, const FString& RequestedName, const FString& NodeType, FJsonWidgetBlueprintImportResult& Result)
	{
		const FString RawRequestedName = RequestedName.IsEmpty() ? NodeType : RequestedName;
		const FString SanitizedRequestedName = SanitizeName(RawRequestedName);
		FName FinalName(*SanitizedRequestedName);

		if (!RawRequestedName.Equals(SanitizedRequestedName, ESearchCase::CaseSensitive))
		{
			AddWarning(Result, FString::Printf(
				TEXT("Widget name '%s' was sanitized to '%s'."),
				*RawRequestedName,
				*SanitizedRequestedName));
		}

		if (FindObject<UObject>(WidgetTree, *SanitizedRequestedName) == nullptr)
		{
			return FinalName;
		}

		const FName UniqueName = MakeUniqueObjectName(WidgetTree, WidgetClass, FinalName);
		AddWarning(Result, FString::Printf(
			TEXT("Widget name conflict: requested '%s', renamed to '%s'."),
			*SanitizedRequestedName,
			*UniqueName.ToString()));
		return UniqueName;
	}

	static UWidget* CreateWidgetByType(UWidgetTree* WidgetTree, const FString& TypeName, const FString& RequestedName, FJsonWidgetBlueprintImportResult& Result)
	{
		if (WidgetTree == nullptr)
		{
			return nullptr;
		}

		UClass* WidgetClass = nullptr;
		if (TypeName.Equals(TEXT("CanvasPanel"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UCanvasPanel::StaticClass();
		}
		else if (TypeName.Equals(TEXT("Border"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UBorder::StaticClass();
		}
		else if (TypeName.Equals(TEXT("Overlay"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UOverlay::StaticClass();
		}
		else if (TypeName.Equals(TEXT("VerticalBox"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UVerticalBox::StaticClass();
		}
		else if (TypeName.Equals(TEXT("HorizontalBox"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UHorizontalBox::StaticClass();
		}
		else if (TypeName.Equals(TEXT("SizeBox"), ESearchCase::IgnoreCase))
		{
			WidgetClass = USizeBox::StaticClass();
		}
		else if (TypeName.Equals(TEXT("Spacer"), ESearchCase::IgnoreCase))
		{
			WidgetClass = USpacer::StaticClass();
		}
		else if (TypeName.Equals(TEXT("TextBlock"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UTextBlock::StaticClass();
		}
		else if (TypeName.Equals(TEXT("CommonTextBlock"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UCommonTextBlock::StaticClass();
		}
		else if (TypeName.Equals(TEXT("RichTextBlock"), ESearchCase::IgnoreCase))
		{
			WidgetClass = URichTextBlock::StaticClass();
		}
		else if (TypeName.Equals(TEXT("Image"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UImage::StaticClass();
		}
		else if (TypeName.Equals(TEXT("Button"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UButton::StaticClass();
		}
		else if (TypeName.Equals(TEXT("ProgressBar"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UProgressBar::StaticClass();
		}
		else if (TypeName.Equals(TEXT("ScrollBox"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UScrollBox::StaticClass();
		}
		else if (TypeName.Equals(TEXT("ListView"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UListView::StaticClass();
		}
		else if (TypeName.Equals(TEXT("TileView"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UTileView::StaticClass();
		}
		else if (TypeName.Equals(TEXT("UniformGridPanel"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UUniformGridPanel::StaticClass();
		}
		else if (TypeName.Equals(TEXT("WrapBox"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UWrapBox::StaticClass();
		}
		else if (TypeName.Equals(TEXT("WidgetSwitcher"), ESearchCase::IgnoreCase))
		{
			WidgetClass = UWidgetSwitcher::StaticClass();
		}

		if (WidgetClass == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Unsupported widget type '%s'; node will be skipped."), *TypeName));
			return nullptr;
		}

		const FName ResolvedName = ResolveWidgetObjectName(WidgetTree, WidgetClass, RequestedName, TypeName, Result);
		return WidgetTree->ConstructWidget<UWidget>(WidgetClass, ResolvedName);
	}

	static UClass* ResolveEntryWidgetClass(const FString& EntryWidgetClassText)
	{
		if (EntryWidgetClassText.IsEmpty())
		{
			return nullptr;
		}

		if (UClass* LoadedClass = FindObject<UClass>(nullptr, *EntryWidgetClassText))
		{
			if (LoadedClass->IsChildOf(UUserWidget::StaticClass()))
			{
				return LoadedClass;
			}
		}

		if (UClass* LoadedClass = LoadClass<UUserWidget>(nullptr, *EntryWidgetClassText))
		{
			return LoadedClass;
		}

		const FString TrimmedName = EntryWidgetClassText.TrimStartAndEnd();
		const FString ExpectedRawName = TrimmedName.StartsWith(TEXT("U")) ? TrimmedName : FString(TEXT("U")) + TrimmedName;
		const FString ExpectedGeneratedName = TrimmedName.EndsWith(TEXT("_C")) ? TrimmedName : TrimmedName + TEXT("_C");
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;
			if (Class == nullptr || !Class->IsChildOf(UUserWidget::StaticClass()))
			{
				continue;
			}

			if (Class->GetName().Equals(TrimmedName) ||
				Class->GetName().Equals(ExpectedRawName) ||
				Class->GetName().Equals(ExpectedGeneratedName) ||
				Class->GetAuthoredName().Equals(TrimmedName))
			{
				return Class;
			}
		}

		return nullptr;
	}

	static bool SetListEntryWidgetClass(UListViewBase* ListViewBase, UClass* EntryClass, FJsonWidgetBlueprintImportResult& Result, const FString& NodePath)
	{
		if (ListViewBase == nullptr || EntryClass == nullptr)
		{
			return false;
		}

		if (!IsValid(ListViewBase))
		{
			AddWarning(Result, FString::Printf(TEXT("ListViewBase for node '%s' is no longer valid when applying EntryWidgetClass."), *NodePath));
			return false;
		}

		if (!EntryClass->ImplementsInterface(UUserObjectListEntry::StaticClass()))
		{
			AddWarning(Result, FString::Printf(
				TEXT("Entry widget class '%s' on node '%s' does not implement UserObjectListEntry."),
				*EntryClass->GetName(),
				*NodePath));
			return false;
		}

		FClassProperty* EntryWidgetClassProperty = FindFProperty<FClassProperty>(UListViewBase::StaticClass(), TEXT("EntryWidgetClass"));
		if (EntryWidgetClassProperty == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Could not find UListViewBase.EntryWidgetClass for node '%s'."), *NodePath));
			return false;
		}

		ListViewBase->Modify();
		EntryWidgetClassProperty->SetObjectPropertyValue_InContainer(ListViewBase, EntryClass);
		return true;
	}

	static UClass* CreateEntryWidgetBlueprint(UWidgetBlueprint* OwnerBlueprint, UListViewBase* ListViewBase, UClass* EntryParentClass, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (OwnerBlueprint == nullptr || ListViewBase == nullptr)
		{
			return nullptr;
		}

		if (EntryParentClass == nullptr || !EntryParentClass->IsChildOf(UUserWidget::StaticClass()))
		{
			EntryParentClass = UUserWidget::StaticClass();
		}

		const FString FolderPath = FPackageName::GetLongPackagePath(OwnerBlueprint->GetOutermost()->GetName());
		const bool bPlainPlaceholder = EntryParentClass == UUserWidget::StaticClass();
		const FString ParentSuffix = bPlainPlaceholder ? TEXT("Entry") : FString::Printf(TEXT("%s_Entry"), *EntryParentClass->GetAuthoredName());
		const FString AssetName = SanitizeName(FString::Printf(TEXT("%s_%s_%s"), *OwnerBlueprint->GetName(), *ListViewBase->GetName(), *ParentSuffix));
		const FString PackageName = FolderPath / AssetName;
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);

		if (FPackageName::DoesPackageExist(PackageName))
		{
			if (UWidgetBlueprint* ExistingBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath))
			{
				if (ExistingBlueprint->GeneratedClass != nullptr && ExistingBlueprint->GeneratedClass->IsChildOf(EntryParentClass))
				{
					AddWarning(Result, FString::Printf(
						TEXT("Reusing entry widget blueprint '%s' for node '%s'."),
						*ObjectPath,
						*NodePath));
					return ExistingBlueprint->GeneratedClass;
				}
			}
		}

		UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
		Factory->BlueprintType = BPTYPE_Normal;
		Factory->ParentClass = EntryParentClass;

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UObject* CreatedAsset = AssetTools.CreateAsset(AssetName, FolderPath, UWidgetBlueprint::StaticClass(), Factory);
		UWidgetBlueprint* EntryBlueprint = Cast<UWidgetBlueprint>(CreatedAsset);
		if (EntryBlueprint == nullptr || EntryBlueprint->WidgetTree == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Failed to create placeholder entry widget for node '%s'."), *NodePath));
			return nullptr;
		}

		AddWarning(Result, FString::Printf(
			TEXT("Generated entry widget blueprint '%s' for node '%s'."),
			*ObjectPath,
			*NodePath));

		EntryBlueprint->Modify();
		EntryBlueprint->WidgetTree->Modify();

		if (!EntryParentClass->ImplementsInterface(UUserObjectListEntry::StaticClass()) &&
			!FBlueprintEditorUtils::ImplementNewInterface(EntryBlueprint, UUserObjectListEntry::StaticClass()->GetClassPathName()))
		{
			AddWarning(Result, FString::Printf(TEXT("Failed to add UserObjectListEntry interface to entry widget for node '%s'."), *NodePath));
			return nullptr;
		}

		if (bPlainPlaceholder)
		{
			UTextBlock* PlaceholderText = EntryBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(),
				MakeUniqueObjectName(EntryBlueprint->WidgetTree, UTextBlock::StaticClass(), TEXT("PlaceholderText")));
			if (PlaceholderText != nullptr)
			{
				PlaceholderText->SetText(FText::FromString(TEXT("List Entry Placeholder")));
				EntryBlueprint->WidgetTree->RootWidget = PlaceholderText;
			}
		}

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(EntryBlueprint);
		FKismetEditorUtilities::CompileBlueprint(EntryBlueprint);

		if (EntryBlueprint->GeneratedClass == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Entry widget blueprint for node '%s' did not produce a generated class."), *NodePath));
			return nullptr;
		}

		return EntryBlueprint->GeneratedClass;
	}

	static void EnsureListEntryWidgetClass(UWidgetBlueprint* OwnerBlueprint, UWidget* Widget, const FImportedWidgetNode& Node, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		UListViewBase* ListViewBase = Cast<UListViewBase>(Widget);
		if (ListViewBase == nullptr)
		{
			return;
		}

		FString EntryWidgetClassText;
		bool bGeneratePreviewEntryWidget = true;
		if (Node.PropsObject.IsValid())
		{
			Node.PropsObject->TryGetStringField(TEXT("entryWidgetClass"), EntryWidgetClassText);
			Node.PropsObject->TryGetBoolField(TEXT("generatePreviewEntryWidget"), bGeneratePreviewEntryWidget);
		}

		UClass* EntryWidgetClass = ResolveEntryWidgetClass(EntryWidgetClassText);
		if (EntryWidgetClass == nullptr)
		{
			if (!EntryWidgetClassText.IsEmpty())
			{
				AddWarning(Result, FString::Printf(
					TEXT("Node '%s' specifies entryWidgetClass '%s' but it could not be resolved. Falling back to a placeholder entry widget."),
					*NodePath,
					*EntryWidgetClassText));
			}

			if (!bGeneratePreviewEntryWidget)
			{
				AddWarning(Result, FString::Printf(
					TEXT("Node '%s' disables generatePreviewEntryWidget, but UE5.5 ListViewBase still requires EntryWidgetClass to compile. Falling back to a placeholder entry widget."),
					*NodePath));
			}

			EntryWidgetClass = CreateEntryWidgetBlueprint(OwnerBlueprint, ListViewBase, UUserWidget::StaticClass(), NodePath, Result);
		}
		else if (Cast<UWidgetBlueprintGeneratedClass>(EntryWidgetClass) == nullptr)
		{
			EntryWidgetClass = CreateEntryWidgetBlueprint(OwnerBlueprint, ListViewBase, EntryWidgetClass, NodePath, Result);
		}

		if (EntryWidgetClass != nullptr)
		{
			SetListEntryWidgetClass(ListViewBase, EntryWidgetClass, Result, NodePath);
		}
	}

	static void ApplyBindWidgetSetting(UWidget* Widget, const FImportedWidgetNode& Node, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (Widget == nullptr || !Node.BindWidget.IsSet())
		{
			return;
		}

		Widget->Modify();
		Widget->bIsVariable = Node.BindWidget.GetValue();

		if (Widget->GetFName().IsNone())
		{
			AddWarning(Result, FString::Printf(TEXT("Node '%s' requested bindWidget but has no stable widget name."), *NodePath));
		}
	}

	static void ApplySemanticMetadata(UWidget* Widget, const FImportedWidgetNode& Node)
	{
		if (Widget == nullptr || !Node.SemanticRole.IsSet())
		{
			return;
		}

		if (UPackage* Package = Widget->GetOutermost())
		{
			if (UMetaData* MetaData = Package->GetMetaData())
			{
				MetaData->SetValue(Widget, JsonWidgetBlueprintSemantics::SemanticRoleMetaDataKey, *Node.SemanticRole.GetValue());
			}
		}
	}

	static UObject* ResolveFontObject(const FString& FontObjectPath, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (FontObjectPath.IsEmpty())
		{
			return nullptr;
		}

		UObject* FontObject = LoadObject<UObject>(nullptr, *FontObjectPath);
		if (FontObject == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Could not load font object '%s' for node '%s'."), *FontObjectPath, *NodePath));
		}

		return FontObject;
	}

	static bool ApplyFontObjectToTextWidget(UWidget* Widget, UObject* FontObject)
	{
		if (Widget == nullptr || FontObject == nullptr)
		{
			return false;
		}

		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.FontObject = FontObject;
			TextBlock->SetFont(FontInfo);
			return true;
		}

		if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
		{
			FTextBlockStyle TextStyle = RichTextBlock->GetDefaultTextStyle();
			FSlateFontInfo FontInfo = TextStyle.Font;
			FontInfo.FontObject = FontObject;
			TextStyle.SetFont(FontInfo);
			RichTextBlock->SetDefaultTextStyle(TextStyle);
			RichTextBlock->SetDefaultFont(FontInfo);
			return true;
		}

		return false;
	}

	static void ApplyFontObjectToTextStyle(FTextBlockStyle& TextStyle, UObject* FontObject, int32 FontSize, const FLinearColor& Color)
	{
		FSlateFontInfo FontInfo = TextStyle.Font;
		if (FontObject != nullptr)
		{
			FontInfo.FontObject = FontObject;
		}
		FontInfo.Size = FontSize;
		TextStyle.SetFont(FontInfo);
		TextStyle.SetColorAndOpacity(FSlateColor(Color));
	}

	static UDataTable* CreateRichTextStyleSetDataTable(UWidgetBlueprint* OwnerBlueprint, const FImportedSchema& Schema, FJsonWidgetBlueprintImportResult& Result)
	{
		if (OwnerBlueprint == nullptr)
		{
			return nullptr;
		}

		if (!Schema.Defaults.RichTextStyleSet.Equals(TEXT("auto"), ESearchCase::IgnoreCase))
		{
			if (!Schema.Defaults.RichTextStyleSet.IsEmpty())
			{
				if (UDataTable* ExistingStyleSet = LoadObject<UDataTable>(nullptr, *Schema.Defaults.RichTextStyleSet))
				{
					return ExistingStyleSet;
				}

				AddWarning(Result, FString::Printf(TEXT("Could not load rich text style set '%s'. RichTextBlock tags may render as literal text."), *Schema.Defaults.RichTextStyleSet));
			}

			return nullptr;
		}

		const FString FolderPath = FPackageName::GetLongPackagePath(OwnerBlueprint->GetOutermost()->GetName());
		const FString AssetName = SanitizeName(FString::Printf(TEXT("%s_RichTextStyleSet"), *OwnerBlueprint->GetName()));
		const FString PackageName = FolderPath / AssetName;
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);

		if (FPackageName::DoesPackageExist(PackageName))
		{
			if (UDataTable* ExistingStyleSet = LoadObject<UDataTable>(nullptr, *ObjectPath))
			{
				if (ExistingStyleSet->GetRowStruct() != nullptr && ExistingStyleSet->GetRowStruct()->IsChildOf(FRichTextStyleRow::StaticStruct()))
				{
					AddWarning(Result, FString::Printf(TEXT("Reusing rich text style set '%s'."), *ObjectPath));
					return ExistingStyleSet;
				}
			}
		}

		UDataTableFactory* Factory = NewObject<UDataTableFactory>();
		Factory->Struct = FRichTextStyleRow::StaticStruct();

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UDataTable* StyleSet = Cast<UDataTable>(AssetTools.CreateAsset(AssetName, FolderPath, UDataTable::StaticClass(), Factory));
		if (StyleSet == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Failed to create rich text style set '%s'."), *ObjectPath));
			return nullptr;
		}

		UObject* FontObject = ResolveFontObject(Schema.Defaults.FontObjectPath, TEXT("defaults.richTextStyleSet"), Result);

		FRichTextStyleRow DefaultRow;
		ApplyFontObjectToTextStyle(DefaultRow.TextStyle, FontObject, 17, FLinearColor(0.76f, 0.78f, 0.76f, 1.0f));
		StyleSet->AddRow(TEXT("Default"), DefaultRow);

		FRichTextStyleRow TitleRow;
		ApplyFontObjectToTextStyle(TitleRow.TextStyle, FontObject, 22, FLinearColor(0.94f, 0.92f, 0.86f, 1.0f));
		StyleSet->AddRow(TEXT("Title"), TitleRow);

		FRichTextStyleRow EmphasisRow;
		ApplyFontObjectToTextStyle(EmphasisRow.TextStyle, FontObject, 17, FLinearColor(0.92f, 0.78f, 0.42f, 1.0f));
		StyleSet->AddRow(TEXT("Emphasis"), EmphasisRow);

		StyleSet->MarkPackageDirty();
		AddWarning(Result, FString::Printf(TEXT("Generated rich text style set '%s'."), *ObjectPath));
		return StyleSet;
	}

	static UDataTable* ResolveRichTextStyleSet(const FImportedSchemaDefaults& Defaults, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (Defaults.GeneratedRichTextStyleSet.IsValid())
		{
			return Defaults.GeneratedRichTextStyleSet.Get();
		}

		if (!Defaults.RichTextStyleSet.IsEmpty() && !Defaults.RichTextStyleSet.Equals(TEXT("auto"), ESearchCase::IgnoreCase))
		{
			if (UDataTable* StyleSet = LoadObject<UDataTable>(nullptr, *Defaults.RichTextStyleSet))
			{
				return StyleSet;
			}

			AddWarning(Result, FString::Printf(TEXT("Could not load rich text style set '%s' for node '%s'."), *Defaults.RichTextStyleSet, *NodePath));
		}

		return nullptr;
	}

	static void ApplyWidgetProps(UWidget* Widget, const FImportedWidgetNode& Node, const FString& NodePath, const FImportedSchemaDefaults& Defaults, FJsonWidgetBlueprintImportResult& Result, FString& OutDeferredButtonText)
	{
		if (Widget == nullptr)
		{
			return;
		}

		if (!Defaults.FontObjectPath.IsEmpty())
		{
			ApplyFontObjectToTextWidget(Widget, ResolveFontObject(Defaults.FontObjectPath, NodePath, Result));
		}

		if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
		{
			if (UDataTable* StyleSet = ResolveRichTextStyleSet(Defaults, NodePath, Result))
			{
				RichTextBlock->SetTextStyleSet(StyleSet);
			}
		}

		if (!Node.PropsObject.IsValid())
		{
			return;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Node.PropsObject->Values)
		{
			const FString& Key = Pair.Key;
			const TSharedPtr<FJsonValue>& Value = Pair.Value;
			bool bHandled = false;

			if (Key.Equals(TEXT("text"), ESearchCase::IgnoreCase))
			{
				FString TextValue;
				if (Value.IsValid() && Value->TryGetString(TextValue))
				{
					if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
					{
						TextBlock->SetText(FText::FromString(TextValue));
						bHandled = true;
					}
					else if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
					{
						RichTextBlock->SetText(FText::FromString(TextValue));
						bHandled = true;
					}
					else if (Cast<UButton>(Widget) != nullptr)
					{
						OutDeferredButtonText = TextValue;
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("fontObject"), ESearchCase::IgnoreCase))
			{
				FString FontObjectPath;
				if (Value.IsValid() && Value->TryGetString(FontObjectPath))
				{
					bHandled = ApplyFontObjectToTextWidget(Widget, ResolveFontObject(FontObjectPath, NodePath, Result));
				}
			}
			else if (Key.Equals(TEXT("fontSize"), ESearchCase::IgnoreCase))
			{
				double FontSize = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(FontSize))
				{
					if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
					{
						FSlateFontInfo FontInfo = TextBlock->GetFont();
						FontInfo.Size = static_cast<int32>(FontSize);
						TextBlock->SetFont(FontInfo);
						bHandled = true;
					}
					else if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
					{
						FTextBlockStyle TextStyle = RichTextBlock->GetDefaultTextStyle();
						FSlateFontInfo FontInfo = TextStyle.Font;
						FontInfo.Size = static_cast<int32>(FontSize);
						TextStyle.SetFont(FontInfo);
						RichTextBlock->SetDefaultTextStyle(TextStyle);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("color"), ESearchCase::IgnoreCase))
			{
				FLinearColor Color;
				if (TryReadLinearColor(Value, Color))
				{
					if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
					{
						TextBlock->SetColorAndOpacity(FSlateColor(Color));
						bHandled = true;
					}
					else if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
					{
						FTextBlockStyle TextStyle = RichTextBlock->GetDefaultTextStyle();
						TextStyle.SetColorAndOpacity(FSlateColor(Color));
						RichTextBlock->SetDefaultTextStyle(TextStyle);
						bHandled = true;
					}
					else if (UImage* Image = Cast<UImage>(Widget))
					{
						Image->SetColorAndOpacity(Color);
						bHandled = true;
					}
					else if (UButton* Button = Cast<UButton>(Widget))
					{
						Button->SetColorAndOpacity(Color);
						bHandled = true;
					}
					else if (UBorder* Border = Cast<UBorder>(Widget))
					{
						Border->SetContentColorAndOpacity(Color);
						bHandled = true;
					}
					else if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
					{
						ProgressBar->SetFillColorAndOpacity(Color);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("fillColor"), ESearchCase::IgnoreCase))
			{
				FLinearColor Color;
				if (TryReadLinearColor(Value, Color))
				{
					if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
					{
						ProgressBar->SetFillColorAndOpacity(Color);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("backgroundColor"), ESearchCase::IgnoreCase))
			{
				FLinearColor Color;
				if (TryReadLinearColor(Value, Color))
				{
					if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
					{
						FProgressBarStyle ProgressBarStyle = ProgressBar->GetWidgetStyle();
						FSlateBrush BackgroundImage = ProgressBarStyle.BackgroundImage;
						BackgroundImage.TintColor = FSlateColor(Color);
						ProgressBarStyle.SetBackgroundImage(BackgroundImage);
						ProgressBar->SetWidgetStyle(ProgressBarStyle);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("percent"), ESearchCase::IgnoreCase))
			{
				double Percent = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(Percent))
				{
					if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
					{
						ProgressBar->SetPercent(FMath::Clamp(static_cast<float>(Percent), 0.0f, 1.0f));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("isMarquee"), ESearchCase::IgnoreCase))
			{
				bool bIsMarquee = false;
				if (Value.IsValid() && Value->TryGetBool(bIsMarquee))
				{
					if (UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
					{
						ProgressBar->SetIsMarquee(bIsMarquee);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("brushColor"), ESearchCase::IgnoreCase))
			{
				FLinearColor Color;
				if (TryReadLinearColor(Value, Color))
				{
					if (UBorder* Border = Cast<UBorder>(Widget))
					{
						Border->SetBrushColor(Color);
						bHandled = true;
					}
					else if (UImage* Image = Cast<UImage>(Widget))
					{
						Image->SetBrushTintColor(FSlateColor(Color));
						bHandled = true;
					}
					else if (UButton* Button = Cast<UButton>(Widget))
					{
						Button->SetBackgroundColor(Color);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("brushResource"), ESearchCase::IgnoreCase) || Key.Equals(TEXT("brushMaterial"), ESearchCase::IgnoreCase))
			{
				FString ResourcePath;
				if (Value.IsValid() && Value->TryGetString(ResourcePath) && !ResourcePath.IsEmpty())
				{
					if (UImage* Image = Cast<UImage>(Widget))
					{
						if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *ResourcePath))
						{
							Image->SetBrushFromMaterial(Material);
							bHandled = true;
						}
						else if (!Key.Equals(TEXT("brushMaterial"), ESearchCase::IgnoreCase))
						{
							if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *ResourcePath))
							{
								Image->SetBrushFromTexture(Texture, false);
								bHandled = true;
							}
						}

						if (!bHandled)
						{
							AddWarning(Result, FString::Printf(TEXT("Could not load image brush resource '%s' for node '%s'."), *ResourcePath, *NodePath));
						}
					}
				}
			}
			else if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
			{
				FMargin Padding;
				if (TryReadMargin(Value, Padding))
				{
					if (UBorder* Border = Cast<UBorder>(Widget))
					{
						Border->SetPadding(Padding);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("desiredSize"), ESearchCase::IgnoreCase))
			{
				FVector2D DesiredSize;
				if (TryReadVector2D(Value, DesiredSize))
				{
					if (USizeBox* SizeBox = Cast<USizeBox>(Widget))
					{
						SizeBox->SetWidthOverride(DesiredSize.X);
						SizeBox->SetHeightOverride(DesiredSize.Y);
						bHandled = true;
					}
					else if (USpacer* Spacer = Cast<USpacer>(Widget))
					{
						Spacer->SetSize(DesiredSize);
						bHandled = true;
					}
					else if (UImage* Image = Cast<UImage>(Widget))
					{
						Image->SetDesiredSizeOverride(DesiredSize);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("widthOverride"), ESearchCase::IgnoreCase))
			{
				double WidthOverride = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(WidthOverride))
				{
					if (USizeBox* SizeBox = Cast<USizeBox>(Widget))
					{
						SizeBox->SetWidthOverride(static_cast<float>(WidthOverride));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("heightOverride"), ESearchCase::IgnoreCase))
			{
				double HeightOverride = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(HeightOverride))
				{
					if (USizeBox* SizeBox = Cast<USizeBox>(Widget))
					{
						SizeBox->SetHeightOverride(static_cast<float>(HeightOverride));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("autoWrap"), ESearchCase::IgnoreCase))
			{
				bool bAutoWrap = false;
				if (Value.IsValid() && Value->TryGetBool(bAutoWrap))
				{
					if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
					{
						TextBlock->SetAutoWrapText(bAutoWrap);
						bHandled = true;
					}
					else if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
					{
						RichTextBlock->SetAutoWrapText(bAutoWrap);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("orientation"), ESearchCase::IgnoreCase))
			{
				FString OrientationText;
				if (Value.IsValid() && Value->TryGetString(OrientationText))
				{
					const bool bHorizontal = OrientationText.Equals(TEXT("horizontal"), ESearchCase::IgnoreCase);
					const bool bVertical = OrientationText.Equals(TEXT("vertical"), ESearchCase::IgnoreCase);
					if (bHorizontal || bVertical)
					{
						const EOrientation Orientation = bHorizontal ? Orient_Horizontal : Orient_Vertical;
						if (UScrollBox* ScrollBox = Cast<UScrollBox>(Widget))
						{
							ScrollBox->SetOrientation(Orientation);
							bHandled = true;
						}
						else if (UWrapBox* WrapBox = Cast<UWrapBox>(Widget))
						{
							WrapBox->SetOrientation(Orientation);
							bHandled = true;
						}
					}
				}
			}
			else if (Key.Equals(TEXT("wrapSize"), ESearchCase::IgnoreCase))
			{
				double WrapSize = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(WrapSize))
				{
					if (UWrapBox* WrapBox = Cast<UWrapBox>(Widget))
					{
						WrapBox->SetExplicitWrapSize(true);
						WrapBox->SetWrapSize(static_cast<float>(WrapSize));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("innerSlotPadding"), ESearchCase::IgnoreCase))
			{
				FVector2D PaddingVector;
				if (TryReadVector2D(Value, PaddingVector))
				{
					if (UWrapBox* WrapBox = Cast<UWrapBox>(Widget))
					{
						WrapBox->SetInnerSlotPadding(PaddingVector);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("slotPadding"), ESearchCase::IgnoreCase))
			{
				FMargin Padding;
				if (TryReadMargin(Value, Padding))
				{
					if (UUniformGridPanel* UniformGridPanel = Cast<UUniformGridPanel>(Widget))
					{
						UniformGridPanel->SetSlotPadding(Padding);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("minDesiredSlotWidth"), ESearchCase::IgnoreCase))
			{
				double MinDesiredSlotWidth = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(MinDesiredSlotWidth))
				{
					if (UUniformGridPanel* UniformGridPanel = Cast<UUniformGridPanel>(Widget))
					{
						UniformGridPanel->SetMinDesiredSlotWidth(static_cast<float>(MinDesiredSlotWidth));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("minDesiredSlotHeight"), ESearchCase::IgnoreCase))
			{
				double MinDesiredSlotHeight = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(MinDesiredSlotHeight))
				{
					if (UUniformGridPanel* UniformGridPanel = Cast<UUniformGridPanel>(Widget))
					{
						UniformGridPanel->SetMinDesiredSlotHeight(static_cast<float>(MinDesiredSlotHeight));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("entryWidth"), ESearchCase::IgnoreCase))
			{
				double EntryWidth = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(EntryWidth))
				{
					if (UTileView* TileView = Cast<UTileView>(Widget))
					{
						TileView->SetEntryWidth(static_cast<float>(EntryWidth));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("entryHeight"), ESearchCase::IgnoreCase))
			{
				double EntryHeight = 0.0;
				if (Value.IsValid() && Value->TryGetNumber(EntryHeight))
				{
					if (UTileView* TileView = Cast<UTileView>(Widget))
					{
						TileView->SetEntryHeight(static_cast<float>(EntryHeight));
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("activeWidgetIndex"), ESearchCase::IgnoreCase))
			{
				int32 ActiveWidgetIndex = 0;
				if (Value.IsValid() && Value->TryGetNumber(ActiveWidgetIndex))
				{
					if (UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(Widget))
					{
						WidgetSwitcher->SetActiveWidgetIndex(ActiveWidgetIndex);
						bHandled = true;
					}
				}
			}
			else if (Key.Equals(TEXT("entryWidgetClass"), ESearchCase::IgnoreCase))
			{
				// Handled separately after widget construction so ListView/TileView can get a resolved or auto-generated entry class.
				bHandled = true;
			}
			else if (Key.Equals(TEXT("generatePreviewEntryWidget"), ESearchCase::IgnoreCase))
			{
				// Handled together with entryWidgetClass policy after widget construction.
				bHandled = true;
			}

			if (!bHandled)
			{
				AddWarning(Result, FString::Printf(TEXT("Unknown or unsupported prop '%s' on node '%s'."), *Key, *NodePath));
			}
		}
	}

	static void ApplySlotData(UPanelSlot* Slot, const TSharedPtr<FJsonObject>& SlotObject, const FString& NodePath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (Slot == nullptr || !SlotObject.IsValid())
		{
			return;
		}

		FString DeclaredKind;
		SlotObject->TryGetStringField(TEXT("kind"), DeclaredKind);
		if (!DeclaredKind.IsEmpty() && !Slot->GetClass()->GetName().Equals(DeclaredKind))
		{
			AddWarning(Result, FString::Printf(
				TEXT("Slot kind '%s' on node '%s' does not match generated slot '%s'. Applying actual slot type."),
				*DeclaredKind,
				*NodePath,
				*Slot->GetClass()->GetName()));
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : SlotObject->Values)
		{
			const FString& Key = Pair.Key;
			const TSharedPtr<FJsonValue>& Value = Pair.Value;
			bool bHandled = false;

			if (Key.Equals(TEXT("kind"), ESearchCase::IgnoreCase))
			{
				bHandled = true;
			}
			else if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
			{
				if (Key.Equals(TEXT("anchors"), ESearchCase::IgnoreCase))
				{
					FAnchors Anchors;
					if (TryReadAnchors(Value, Anchors))
					{
						CanvasSlot->SetAnchors(Anchors);
						bHandled = true;
					}
				}
				else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
				{
					FVector2D Alignment;
					if (TryReadVector2D(Value, Alignment))
					{
						CanvasSlot->SetAlignment(Alignment);
						bHandled = true;
					}
				}
				else if (Key.Equals(TEXT("position"), ESearchCase::IgnoreCase))
				{
					FVector2D Position;
					if (TryReadVector2D(Value, Position))
					{
						CanvasSlot->SetPosition(Position);
						bHandled = true;
					}
				}
				else if (Key.Equals(TEXT("size"), ESearchCase::IgnoreCase))
				{
					FVector2D Size;
					if (TryReadVector2D(Value, Size))
					{
						CanvasSlot->SetSize(Size);
						bHandled = true;
					}
				}
				else if (Key.Equals(TEXT("offsets"), ESearchCase::IgnoreCase))
				{
					FMargin Offsets;
					if (TryReadMargin(Value, Offsets))
					{
						CanvasSlot->SetOffsets(Offsets);
						bHandled = true;
					}
				}
			}

			if (!bHandled)
			{
				if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							ButtonSlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							ButtonSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							ButtonSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("horizontalAlignment"), ESearchCase::IgnoreCase))
					{
						EHorizontalAlignment HorizontalAlignment = HAlign_Center;
						if (TryReadHorizontalAlignmentKeyword(Value, HorizontalAlignment))
						{
							ButtonSlot->SetHorizontalAlignment(HorizontalAlignment);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("verticalAlignment"), ESearchCase::IgnoreCase))
					{
						EVerticalAlignment VerticalAlignment = VAlign_Center;
						if (TryReadVerticalAlignmentKeyword(Value, VerticalAlignment))
						{
							ButtonSlot->SetVerticalAlignment(VerticalAlignment);
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				if (UScrollBoxSlot* ScrollBoxSlot = Cast<UScrollBoxSlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							ScrollBoxSlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("sizeRule"), ESearchCase::IgnoreCase))
					{
						FString SizeRuleText;
						if (Value.IsValid() && Value->TryGetString(SizeRuleText))
						{
							const FSlateChildSize Size = MakeSlateChildSize(SizeRuleText, ScrollBoxSlot->GetSize().Value);
							ScrollBoxSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("fillWidth"), ESearchCase::IgnoreCase))
					{
						double FillValue = 1.0;
						if (Value.IsValid() && Value->TryGetNumber(FillValue))
						{
							FSlateChildSize Size = ScrollBoxSlot->GetSize();
							Size.SizeRule = ESlateSizeRule::Fill;
							Size.Value = static_cast<float>(FillValue);
							ScrollBoxSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							ScrollBoxSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							ScrollBoxSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							VerticalSlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("sizeRule"), ESearchCase::IgnoreCase))
					{
						FString SizeRuleText;
						if (Value.IsValid() && Value->TryGetString(SizeRuleText))
						{
							const FSlateChildSize Size = MakeSlateChildSize(SizeRuleText, VerticalSlot->GetSize().Value);
							VerticalSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("fillWidth"), ESearchCase::IgnoreCase))
					{
						double FillValue = 1.0;
						if (Value.IsValid() && Value->TryGetNumber(FillValue))
						{
							FSlateChildSize Size = VerticalSlot->GetSize();
							Size.SizeRule = ESlateSizeRule::Fill;
							Size.Value = static_cast<float>(FillValue);
							VerticalSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							VerticalSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							VerticalSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("horizontalAlignment"), ESearchCase::IgnoreCase))
					{
						EHorizontalAlignment HorizontalAlignment = HAlign_Center;
						if (TryReadHorizontalAlignmentKeyword(Value, HorizontalAlignment))
						{
							VerticalSlot->SetHorizontalAlignment(HorizontalAlignment);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("verticalAlignment"), ESearchCase::IgnoreCase))
					{
						EVerticalAlignment VerticalAlignment = VAlign_Center;
						if (TryReadVerticalAlignmentKeyword(Value, VerticalAlignment))
						{
							VerticalSlot->SetVerticalAlignment(VerticalAlignment);
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				if (UHorizontalBoxSlot* HorizontalSlot = Cast<UHorizontalBoxSlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							HorizontalSlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("sizeRule"), ESearchCase::IgnoreCase))
					{
						FString SizeRuleText;
						if (Value.IsValid() && Value->TryGetString(SizeRuleText))
						{
							const FSlateChildSize Size = MakeSlateChildSize(SizeRuleText, HorizontalSlot->GetSize().Value);
							HorizontalSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("fillWidth"), ESearchCase::IgnoreCase))
					{
						double FillValue = 1.0;
						if (Value.IsValid() && Value->TryGetNumber(FillValue))
						{
							FSlateChildSize Size = HorizontalSlot->GetSize();
							Size.SizeRule = ESlateSizeRule::Fill;
							Size.Value = static_cast<float>(FillValue);
							HorizontalSlot->SetSize(Size);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							HorizontalSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							HorizontalSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("horizontalAlignment"), ESearchCase::IgnoreCase))
					{
						EHorizontalAlignment HorizontalAlignment = HAlign_Center;
						if (TryReadHorizontalAlignmentKeyword(Value, HorizontalAlignment))
						{
							HorizontalSlot->SetHorizontalAlignment(HorizontalAlignment);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("verticalAlignment"), ESearchCase::IgnoreCase))
					{
						EVerticalAlignment VerticalAlignment = VAlign_Center;
						if (TryReadVerticalAlignmentKeyword(Value, VerticalAlignment))
						{
							HorizontalSlot->SetVerticalAlignment(VerticalAlignment);
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				if (UUniformGridSlot* UniformGridSlot = Cast<UUniformGridSlot>(Slot))
				{
					if (Key.Equals(TEXT("row"), ESearchCase::IgnoreCase))
					{
						int32 Row = 0;
						if (Value.IsValid() && Value->TryGetNumber(Row))
						{
							UniformGridSlot->SetRow(Row);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("column"), ESearchCase::IgnoreCase))
					{
						int32 Column = 0;
						if (Value.IsValid() && Value->TryGetNumber(Column))
						{
							UniformGridSlot->SetColumn(Column);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							UniformGridSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							UniformGridSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("rowSpan"), ESearchCase::IgnoreCase) ||
							 Key.Equals(TEXT("columnSpan"), ESearchCase::IgnoreCase) ||
							 Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						// UE5.5 UUniformGridSlot does not expose per-child rowSpan/columnSpan/padding.
						AddWarning(Result, FString::Printf(
							TEXT("UniformGridSlot on node '%s' does not support '%s' in UE5.5; field is ignored."),
							*NodePath,
							*Key));
						bHandled = true;
					}
				}
			}

			if (!bHandled)
			{
				if (UWrapBoxSlot* WrapBoxSlot = Cast<UWrapBoxSlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							WrapBoxSlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("fillSpanWhenLessThan"), ESearchCase::IgnoreCase))
					{
						double FillSpanWhenLessThan = 0.0;
						if (Value.IsValid() && Value->TryGetNumber(FillSpanWhenLessThan))
						{
							WrapBoxSlot->SetFillSpanWhenLessThan(static_cast<float>(FillSpanWhenLessThan));
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("fillEmptySpace"), ESearchCase::IgnoreCase))
					{
						bool bFillEmptySpace = false;
						if (Value.IsValid() && Value->TryGetBool(bFillEmptySpace))
						{
							WrapBoxSlot->SetFillEmptySpace(bFillEmptySpace);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							WrapBoxSlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							WrapBoxSlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Slot))
				{
					if (Key.Equals(TEXT("padding"), ESearchCase::IgnoreCase))
					{
						FMargin Padding;
						if (TryReadMargin(Value, Padding))
						{
							OverlaySlot->SetPadding(Padding);
							bHandled = true;
						}
					}
					else if (Key.Equals(TEXT("alignment"), ESearchCase::IgnoreCase))
					{
						FVector2D Alignment;
						if (TryReadVector2D(Value, Alignment))
						{
							OverlaySlot->SetHorizontalAlignment(ToHorizontalAlignment(Alignment.X));
							OverlaySlot->SetVerticalAlignment(ToVerticalAlignment(Alignment.Y));
							bHandled = true;
						}
					}
				}
			}

			if (!bHandled)
			{
				AddWarning(Result, FString::Printf(TEXT("Unknown or unsupported slot field '%s' on node '%s'."), *Key, *NodePath));
			}
		}
	}

	static UClass* ResolveNotifyFieldValueClass(const FString& ClassText)
	{
		if (ClassText.IsEmpty())
		{
			return nullptr;
		}

		if (UClass* LoadedClass = FindObject<UClass>(nullptr, *ClassText))
		{
			if (LoadedClass->ImplementsInterface(UNotifyFieldValueChanged::StaticClass()))
			{
				return LoadedClass;
			}
		}

		if (UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassText))
		{
			if (LoadedClass->ImplementsInterface(UNotifyFieldValueChanged::StaticClass()))
			{
				return LoadedClass;
			}
		}

		const FString TrimmedName = ClassText.TrimStartAndEnd();
		const FString ExpectedRawName = TrimmedName.StartsWith(TEXT("U")) ? TrimmedName : FString(TEXT("U")) + TrimmedName;
		const FString ExpectedGeneratedName = TrimmedName.EndsWith(TEXT("_C")) ? TrimmedName : TrimmedName + TEXT("_C");
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;
			if (Class == nullptr || !Class->ImplementsInterface(UNotifyFieldValueChanged::StaticClass()))
			{
				continue;
			}

			if (Class->GetName().Equals(TrimmedName) ||
				Class->GetName().Equals(ExpectedRawName) ||
				Class->GetName().Equals(ExpectedGeneratedName) ||
				Class->GetAuthoredName().Equals(TrimmedName))
			{
				return Class;
			}
		}

		return nullptr;
	}

	static bool IsManualViewModelCreationType(const FString& CreationType)
	{
		return CreationType.IsEmpty() || CreationType.Equals(TEXT("Manual"), ESearchCase::IgnoreCase);
	}

	static void CollectMVVMBindingsRecursive(const FImportedWidgetNode& Node, TArray<FImportedMVVMBindingDefinition>& OutBindings)
	{
		if (Node.MVVMObject.IsValid())
		{
			FImportedMVVMBindingDefinition Binding;
			Binding.WidgetName = Node.Name;
			Node.MVVMObject->TryGetStringField(TEXT("destination"), Binding.Destination);
			Node.MVVMObject->TryGetStringField(TEXT("sourceViewModel"), Binding.SourceViewModel);
			Node.MVVMObject->TryGetStringField(TEXT("sourceField"), Binding.SourceField);
			if (!Binding.WidgetName.IsEmpty() && !Binding.Destination.IsEmpty() && !Binding.SourceViewModel.IsEmpty() && !Binding.SourceField.IsEmpty())
			{
				OutBindings.Add(MoveTemp(Binding));
			}
		}

		for (const FImportedWidgetNode& ChildNode : Node.Children)
		{
			CollectMVVMBindingsRecursive(ChildNode, OutBindings);
		}
	}

	static void ApplyMVVMViewModels(UWidgetBlueprint* WidgetBlueprint, const FImportedMVVMSchema& MVVMSchema, TMap<FString, FGuid>& OutViewModelIdsByName, FJsonWidgetBlueprintImportResult& Result)
	{
		if (WidgetBlueprint == nullptr || GEditor == nullptr || MVVMSchema.ViewModels.IsEmpty())
		{
			return;
		}

		UMVVMEditorSubsystem* MVVMEditorSubsystem = GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>();
		if (MVVMEditorSubsystem == nullptr)
		{
			AddWarning(Result, TEXT("MVVM editor subsystem is unavailable; skipping automatic MVVM setup."));
			return;
		}

		UMVVMBlueprintView* BlueprintView = MVVMEditorSubsystem->RequestView(WidgetBlueprint);
		if (BlueprintView == nullptr)
		{
			AddWarning(Result, TEXT("Failed to create MVVM blueprint view; skipping automatic MVVM setup."));
			return;
		}

		for (const FImportedViewModelDefinition& ViewModelDefinition : MVVMSchema.ViewModels)
		{
			UClass* ViewModelClass = ResolveNotifyFieldValueClass(ViewModelDefinition.ClassName);
			if (ViewModelClass == nullptr)
			{
				AddWarning(Result, FString::Printf(TEXT("Could not resolve MVVM viewmodel class '%s'."), *ViewModelDefinition.ClassName));
				continue;
			}

			FGuid ViewModelId;
			if (const FMVVMBlueprintViewModelContext* ExistingViewModel = BlueprintView->FindViewModel(FName(*ViewModelDefinition.Name)))
			{
				ViewModelId = ExistingViewModel->GetViewModelId();
			}
			else
			{
				ViewModelId = MVVMEditorSubsystem->AddViewModel(WidgetBlueprint, ViewModelClass);
			}

			if (!ViewModelId.IsValid())
			{
				AddWarning(Result, FString::Printf(TEXT("Failed to add MVVM viewmodel '%s'."), *ViewModelDefinition.Name));
				continue;
			}

			if (FMVVMBlueprintViewModelContext* ViewModelContext = BlueprintView->FindViewModel(ViewModelId))
			{
				BlueprintView->Modify();
				ViewModelContext->ViewModelName = FName(*ViewModelDefinition.Name);
				if (IsManualViewModelCreationType(ViewModelDefinition.CreationType))
				{
					ViewModelContext->CreationType = EMVVMBlueprintViewModelContextCreationType::Manual;
					ViewModelContext->bOptional = true;
					ViewModelContext->bCreateSetterFunction = true;
				}
			}

			OutViewModelIdsByName.Add(ViewModelDefinition.Name, ViewModelId);
		}
	}

	static void ApplyMVVMBindings(UWidgetBlueprint* WidgetBlueprint, const TArray<FImportedMVVMBindingDefinition>& BindingDefinitions, const TMap<FString, FGuid>& ViewModelIdsByName, FJsonWidgetBlueprintImportResult& Result)
	{
		if (WidgetBlueprint == nullptr || GEditor == nullptr || BindingDefinitions.IsEmpty())
		{
			return;
		}

		UMVVMEditorSubsystem* MVVMEditorSubsystem = GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>();
		UMVVMBlueprintView* BlueprintView = MVVMEditorSubsystem ? MVVMEditorSubsystem->RequestView(WidgetBlueprint) : nullptr;
		if (MVVMEditorSubsystem == nullptr || BlueprintView == nullptr)
		{
			AddWarning(Result, TEXT("MVVM editor subsystem is unavailable; skipping automatic MVVM bindings."));
			return;
		}

		for (const FImportedMVVMBindingDefinition& BindingDefinition : BindingDefinitions)
		{
			const FGuid* ViewModelId = ViewModelIdsByName.Find(BindingDefinition.SourceViewModel);
			if (ViewModelId == nullptr || !ViewModelId->IsValid())
			{
				AddWarning(Result, FString::Printf(
					TEXT("Skipping MVVM binding for widget '%s' because source viewmodel '%s' is not registered."),
					*BindingDefinition.WidgetName,
					*BindingDefinition.SourceViewModel));
				continue;
			}

			const FMVVMBlueprintViewModelContext* ViewModelContext = BlueprintView->FindViewModel(*ViewModelId);
			UClass* ViewModelClass = ViewModelContext ? ViewModelContext->GetViewModelClass() : nullptr;
			const FProperty* SourceProperty = ViewModelClass ? FindFProperty<FProperty>(ViewModelClass, FName(*BindingDefinition.SourceField)) : nullptr;
			if (SourceProperty == nullptr)
			{
				AddWarning(Result, FString::Printf(
					TEXT("Skipping MVVM binding for widget '%s' because source field '%s.%s' was not found."),
					*BindingDefinition.WidgetName,
					*BindingDefinition.SourceViewModel,
					*BindingDefinition.SourceField));
				continue;
			}

			UWidget* Widget = WidgetBlueprint->WidgetTree ? WidgetBlueprint->WidgetTree->FindWidget(FName(*BindingDefinition.WidgetName)) : nullptr;
			const FProperty* DestinationProperty = Widget ? FindFProperty<FProperty>(Widget->GetClass(), FName(*BindingDefinition.Destination)) : nullptr;
			if (Widget == nullptr || DestinationProperty == nullptr)
			{
				AddWarning(Result, FString::Printf(
					TEXT("Skipping MVVM binding for widget '%s' because destination property '%s' was not found."),
					*BindingDefinition.WidgetName,
					*BindingDefinition.Destination));
				continue;
			}

			if (FMVVMBlueprintViewBinding* ExistingBinding = BlueprintView->FindBinding(Widget, DestinationProperty))
			{
				BlueprintView->RemoveBinding(ExistingBinding);
			}

			FMVVMBlueprintViewBinding& NewBinding = MVVMEditorSubsystem->AddBinding(WidgetBlueprint);

			FMVVMBlueprintPropertyPath SourcePath;
			SourcePath.SetViewModelId(*ViewModelId);
			SourcePath.SetPropertyPath(WidgetBlueprint, UE::MVVM::FMVVMConstFieldVariant(SourceProperty));
			MVVMEditorSubsystem->SetSourcePathForBinding(WidgetBlueprint, NewBinding, SourcePath);

			FMVVMBlueprintPropertyPath DestinationPath;
			DestinationPath.SetWidgetName(Widget->GetFName());
			DestinationPath.SetPropertyPath(WidgetBlueprint, UE::MVVM::FMVVMConstFieldVariant(DestinationProperty));
			MVVMEditorSubsystem->SetDestinationPathForBinding(WidgetBlueprint, NewBinding, DestinationPath, false);
			MVVMEditorSubsystem->SetBindingTypeForBinding(WidgetBlueprint, NewBinding, EMVVMBindingMode::OneWayToDestination);
			MVVMEditorSubsystem->SetEnabledForBinding(WidgetBlueprint, NewBinding, true);
			MVVMEditorSubsystem->SetCompileForBinding(WidgetBlueprint, NewBinding, true);
		}
	}

	static UPanelSlot* AttachWidgetToParent(UWidget* ParentWidget, UWidget* ChildWidget, const FString& ParentPath, FJsonWidgetBlueprintImportResult& Result)
	{
		if (ParentWidget == nullptr || ChildWidget == nullptr)
		{
			return nullptr;
		}

		if (UContentWidget* ContentWidget = Cast<UContentWidget>(ParentWidget))
		{
			if (ContentWidget->GetContent() != nullptr)
			{
				AddWarning(Result, FString::Printf(
					TEXT("Widget '%s' only supports one child. Extra children are ignored."),
					*ParentPath));
				return nullptr;
			}

			return ContentWidget->SetContent(ChildWidget);
		}

		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(ParentWidget))
		{
			return PanelWidget->AddChild(ChildWidget);
		}

		AddWarning(Result, FString::Printf(
			TEXT("Widget '%s' does not support children. Child '%s' is ignored."),
			*ParentPath,
			*ChildWidget->GetName()));
		return nullptr;
	}

	static void BuildButtonLabel(UWidgetTree* WidgetTree, UButton* Button, const FImportedWidgetNode& Node, const FString& NodePath, const FString& ButtonText, const FImportedSchemaDefaults& Defaults, FJsonWidgetBlueprintImportResult& Result)
	{
		if (WidgetTree == nullptr || Button == nullptr || ButtonText.IsEmpty())
		{
			return;
		}

		if (Button->GetContent() != nullptr)
		{
			AddWarning(Result, FString::Printf(
				TEXT("Button node '%s' has both 'props.text' and explicit children; text prop is ignored."),
				*NodePath));
			return;
		}

		const FString LabelName = SanitizeName(Node.Name.IsEmpty() ? TEXT("ButtonLabel") : Node.Name + TEXT("_Label"));
		UTextBlock* LabelWidget = Cast<UTextBlock>(WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			MakeUniqueObjectName(WidgetTree, UTextBlock::StaticClass(), FName(*LabelName))));

		if (LabelWidget == nullptr)
		{
			AddWarning(Result, FString::Printf(TEXT("Failed to create implicit text label for button '%s'."), *NodePath));
			return;
		}

		LabelWidget->SetText(FText::FromString(ButtonText));
		Button->SetContent(LabelWidget);

		if (!Defaults.FontObjectPath.IsEmpty())
		{
			ApplyFontObjectToTextWidget(LabelWidget, ResolveFontObject(Defaults.FontObjectPath, NodePath, Result));
		}

		if (Node.PropsObject.IsValid())
		{
			const TSharedPtr<FJsonValue>* FontObjectValue = Node.PropsObject->Values.Find(TEXT("fontObject"));
			if (FontObjectValue != nullptr && FontObjectValue->IsValid())
			{
				FString FontObjectPath;
				if ((*FontObjectValue)->TryGetString(FontObjectPath))
				{
					ApplyFontObjectToTextWidget(LabelWidget, ResolveFontObject(FontObjectPath, NodePath, Result));
				}
			}

			const TSharedPtr<FJsonValue>* FontSizeValue = Node.PropsObject->Values.Find(TEXT("fontSize"));
			if (FontSizeValue != nullptr && FontSizeValue->IsValid())
			{
				double FontSize = 0.0;
				if ((*FontSizeValue)->TryGetNumber(FontSize))
				{
					FSlateFontInfo FontInfo = LabelWidget->GetFont();
					FontInfo.Size = static_cast<int32>(FontSize);
					LabelWidget->SetFont(FontInfo);
				}
			}

			const TSharedPtr<FJsonValue>* ColorValue = Node.PropsObject->Values.Find(TEXT("color"));
			if (ColorValue != nullptr && ColorValue->IsValid())
			{
				FLinearColor Color;
				if (TryReadLinearColor(*ColorValue, Color))
				{
					LabelWidget->SetColorAndOpacity(FSlateColor(Color));
				}
			}
		}
	}

	static UWidget* BuildNodeRecursive(UWidgetBlueprint* Blueprint, UWidget* ParentWidget, const FImportedWidgetNode& Node, const FString& ParentPath, const FImportedSchemaDefaults& Defaults, FJsonWidgetBlueprintImportResult& Result)
	{
		if (Blueprint == nullptr || Blueprint->WidgetTree == nullptr)
		{
			return nullptr;
		}

		const FString NodePath = MakeNodePath(ParentPath, Node.Name, Node.Type);
		UWidget* Widget = CreateWidgetByType(Blueprint->WidgetTree, Node.Type, Node.Name, Result);
		if (Widget == nullptr)
		{
			return nullptr;
		}

		ApplyBindWidgetSetting(Widget, Node, NodePath, Result);

		FString DeferredButtonText;
		ApplyWidgetProps(Widget, Node, NodePath, Defaults, Result, DeferredButtonText);

		if (ParentWidget == nullptr)
		{
			Blueprint->WidgetTree->RootWidget = Widget;
		}
		else
		{
			UPanelSlot* Slot = AttachWidgetToParent(ParentWidget, Widget, ParentPath, Result);
			ApplySlotData(Slot, Node.SlotObject, NodePath, Result);
		}

		for (const FImportedWidgetNode& ChildNode : Node.Children)
		{
			BuildNodeRecursive(Blueprint, Widget, ChildNode, NodePath, Defaults, Result);
		}

		ApplySemanticMetadata(Widget, Node);

		// ListView/TileView entry blueprints may trigger compilation work and GC. Do this only after
		// the widget has been attached to the current WidgetTree so the widget instance is strongly owned.
		EnsureListEntryWidgetClass(Blueprint, Widget, Node, NodePath, Result);

		if (UButton* Button = Cast<UButton>(Widget))
		{
			BuildButtonLabel(Blueprint->WidgetTree, Button, Node, NodePath, DeferredButtonText, Defaults, Result);
		}

		return Widget;
	}
}

FJsonWidgetBlueprintImportResult FJsonWidgetBlueprintImporter::ImportFromFile(const FJsonWidgetBlueprintImportRequest& Request)
{
	using namespace JsonWidgetBlueprintImporter;

	FJsonWidgetBlueprintImportResult Result;

	if (Request.JsonFilePath.IsEmpty())
	{
		AddError(Result, TEXT("JSON file path is empty."));
		return Result;
	}

	if (Request.TargetContentFolder.IsEmpty())
	{
		AddError(Result, TEXT("Target content folder is empty."));
		return Result;
	}

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *Request.JsonFilePath))
	{
		AddError(Result, FString::Printf(TEXT("Failed to read JSON file '%s'."), *Request.JsonFilePath));
		return Result;
	}

	FImportedSchema Schema;
	if (!ParseSchema(JsonText, Schema, Result))
	{
		return Result;
	}

	const UClass* ParentClass = ResolveParentClass(Schema.ParentClass, Result);
	if (ParentClass == nullptr)
	{
		return Result;
	}

	FString TargetContentFolder = Request.TargetContentFolder;
	TargetContentFolder.RemoveFromEnd(TEXT("/"));
	if (!TargetContentFolder.StartsWith(TEXT("/Game")))
	{
		AddError(Result, FString::Printf(TEXT("Target content folder must be under /Game. Got '%s'."), *TargetContentFolder));
		return Result;
	}

	if (!FPackageName::IsValidLongPackageName(TargetContentFolder))
	{
		AddError(Result, FString::Printf(TEXT("Invalid target content folder '%s'."), *TargetContentFolder));
		return Result;
	}

	const FString AssetName = SanitizeName(Schema.WidgetBlueprintName);
	const FString PackageName = TargetContentFolder / AssetName;
	if (FPackageName::DoesPackageExist(PackageName))
	{
		AddError(Result, FString::Printf(TEXT("Asset '%s' already exists."), *PackageName));
		return Result;
	}

	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->BlueprintType = BPTYPE_Normal;
	Factory->ParentClass = const_cast<UClass*>(ParentClass);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UObject* CreatedAsset = AssetTools.CreateAsset(AssetName, TargetContentFolder, UWidgetBlueprint::StaticClass(), Factory);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(CreatedAsset);
	if (WidgetBlueprint == nullptr || WidgetBlueprint->WidgetTree == nullptr)
	{
		AddError(Result, TEXT("Failed to create Widget Blueprint asset."));
		return Result;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();
	Schema.Defaults.GeneratedRichTextStyleSet = CreateRichTextStyleSetDataTable(WidgetBlueprint, Schema, Result);

	if (WidgetBlueprint->WidgetTree->RootWidget != nullptr)
	{
		WidgetBlueprint->WidgetTree->RemoveWidget(WidgetBlueprint->WidgetTree->RootWidget);
		WidgetBlueprint->WidgetTree->RootWidget = nullptr;
	}

	if (BuildNodeRecursive(WidgetBlueprint, nullptr, Schema.Root, TEXT(""), Schema.Defaults, Result) == nullptr)
	{
		AddError(Result, TEXT("Failed to build widget tree from schema root."));
		return Result;
	}

	TMap<FString, FGuid> ViewModelIdsByName;
	ApplyMVVMViewModels(WidgetBlueprint, Schema.MVVM, ViewModelIdsByName, Result);

	TArray<FImportedMVVMBindingDefinition> MVVMBindingDefinitions;
	CollectMVVMBindingsRecursive(Schema.Root, MVVMBindingDefinitions);
	ApplyMVVMBindings(WidgetBlueprint, MVVMBindingDefinitions, ViewModelIdsByName, Result);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	if (Request.bOpenBlueprintAfterImport && GEditor != nullptr)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditorSubsystem->OpenEditorForAsset(WidgetBlueprint);
		}
	}

	Result.bSuccess = true;
	Result.AssetPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	return Result;
}

UClass* FJsonWidgetBlueprintImporter::ResolveParentClass(const FString& ParentClassText, FJsonWidgetBlueprintImportResult& Result)
{
	using namespace JsonWidgetBlueprintImporter;

	if (ParentClassText.IsEmpty())
	{
		return UUserWidget::StaticClass();
	}

	if (UClass* LoadedClass = FindObject<UClass>(nullptr, *ParentClassText))
	{
		if (LoadedClass->IsChildOf(UUserWidget::StaticClass()))
		{
			return LoadedClass;
		}
	}

	if (UClass* LoadedClass = LoadClass<UUserWidget>(nullptr, *ParentClassText))
	{
		return LoadedClass;
	}

	const FString TrimmedName = ParentClassText.TrimStartAndEnd();
	const FString ExpectedRawName = TrimmedName.StartsWith(TEXT("U")) ? TrimmedName : FString(TEXT("U")) + TrimmedName;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (Class == nullptr || !Class->IsChildOf(UUserWidget::StaticClass()))
		{
			continue;
		}

		if (Class->GetName().Equals(TrimmedName) ||
			Class->GetName().Equals(ExpectedRawName) ||
			Class->GetAuthoredName().Equals(TrimmedName))
		{
			return Class;
		}
	}

	AddError(Result, FString::Printf(
		TEXT("Failed to resolve parentClass '%s'. Use a valid widget class name or class path."),
		*ParentClassText));
	return nullptr;
}
