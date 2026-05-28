#pragma once

#include "CoreMinimal.h"
#include "Core/OrigamiBirdMatchTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "OrigamiBirdMatchSubsystem.generated.h"

class UDataTable;
class UOrigamiBirdMatchGameObject;

// Origami Bird Match entry point.
// Owns data-table access and the currently running match. Board rules stay in UOrigamiBirdMatchGameObject.
UCLASS()
class HOYOGAS_API UOrigamiBirdMatchSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* StartDefaultDevelopmentMatch();

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* StartMatchByLevelId(FName LevelId);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void EndActiveMatch();

	UFUNCTION(BlueprintPure, Category = "OrigamiBird")
	UOrigamiBirdMatchGameObject* GetActiveMatch() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindLevelDefinition(FName LevelId, FOrigamiBirdLevelDefinitionRow& OutDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindTileDefinition(EOrigamiBirdTileType TileType, FOrigamiBirdTileDefinitionRow& OutDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllTileDefinitions(TArray<FOrigamiBirdTileDefinitionRow>& OutDefinitions) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllLevelIds(TArray<FName>& OutLevelIds) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	bool FindPropDefinition(FName PropId, FOrigamiBirdPropDefinitionRow& OutDefinition) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllPropIds(TArray<FName>& OutPropIds) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	void GetAllPropDefinitions(TArray<FOrigamiBirdPropDefinitionRow>& OutDefinitions) const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool GrantPropToActiveMatch(FName PropId, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool ConsumePropFromActiveMatch(FName PropId, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird|Props")
	bool UsePropOnActiveMatch(const FOrigamiBirdPropUseRequest& Request, FOrigamiBirdPropUseResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UDataTable* LoadTileDefinitionTable() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UDataTable* LoadLevelDefinitionTable() const;

	UFUNCTION(BlueprintCallable, Category = "OrigamiBird")
	UDataTable* LoadPropDefinitionTable() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UOrigamiBirdMatchGameObject> ActiveMatch;
};
