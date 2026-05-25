#pragma once

#include "CoreMinimal.h"

class UUserWidget;

struct FJsonWidgetBlueprintImportRequest
{
	FString JsonFilePath;
	FString TargetContentFolder = TEXT("/Game/UI/Generated");
	bool bOpenBlueprintAfterImport = true;
};

struct FJsonWidgetBlueprintImportResult
{
	bool bSuccess = false;
	FString AssetPath;
	TArray<FString> Warnings;
	TArray<FString> Errors;
};

class FJsonWidgetBlueprintImporter
{
public:
	static FJsonWidgetBlueprintImportResult ImportFromFile(const FJsonWidgetBlueprintImportRequest& Request);

private:
	static UClass* ResolveParentClass(const FString& ParentClassText, FJsonWidgetBlueprintImportResult& Result);
};
