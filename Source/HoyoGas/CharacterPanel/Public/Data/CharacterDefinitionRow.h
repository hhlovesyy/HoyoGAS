#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Progression/HoyoCheckTypes.h"
#include "CharacterDefinitionRow.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct HOYOGAS_API FCharacterStoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FName StoryId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel", meta = (MultiLine = "true"))
	FText RichTextBody; //multiline=true意味着：开启这个选项后，输入框会变成一个带有滚动条的文本编辑框，方便用户输入和查看多行文字。

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FHoyoCheckConditionSet UnlockConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText LockedReasonText;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FCharacterDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FName CharacterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText PathName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FGameplayTag PathTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText FactionName; //这里应该指的是阵营

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText ElementName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	FText FirstMetDateText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Rarity = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel", meta = (ClampMin = "1"))
	int32 DefaultLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel", meta = (ClampMin = "1"))
	int32 MaxLevel = 80;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	TSoftObjectPtr<UTexture2D> PortraitTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel", meta = (MultiLine = "true"))
	FText ShortBio; //应该是角色简介之类的

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterPanel")
	TArray<FCharacterStoryEntry> Stories;
};
