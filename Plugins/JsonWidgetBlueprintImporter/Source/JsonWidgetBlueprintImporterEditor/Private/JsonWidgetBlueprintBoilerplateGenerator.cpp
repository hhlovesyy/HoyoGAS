#include "JsonWidgetBlueprintBoilerplateGenerator.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

namespace JsonWidgetBlueprintBoilerplateGenerator
{
	struct FManifestCallback
	{
		FString Widget;
		FString Function;
		FString WidgetPath;
	};

	struct FManifestViewModelField
	{
		FString Widget;
		FString Field;
		FString WidgetPath;
	};

	struct FManifestAnnotatedWidget
	{
		FString Widget;
		FString WidgetClass;
		FString WidgetPath;
		bool bBindWidget = false;
		FString Callback;
		FString ViewModelField;
		FString SemanticRole;
		FString Notes;
	};

	struct FManifestSchema
	{
		FString WidgetBlueprint;
		FString ParentClass;
		FString WidgetKind;
		bool bSingleMainViewModel = false;
		TArray<FString> BindWidgets;
		TArray<FManifestCallback> Callbacks;
		TArray<FManifestViewModelField> ViewModelFields;
		TArray<FManifestAnnotatedWidget> AnnotatedWidgets;
	};

	struct FWidgetBindingInfo
	{
		FString WidgetName;
		FString WidgetClassName;
		FString WidgetPath;
		FString SemanticRole;
	};

	struct FResolvedNames
	{
		FString ScreenStem;
		FString StoreStem;
		FString ScreenClassName;
		FString ScreenFileStem;
		FString ViewModelClassName;
		FString ViewModelFileStem;
		FString StoreClassName;
		FString StoreFileStem;
		FString ApiMacro;
		FString ModuleName;
	};

	struct FBaseClassInfo
	{
		FString IncludePath;
		FString ClassName;
	};

	static void AddWarning(FJsonWidgetBlueprintBoilerplateGenerationResult& Result, const FString& Message)
	{
		Result.Warnings.Add(Message);
	}

	static void AddError(FJsonWidgetBlueprintBoilerplateGenerationResult& Result, const FString& Message)
	{
		Result.Errors.Add(Message);
	}

	static bool TryGetObjectField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, TSharedPtr<FJsonObject>& OutObject)
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

	static bool TryGetArrayField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const TArray<TSharedPtr<FJsonValue>>*& OutArray)
	{
		return Object.IsValid() && Object->TryGetArrayField(FieldName, OutArray);
	}

	static FString SanitizeIdentifier(const FString& InValue)
	{
		FString Result = InValue.TrimStartAndEnd();
		for (TCHAR& Char : Result)
		{
			if (!(FChar::IsAlnum(Char) || Char == TEXT('_')))
			{
				Char = TEXT('_');
			}
		}

		while (Result.StartsWith(TEXT("_")))
		{
			Result.RightChopInline(1);
		}

		if (Result.IsEmpty())
		{
			Result = TEXT("Generated");
		}

		if (FChar::IsDigit(Result[0]))
		{
			Result = FString(TEXT("_")) + Result;
		}

		return Result;
	}

	static FString StripPrefixIfPresent(const FString& InValue, const TCHAR* Prefix)
	{
		return InValue.StartsWith(Prefix) ? InValue.RightChop(FCString::Strlen(Prefix)) : InValue;
	}

	static FString StripTrailingSuffixIfPresent(const FString& InValue, const TCHAR* Suffix)
	{
		return InValue.EndsWith(Suffix) ? InValue.LeftChop(FCString::Strlen(Suffix)) : InValue;
	}

	static FString ToApiMacro(const FString& ModuleName)
	{
		FString Upper = ModuleName.ToUpper();
		Upper.ReplaceInline(TEXT(" "), TEXT(""));
		Upper.ReplaceInline(TEXT("-"), TEXT(""));
		return FString::Printf(TEXT("%s_API"), *Upper);
	}

	static FString ResolveModuleNameFromSourceRoot(const FString& SourceRoot)
	{
		FString CurrentDirectory = FPaths::ConvertRelativePathToFull(SourceRoot);
		IFileManager& FileManager = IFileManager::Get();

		while (!CurrentDirectory.IsEmpty())
		{
			TArray<FString> BuildFiles;
			FileManager.FindFiles(BuildFiles, *(CurrentDirectory / TEXT("*.Build.cs")), true, false);
			if (BuildFiles.Num() > 0)
			{
				return StripTrailingSuffixIfPresent(FPaths::GetBaseFilename(BuildFiles[0]), TEXT(".Build"));
			}

			const FString ParentDirectory = FPaths::GetPath(CurrentDirectory);
			if (ParentDirectory == CurrentDirectory)
			{
				break;
			}

			CurrentDirectory = ParentDirectory;
		}

		return FPaths::GetCleanFilename(FPaths::ConvertRelativePathToFull(SourceRoot));
	}

	static FBaseClassInfo ResolveScreenBaseClass(const FString& ParentClassName)
	{
		const FString TrimmedName = ParentClassName.TrimStartAndEnd();
		if (TrimmedName.Equals(TEXT("MyMVVMScreenBase"), ESearchCase::IgnoreCase) ||
			TrimmedName.Equals(TEXT("UMyMVVMScreenBase"), ESearchCase::IgnoreCase))
		{
			return { TEXT("Widgets/MyMVVMScreenBase.h"), TEXT("UMyMVVMScreenBase") };
		}

		if (TrimmedName.Equals(TEXT("MyMVVMDialogBase"), ESearchCase::IgnoreCase) ||
			TrimmedName.Equals(TEXT("UMyMVVMDialogBase"), ESearchCase::IgnoreCase))
		{
			return { TEXT("Widgets/MyMVVMDialogBase.h"), TEXT("UMyMVVMDialogBase") };
		}

		if (TrimmedName.Equals(TEXT("MyScreenBase"), ESearchCase::IgnoreCase) ||
			TrimmedName.Equals(TEXT("UMyScreenBase"), ESearchCase::IgnoreCase))
		{
			return { TEXT("Widgets/MyScreenBase.h"), TEXT("UMyScreenBase") };
		}

		return { TEXT("Widgets/MyMVVMScreenBase.h"), TEXT("UMyMVVMScreenBase") };
	}

	static FString ResolveWidgetClassName(const FString& WidgetClass)
	{
		const FString Trimmed = WidgetClass.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return TEXT("UWidget");
		}

		if (Trimmed.StartsWith(TEXT("U")))
		{
			return Trimmed;
		}

		return FString(TEXT("U")) + Trimmed;
	}

	static FString MakeViewModelFieldPropertyName(const FString& FieldName)
	{
		return SanitizeIdentifier(FieldName);
	}

	static bool ParseManifestSchema(
		const FString& JsonText,
		FManifestSchema& OutSchema,
		FJsonWidgetBlueprintBoilerplateGenerationResult& Result)
	{
		TSharedPtr<FJsonObject> RootObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			AddError(Result, TEXT("Failed to deserialize manifest JSON."));
			return false;
		}

		if (!RootObject->TryGetStringField(TEXT("widgetBlueprint"), OutSchema.WidgetBlueprint) || OutSchema.WidgetBlueprint.IsEmpty())
		{
			AddError(Result, TEXT("Manifest is missing 'widgetBlueprint'."));
			return false;
		}

		RootObject->TryGetStringField(TEXT("parentClass"), OutSchema.ParentClass);
		RootObject->TryGetStringField(TEXT("widgetKind"), OutSchema.WidgetKind);

		if (TSharedPtr<FJsonObject> UIFrameworkObject; TryGetObjectField(RootObject, TEXT("uiFramework"), UIFrameworkObject))
		{
			UIFrameworkObject->TryGetBoolField(TEXT("singleMainViewModel"), OutSchema.bSingleMainViewModel);
		}

		if (const TArray<TSharedPtr<FJsonValue>>* BindWidgetsArray = nullptr; TryGetArrayField(RootObject, TEXT("bindWidgets"), BindWidgetsArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *BindWidgetsArray)
			{
				FString WidgetName;
				if (Value.IsValid() && Value->TryGetString(WidgetName) && !WidgetName.IsEmpty())
				{
					OutSchema.BindWidgets.Add(WidgetName);
				}
			}
		}

		if (const TArray<TSharedPtr<FJsonValue>>* CallbacksArray = nullptr; TryGetArrayField(RootObject, TEXT("callbacks"), CallbacksArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CallbacksArray)
			{
				const TSharedPtr<FJsonObject>* CallbackObject = nullptr;
				if (!Value.IsValid() || !Value->TryGetObject(CallbackObject) || CallbackObject == nullptr || !CallbackObject->IsValid())
				{
					continue;
				}

				FManifestCallback Callback;
				(*CallbackObject)->TryGetStringField(TEXT("widget"), Callback.Widget);
				(*CallbackObject)->TryGetStringField(TEXT("function"), Callback.Function);
				(*CallbackObject)->TryGetStringField(TEXT("widgetPath"), Callback.WidgetPath);
				if (!Callback.Widget.IsEmpty() && !Callback.Function.IsEmpty())
				{
					OutSchema.Callbacks.Add(MoveTemp(Callback));
				}
			}
		}

		if (const TArray<TSharedPtr<FJsonValue>>* FieldsArray = nullptr; TryGetArrayField(RootObject, TEXT("viewModelFields"), FieldsArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *FieldsArray)
			{
				const TSharedPtr<FJsonObject>* FieldObject = nullptr;
				if (!Value.IsValid() || !Value->TryGetObject(FieldObject) || FieldObject == nullptr || !FieldObject->IsValid())
				{
					continue;
				}

				FManifestViewModelField Field;
				(*FieldObject)->TryGetStringField(TEXT("widget"), Field.Widget);
				(*FieldObject)->TryGetStringField(TEXT("field"), Field.Field);
				(*FieldObject)->TryGetStringField(TEXT("widgetPath"), Field.WidgetPath);
				if (!Field.Widget.IsEmpty() && !Field.Field.IsEmpty())
				{
					OutSchema.ViewModelFields.Add(MoveTemp(Field));
				}
			}
		}

		if (const TArray<TSharedPtr<FJsonValue>>* AnnotatedArray = nullptr; TryGetArrayField(RootObject, TEXT("annotatedWidgets"), AnnotatedArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *AnnotatedArray)
			{
				const TSharedPtr<FJsonObject>* AnnotatedObject = nullptr;
				if (!Value.IsValid() || !Value->TryGetObject(AnnotatedObject) || AnnotatedObject == nullptr || !AnnotatedObject->IsValid())
				{
					continue;
				}

				FManifestAnnotatedWidget Annotated;
				(*AnnotatedObject)->TryGetStringField(TEXT("widget"), Annotated.Widget);
				(*AnnotatedObject)->TryGetStringField(TEXT("widgetClass"), Annotated.WidgetClass);
				(*AnnotatedObject)->TryGetStringField(TEXT("widgetPath"), Annotated.WidgetPath);
				(*AnnotatedObject)->TryGetStringField(TEXT("callback"), Annotated.Callback);
				(*AnnotatedObject)->TryGetStringField(TEXT("viewModelField"), Annotated.ViewModelField);
				(*AnnotatedObject)->TryGetStringField(TEXT("semanticRole"), Annotated.SemanticRole);
				(*AnnotatedObject)->TryGetStringField(TEXT("notes"), Annotated.Notes);
				(*AnnotatedObject)->TryGetBoolField(TEXT("bindWidget"), Annotated.bBindWidget);

				if (!Annotated.Widget.IsEmpty())
				{
					OutSchema.AnnotatedWidgets.Add(MoveTemp(Annotated));
				}
			}
		}

		return true;
	}

	static FResolvedNames ResolveNames(const FManifestSchema& Manifest, const FString& SourceRoot)
	{
		FResolvedNames Names;
		Names.ScreenStem = SanitizeIdentifier(StripPrefixIfPresent(Manifest.WidgetBlueprint, TEXT("WBP_")));
		Names.StoreStem = StripTrailingSuffixIfPresent(Names.ScreenStem, TEXT("Screen"));
		Names.StoreStem = StripTrailingSuffixIfPresent(Names.StoreStem, TEXT("Dialog"));
		if (Names.StoreStem.IsEmpty())
		{
			Names.StoreStem = Names.ScreenStem;
		}

		Names.ScreenClassName = FString::Printf(TEXT("U%s"), *Names.ScreenStem);
		Names.ScreenFileStem = Names.ScreenStem;
		Names.ViewModelClassName = FString::Printf(TEXT("UVM_%s"), *Names.ScreenStem);
		Names.ViewModelFileStem = FString::Printf(TEXT("VM_%s"), *Names.ScreenStem);
		Names.StoreClassName = FString::Printf(TEXT("U%sUIStore"), *Names.StoreStem);
		Names.StoreFileStem = FString::Printf(TEXT("%sUIStore"), *Names.StoreStem);
		Names.ModuleName = ResolveModuleNameFromSourceRoot(SourceRoot);
		Names.ApiMacro = ToApiMacro(Names.ModuleName);
		return Names;
	}

	static TArray<FWidgetBindingInfo> CollectBindWidgets(const FManifestSchema& Manifest)
	{
		TMap<FString, FWidgetBindingInfo> WidgetsByName;

		for (const FManifestAnnotatedWidget& AnnotatedWidget : Manifest.AnnotatedWidgets)
		{
			if (!AnnotatedWidget.bBindWidget)
			{
				continue;
			}

			FWidgetBindingInfo Info;
			Info.WidgetName = SanitizeIdentifier(AnnotatedWidget.Widget);
			Info.WidgetClassName = ResolveWidgetClassName(AnnotatedWidget.WidgetClass);
			Info.WidgetPath = AnnotatedWidget.WidgetPath;
			Info.SemanticRole = AnnotatedWidget.SemanticRole;
			WidgetsByName.Add(Info.WidgetName, MoveTemp(Info));
		}

		for (const FString& BindWidgetName : Manifest.BindWidgets)
		{
			const FString SanitizedName = SanitizeIdentifier(BindWidgetName);
			if (!WidgetsByName.Contains(SanitizedName))
			{
				FWidgetBindingInfo Info;
				Info.WidgetName = SanitizedName;
				Info.WidgetClassName = TEXT("UWidget");
				WidgetsByName.Add(SanitizedName, MoveTemp(Info));
			}
		}

		TArray<FWidgetBindingInfo> CollectedWidgets;
		WidgetsByName.GenerateValueArray(CollectedWidgets);
		CollectedWidgets.Sort([](const FWidgetBindingInfo& Left, const FWidgetBindingInfo& Right)
		{
			return Left.WidgetName < Right.WidgetName;
		});
		return CollectedWidgets;
	}

	static FString BuildStoreHeader(
		const FResolvedNames& Names)
	{
		return FString::Printf(TEXT(
R"(#pragma once

#include "CoreMinimal.h"
#include "Stores/MyUIStoreBase.h"
#include "%s.generated.h"

UCLASS(BlueprintType)
class %s %s : public UUIStoreBase
{
	GENERATED_BODY()

public:
	virtual void BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState) override;
	virtual void UnbindFromPlayerContext() override;

	void BindBattleSources();
	void UnbindBattleSources();
};
)"),
			*Names.StoreFileStem,
			*Names.ApiMacro,
			*Names.StoreClassName);
	}

	static FString BuildStoreCpp(const FResolvedNames& Names)
	{
		return FString::Printf(TEXT(
R"(#include "Stores/%s.h"

void %s::BindToPlayerContext(APawn* InPawn, APlayerState* InPlayerState)
{
	Super::BindToPlayerContext(InPawn, InPlayerState);
	BindBattleSources();
	BroadcastStoreChanged();
}

void %s::UnbindFromPlayerContext()
{
	UnbindBattleSources();
	Super::UnbindFromPlayerContext();
	BroadcastStoreChanged();
}

void %s::BindBattleSources()
{
	// TODO: Subscribe to UHoyoBattleFlowSubsystem and any battle-specific gameplay data sources.
}

void %s::UnbindBattleSources()
{
	// TODO: Remove battle-flow and gameplay data subscriptions.
}
)"),
			*Names.StoreFileStem,
			*Names.StoreClassName,
			*Names.StoreClassName,
			*Names.StoreClassName,
			*Names.StoreClassName);
	}

	static FString BuildViewModelHeader(
		const FResolvedNames& Names,
		const TArray<FManifestViewModelField>& ViewModelFields)
	{
		FString Declarations;
		FString Properties;

		for (const FManifestViewModelField& Field : ViewModelFields)
		{
			const FString FieldName = MakeViewModelFieldPropertyName(Field.Field);
			Declarations += FString::Printf(TEXT("\tFText Get%s() const;\n"), *FieldName);
			Declarations += FString::Printf(TEXT("\tvoid Set%s(const FText& InValue);\n\n"), *FieldName);
			Properties += FString::Printf(TEXT(
				"\tUPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = \"true\"))\n\tFText %s;\n\n"),
				*FieldName);
		}

		return FString::Printf(TEXT(
R"(#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "%s.generated.h"

class UUIStoreBase;
class %s;

UCLASS(BlueprintType)
class %s %s : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(%s* InBattleStore);
	void Teardown();

%s
private:
	void RefreshFromStore();
	void HandleStoreChanged(UUIStoreBase* ChangedStore);

%s	TWeakObjectPtr<%s> BattleStore;
};
)"),
			*Names.ViewModelFileStem,
			*Names.StoreClassName,
			*Names.ApiMacro,
			*Names.ViewModelClassName,
			*Names.StoreClassName,
			*Declarations,
			*Properties,
			*Names.StoreClassName);
	}

	static FString BuildViewModelCpp(
		const FResolvedNames& Names,
		const TArray<FManifestViewModelField>& ViewModelFields)
	{
		FString GetterSetterDefinitions;
		FString PlaceholderAssignments;

		for (const FManifestViewModelField& Field : ViewModelFields)
		{
			const FString FieldName = MakeViewModelFieldPropertyName(Field.Field);
			GetterSetterDefinitions += FString::Printf(TEXT(
R"(
FText %s::Get%s() const
{
	return %s;
}

void %s::Set%s(const FText& InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(%s, InValue);
}
)"),
				*Names.ViewModelClassName,
				*FieldName,
				*FieldName,
				*Names.ViewModelClassName,
				*FieldName,
				*FieldName);

			PlaceholderAssignments += FString::Printf(TEXT("\tSet%s(FText::GetEmpty());\n"), *FieldName);
		}

		return FString::Printf(TEXT(
R"(#include "%s.h"

#include "Stores/%s.h"
#include "Stores/MyUIStoreBase.h"

void %s::Initialize(%s* InBattleStore)
{
	if (BattleStore.Get() != InBattleStore)
	{
		if (BattleStore.IsValid())
		{
			BattleStore->OnStoreChanged().RemoveAll(this);
		}

		BattleStore = InBattleStore;
		if (BattleStore.IsValid())
		{
			BattleStore->OnStoreChanged().AddUObject(this, &%s::HandleStoreChanged);
		}
	}

	RefreshFromStore();
}

void %s::Teardown()
{
	if (BattleStore.IsValid())
	{
		BattleStore->OnStoreChanged().RemoveAll(this);
		BattleStore.Reset();
	}
}
%s
void %s::RefreshFromStore()
{
	// TODO: Pull display data from %s and project it into MVVM-friendly fields.
%s}

void %s::HandleStoreChanged(UUIStoreBase* ChangedStore)
{
	(void)ChangedStore;
	RefreshFromStore();
}
)"),
			*Names.ViewModelFileStem,
			*Names.StoreFileStem,
			*Names.ViewModelClassName,
			*Names.StoreClassName,
			*Names.ViewModelClassName,
			*Names.ViewModelClassName,
			*GetterSetterDefinitions,
			*Names.ViewModelClassName,
			*Names.StoreClassName,
			*PlaceholderAssignments,
			*Names.ViewModelClassName);
	}

	static FString BuildScreenHeader(
		const FResolvedNames& Names,
		const FBaseClassInfo& BaseClass,
		const TArray<FWidgetBindingInfo>& Widgets,
		const TArray<FManifestCallback>& Callbacks)
	{
		TSet<FString> ForwardDecls;
		for (const FWidgetBindingInfo& Widget : Widgets)
		{
			ForwardDecls.Add(Widget.WidgetClassName);
		}

		FString ForwardDeclLines;
		TArray<FString> SortedForwardDecls = ForwardDecls.Array();
		SortedForwardDecls.Sort();
		for (const FString& ForwardDecl : SortedForwardDecls)
		{
			ForwardDeclLines += FString::Printf(TEXT("class %s;\n"), *ForwardDecl);
		}
		ForwardDeclLines += FString::Printf(TEXT("class %s;\n"), *Names.ViewModelClassName);

		FString CallbackDeclarations;
		for (const FManifestCallback& Callback : Callbacks)
		{
			CallbackDeclarations += FString::Printf(TEXT("\tUFUNCTION()\n\tvoid %s();\n"), *SanitizeIdentifier(Callback.Function));
		}

		FString PropertyDeclarations;
		for (const FWidgetBindingInfo& Widget : Widgets)
		{
			PropertyDeclarations += FString::Printf(TEXT(
				"\tUPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = \"true\"))\n\tTObjectPtr<%s> %s;\n"),
				*Widget.WidgetClassName,
				*Widget.WidgetName);
			if (!Widget.SemanticRole.IsEmpty())
			{
				PropertyDeclarations += FString::Printf(TEXT("\t// SemanticRole: %s\n"), *Widget.SemanticRole);
			}
			PropertyDeclarations += TEXT("\n");
		}

		return FString::Printf(TEXT(
R"(#pragma once

#include "CoreMinimal.h"
#include "%s"
#include "%s.generated.h"

class UMyUIStoreSubsystem;
%s
UCLASS()
class %s %s : public %s
{
	GENERATED_BODY()

public:
	%s(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

protected:
	virtual UObject* CreateViewModelInstance() override;
	virtual void InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem) override;
	virtual void TeardownViewModel(UObject* ViewModel) override;

private:
%s
	%s* GetScreenViewModel() const;

%s};
)"),
			*BaseClass.IncludePath,
			*Names.ScreenFileStem,
			*ForwardDeclLines,
			*Names.ApiMacro,
			*Names.ScreenClassName,
			*BaseClass.ClassName,
			*Names.ScreenClassName,
			*CallbackDeclarations,
			*Names.ViewModelClassName,
			*PropertyDeclarations);
	}

	static FString BuildScreenCpp(
		const FResolvedNames& Names,
		const TArray<FManifestCallback>& Callbacks)
	{
		const bool bHasButtons = Callbacks.Num() > 0;

		FString NativeOnInitializedBindings;
		for (const FManifestCallback& Callback : Callbacks)
		{
			const FString FunctionName = SanitizeIdentifier(Callback.Function);
			const FString WidgetName = SanitizeIdentifier(Callback.Widget);
			NativeOnInitializedBindings += FString::Printf(TEXT(
				"\tif (%s)\n\t{\n\t\t%s->OnClicked.RemoveDynamic(this, &%s::%s);\n\t\t%s->OnClicked.AddDynamic(this, &%s::%s);\n\t}\n\n"),
				*WidgetName,
				*WidgetName,
				*Names.ScreenClassName,
				*FunctionName,
				*WidgetName,
				*Names.ScreenClassName,
				*FunctionName);
		}

		FString NativeGetDesiredFocusTargetBody = TEXT("\treturn nullptr;");
		for (const FManifestCallback& Callback : Callbacks)
		{
			const FString WidgetName = SanitizeIdentifier(Callback.Widget);
			NativeGetDesiredFocusTargetBody = FString::Printf(TEXT("\treturn %s ? Cast<UWidget>(%s) : nullptr;"), *WidgetName, *WidgetName);
			break;
		}

		FString CallbackDefinitions;
		for (const FManifestCallback& Callback : Callbacks)
		{
			const FString FunctionName = SanitizeIdentifier(Callback.Function);
			CallbackDefinitions += FString::Printf(TEXT(
R"(
void %s::%s()
{
	// TODO: Implement '%s' for widget '%s'.
}
)"),
				*Names.ScreenClassName,
				*FunctionName,
				*FunctionName,
				*Callback.Widget);
		}

		return FString::Printf(TEXT(
R"(#include "Widgets/%s.h"

%s#include "Stores/%s.h"
#include "Subsystems/MyUIStoreSubsystem.h"
#include "%s.h"

%s::%s(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreferredLayer = EMyUILayer::Menu;
}

void %s::NativeOnInitialized()
{
	Super::NativeOnInitialized();

%s}

UObject* %s::CreateViewModelInstance()
{
	return NewObject<%s>(this);
}

void %s::InitializeViewModel(UObject* ViewModel, UMyUIStoreSubsystem* StoreSubsystem)
{
	if (%s* ScreenViewModel = Cast<%s>(ViewModel))
	{
		ScreenViewModel->Initialize(StoreSubsystem ? StoreSubsystem->GetStore<%s>() : nullptr);
	}
}

void %s::TeardownViewModel(UObject* ViewModel)
{
	if (%s* ScreenViewModel = Cast<%s>(ViewModel))
	{
		ScreenViewModel->Teardown();
	}
}

UWidget* %s::NativeGetDesiredFocusTarget() const
{
%s
}

%s* %s::GetScreenViewModel() const
{
	return Cast<%s>(GetViewModelObject());
}
%s)"),
			*Names.ScreenFileStem,
			bHasButtons ? TEXT("#include \"Components/Button.h\"\n") : TEXT(""),
			*Names.StoreFileStem,
			*Names.ViewModelFileStem,
			*Names.ScreenClassName,
			*Names.ScreenClassName,
			*Names.ScreenClassName,
			*NativeOnInitializedBindings,
			*Names.ScreenClassName,
			*Names.ViewModelClassName,
			*Names.ScreenClassName,
			*Names.ViewModelClassName,
			*Names.ViewModelClassName,
			*Names.StoreClassName,
			*Names.ScreenClassName,
			*Names.ViewModelClassName,
			*Names.ViewModelClassName,
			*Names.ScreenClassName,
			*NativeGetDesiredFocusTargetBody,
			*Names.ViewModelClassName,
			*Names.ScreenClassName,
			*Names.ViewModelClassName,
			*CallbackDefinitions);
	}

	static bool WriteGeneratedFile(
		const FString& TargetPath,
		const FString& FileContents,
		const bool bOverwriteExistingFiles,
		FJsonWidgetBlueprintBoilerplateGenerationResult& Result)
	{
		if (!bOverwriteExistingFiles && FPaths::FileExists(TargetPath))
		{
			AddError(Result, FString::Printf(TEXT("Refusing to overwrite existing file '%s'."), *TargetPath));
			return false;
		}

		const FString DirectoryPath = FPaths::GetPath(TargetPath);
		IFileManager::Get().MakeDirectory(*DirectoryPath, true);

		if (!FFileHelper::SaveStringToFile(FileContents, *TargetPath))
		{
			AddError(Result, FString::Printf(TEXT("Failed to write generated file '%s'."), *TargetPath));
			return false;
		}

		Result.GeneratedFiles.Add(TargetPath);
		return true;
	}
}

FJsonWidgetBlueprintBoilerplateGenerationResult FJsonWidgetBlueprintBoilerplateGenerator::GenerateFromManifestFile(
	const FJsonWidgetBlueprintBoilerplateGenerationRequest& Request)
{
	using namespace JsonWidgetBlueprintBoilerplateGenerator;

	FJsonWidgetBlueprintBoilerplateGenerationResult Result;

	if (Request.ManifestFilePath.IsEmpty())
	{
		AddError(Result, TEXT("Manifest file path is empty."));
		return Result;
	}

	if (Request.TargetSourceRoot.IsEmpty())
	{
		AddError(Result, TEXT("Target source root is empty."));
		return Result;
	}

	const FString AbsoluteSourceRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), Request.TargetSourceRoot);
	if (!IFileManager::Get().DirectoryExists(*AbsoluteSourceRoot))
	{
		AddError(Result, FString::Printf(TEXT("Target source root '%s' does not exist."), *AbsoluteSourceRoot));
		return Result;
	}

	FString ManifestJson;
	if (!FFileHelper::LoadFileToString(ManifestJson, *Request.ManifestFilePath))
	{
		AddError(Result, FString::Printf(TEXT("Failed to read manifest file '%s'."), *Request.ManifestFilePath));
		return Result;
	}

	FManifestSchema Manifest;
	if (!ParseManifestSchema(ManifestJson, Manifest, Result))
	{
		return Result;
	}

	const FResolvedNames Names = ResolveNames(Manifest, AbsoluteSourceRoot);
	const FBaseClassInfo BaseClass = ResolveScreenBaseClass(Manifest.ParentClass);
	const TArray<FWidgetBindingInfo> Widgets = CollectBindWidgets(Manifest);

	const FString ScreenHeaderPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIFramework/Public/Widgets"), Names.ScreenFileStem + TEXT(".h"));
	const FString ScreenCppPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIFramework/Private/Widgets"), Names.ScreenFileStem + TEXT(".cpp"));
	const FString ViewModelHeaderPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIViewModels/Public"), Names.ViewModelFileStem + TEXT(".h"));
	const FString ViewModelCppPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIViewModels/Private"), Names.ViewModelFileStem + TEXT(".cpp"));
	const FString StoreHeaderPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIFramework/Public/Stores"), Names.StoreFileStem + TEXT(".h"));
	const FString StoreCppPath = FPaths::Combine(AbsoluteSourceRoot, TEXT("UIFramework/Private/Stores"), Names.StoreFileStem + TEXT(".cpp"));

	const FString ScreenHeaderContents = BuildScreenHeader(
		Names,
		BaseClass,
		Widgets,
		Manifest.Callbacks);
	const FString ScreenCppContents = BuildScreenCpp(Names, Manifest.Callbacks);
	const FString ViewModelHeaderContents = BuildViewModelHeader(
		Names,
		Manifest.ViewModelFields);
	const FString ViewModelCppContents = BuildViewModelCpp(Names, Manifest.ViewModelFields);
	const FString StoreHeaderContents = BuildStoreHeader(Names);
	const FString StoreCppContents = BuildStoreCpp(Names);

	if (!WriteGeneratedFile(ScreenHeaderPath, ScreenHeaderContents, Request.bOverwriteExistingFiles, Result) ||
		!WriteGeneratedFile(ScreenCppPath, ScreenCppContents, Request.bOverwriteExistingFiles, Result) ||
		!WriteGeneratedFile(ViewModelHeaderPath, ViewModelHeaderContents, Request.bOverwriteExistingFiles, Result) ||
		!WriteGeneratedFile(ViewModelCppPath, ViewModelCppContents, Request.bOverwriteExistingFiles, Result) ||
		!WriteGeneratedFile(StoreHeaderPath, StoreHeaderContents, Request.bOverwriteExistingFiles, Result) ||
		!WriteGeneratedFile(StoreCppPath, StoreCppContents, Request.bOverwriteExistingFiles, Result))
	{
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
