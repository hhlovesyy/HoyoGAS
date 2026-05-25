#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayDemoStatics.generated.h"

struct FItemDefinitionRow;
struct FLevelProgressionRow;

UCLASS()
class HOYOGAS_API UGameplayDemoStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, FItemDefinitionRow& OutRow);
	static void GetAllItemDefinitions(const UObject* WorldContextObject, TArray<FItemDefinitionRow>& OutRows);
	static void GetAllLevelProgressionRows(const UObject* WorldContextObject, TArray<FLevelProgressionRow>& OutRows);
	static int32 GetItemScoreValue(const UObject* WorldContextObject, FName ItemId);
};
