// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MotionPreviewClient.h"
#include "MotionPreviewPanelWidget.generated.h"

class UButton;
class UEditorUtilityEditableText;
class UEditorUtilitySpinBox;
class UTextBlock;
class UAnimSequence;
class UIKRetargeter;
class USkeleton;

UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UMotionPreviewPanelWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config", meta = (DisplayName = "Ebx_ExpName"))
	FString FixedExpName = TEXT("Varsapura");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config", meta = (DisplayName = "Spn_MotionIndex"))
	int32 FixedMotionIndex = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config")
	FString FixedServerBaseUrl = TEXT("http://127.0.0.1:8000");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config", meta = (DisplayName = "Ebx_DownloadDir_1"))
	FString FixedDownloadDir = TEXT("D:/GraduateStudent/Research/2026_03/0331/motion/fbx_out");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config", meta = (DisplayName = "Ebx_ImportPath_1"))
	FString FixedImportPath_1 = TEXT("/Game/MotionBigPaper/Phase1/ImportedAnimations");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Config", meta = (DisplayName = "Ebx_ImportScale_1"))
	FString FixedImportScale_1 = TEXT("0.01");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Import", meta = (DisplayName = "Snap To Closest Frame Boundary"))
	bool bFixedSnapToClosestFrameBoundary = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Import", meta = (DisplayName = "Use Default Sample Rate"))
	bool bFixedUseDefaultSampleRate = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Import", meta = (DisplayName = "Custom Sample Rate", EditCondition = "!bFixedUseDefaultSampleRate", ClampMin = "0", UIMin = "0"))
	int32 FixedCustomSampleRate = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Import", meta = (DisplayName = "Import Skeleton"))
	TSoftObjectPtr<USkeleton> FixedImportSkeleton = TSoftObjectPtr<USkeleton>(FSoftObjectPath(TEXT("/Game/MotionBigPaper/Phase1/SMPLHWithAPose/SMPLH_Skeleton.SMPLH_Skeleton")));

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Retarget", meta = (DisplayName = "Retarget Output Path"))
	FString FixedRetargetOutputPath = TEXT("/Game/MotionBigPaper/Phase1/RetargetedAnimations");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Preview Retarget", meta = (DisplayName = "IK Retargeter"))
	TSoftObjectPtr<UIKRetargeter> FixedIKRetargeter = TSoftObjectPtr<UIKRetargeter>(FSoftObjectPath(TEXT("/Game/MotionBigPaper/Phase1/SMPLHWithAPose/RTG_NewRetargeter.RTG_NewRetargeter")));

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void HandleRunFixedTestClicked();

	UFUNCTION()
	void HandleImportDownloadedFbxClicked();

	UFUNCTION()
	void HandleRetargetImportedAnimClicked();

	void ApplyFixedConfigToWidgets();
	void PullWidgetValuesToFixedConfig();
	void SyncClientFixedConfig() const;
	void SetToolFeedback(const FString& InStatusMessage, const FString& InErrorMessage = FString());
	bool TryGetReadyDownloadedFbxPath(FString& OutFbxPath) const;
	bool IsValidContentBrowserPath(const FString& InPath) const;
	UAnimSequence* FindImportedAnimSequenceByName(const FString& InPackagePath, const FString& InAssetName) const;
	void RefreshTexts();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_RunFixedTest;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_ImportDownloadedFbx;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_RetargetImportedAnim;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditorUtilityEditableText> Ebx_ExpName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditorUtilityEditableText> Ebx_DownloadDir_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditorUtilityEditableText> Ebx_ImportPath_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditorUtilityEditableText> Ebx_ImportScale_1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditorUtilitySpinBox> Spn_MotionIndex;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Status;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Stage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Message;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Elapsed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_File;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Error;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ErrorDetail;

	UPROPERTY(Transient)
	TObjectPtr<UMotionPreviewClient> Client;

	UPROPERTY(Transient)
	bool bPendingApplyFixedConfigToWidgets = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> LastImportedAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> LastRetargetedAnimation;

	UPROPERTY(Transient)
	FString ToolStatusMessage;

	UPROPERTY(Transient)
	FString ToolErrorMessage;
};
