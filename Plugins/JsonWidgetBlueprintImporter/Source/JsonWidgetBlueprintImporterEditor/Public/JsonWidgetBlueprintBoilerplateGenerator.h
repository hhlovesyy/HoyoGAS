#pragma once

#include "CoreMinimal.h"

struct FJsonWidgetBlueprintBoilerplateGenerationRequest
{
	FString ManifestFilePath;
	FString TargetSourceRoot = TEXT("Source/HoyoGas");
	bool bOverwriteExistingFiles = false;
};

struct FJsonWidgetBlueprintBoilerplateGenerationResult
{
	bool bSuccess = false;
	TArray<FString> GeneratedFiles;
	TArray<FString> Warnings;
	TArray<FString> Errors;
};

class FJsonWidgetBlueprintBoilerplateGenerator
{
public:
	static FJsonWidgetBlueprintBoilerplateGenerationResult GenerateFromManifestFile(
		const FJsonWidgetBlueprintBoilerplateGenerationRequest& Request);
};
