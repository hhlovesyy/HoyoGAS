#pragma once

#include "CoreMinimal.h"

class FWidgetBlueprintEditor;
class UWidget;
class UWidgetBlueprint;

namespace JsonWidgetBlueprintSemantics
{
	extern const FName CallbackNameMetaDataKey;
	extern const FName NotesMetaDataKey;
	extern const FName SemanticRoleMetaDataKey;
	extern const FName ViewModelFieldMetaDataKey;

	bool GetWidgetMetadataValue(const UWidget* Widget, FName Key, FString& OutValue);
	bool HasWidgetMetadataValue(const UWidget* Widget, FName Key);
	void SetWidgetMetadataValue(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, FName Key, const FString& Value);

	bool GetBindWidget(const UWidget* Widget);
	void SetBindWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, bool bBindWidget);

	FString GetWidgetKind(const UWidgetBlueprint* WidgetBlueprint);
	FString GetSuggestedManifestOutputPath(const UWidgetBlueprint* WidgetBlueprint);
	bool ExportManifestWithDialog(const TSharedPtr<FWidgetBlueprintEditor>& WidgetBlueprintEditor);
}
