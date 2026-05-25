#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_CharacterStoryEntry.generated.h"

UCLASS(BlueprintType)
class HOYOGAS_API UVM_CharacterStoryEntry : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	FName GetStoryId() const;
	void SetStoryId(FName InValue);

	int32 GetStoryIndex() const;
	void SetStoryIndex(int32 InValue);

	FText GetTitleText() const;
	void SetTitleText(const FText& InValue);

	bool GetIsUnlocked() const;
	void SetIsUnlocked(bool bInValue);

	bool GetIsSelected() const;
	void SetIsSelected(bool bInValue);

protected:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FName StoryId = NAME_None;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	int32 StoryIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Setter, meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetIsUnlocked", Setter = "SetIsUnlocked", meta = (AllowPrivateAccess = "true"))
	bool bIsUnlocked = true;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter = "GetIsSelected", Setter = "SetIsSelected", meta = (AllowPrivateAccess = "true"))
	bool bIsSelected = false;
};
