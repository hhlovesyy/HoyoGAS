#pragma once

#include "CoreMinimal.h"
#include "UIFrameworkTypes.generated.h"

class UMyScreenBase;

UENUM(BlueprintType)
enum class EMyUILayer : uint8
{
	Game,
	Menu,
	Modal,
	Toast
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FMyUIPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FName ContextTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	int32 IntValue = 0;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FMyUIScreenConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FName ScreenTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	TSoftClassPtr<UMyScreenBase> ScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	EMyUILayer DefaultLayer = EMyUILayer::Game;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	bool bSingletonPerPlayer = false;
};

USTRUCT(BlueprintType)
struct HOYOGAS_API FMyConfirmDialogConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FText ConfirmText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIFramework")
	FText CancelText;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FMyDialogResultDelegate, bool, bConfirmed);
