#include "MotionPreviewPanelWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "EditorFramework/AssetImportData.h"
#include "EditorUtilityWidgetComponents.h"
#include "Factories/FbxAnimSequenceImportData.h"
#include "Factories/FbxImportUI.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "RetargetEditor/IKRetargetBatchOperation.h"
#include "Retargeter/IKRetargeter.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetImportTask.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"

namespace
{
	constexpr float DefaultImportScale = 0.01f;
}

void UMotionPreviewPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Client)
	{
		Client = NewObject<UMotionPreviewClient>(this);
	}

	bPendingApplyFixedConfigToWidgets = true;
	SyncClientFixedConfig();

	if (Btn_RunFixedTest)
	{
		Btn_RunFixedTest->OnClicked.RemoveAll(this);
		Btn_RunFixedTest->OnClicked.AddDynamic(this, &UMotionPreviewPanelWidget::HandleRunFixedTestClicked);
	}

	if (Btn_ImportDownloadedFbx)
	{
		Btn_ImportDownloadedFbx->OnClicked.RemoveAll(this);
		Btn_ImportDownloadedFbx->OnClicked.AddDynamic(this, &UMotionPreviewPanelWidget::HandleImportDownloadedFbxClicked);
	}

	if (Btn_RetargetImportedAnim)
	{
		Btn_RetargetImportedAnim->OnClicked.RemoveAll(this);
		Btn_RetargetImportedAnim->OnClicked.AddDynamic(this, &UMotionPreviewPanelWidget::HandleRetargetImportedAnimClicked);
	}

	RefreshTexts();
}

void UMotionPreviewPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bPendingApplyFixedConfigToWidgets)
	{
		ApplyFixedConfigToWidgets();
		bPendingApplyFixedConfigToWidgets = false;
	}

	RefreshTexts();
}

void UMotionPreviewPanelWidget::HandleRunFixedTestClicked()
{
	if (!Client)
	{
		Client = NewObject<UMotionPreviewClient>(this);
	}

	if (!Client)
	{
		SetToolFeedback(TEXT("client create failed"), TEXT("Failed to create MotionPreviewClient."));
		return;
	}

	PullWidgetValuesToFixedConfig();
	SyncClientFixedConfig();
	Client->StartJob(FixedExpName, FixedMotionIndex);
	SetToolFeedback(TEXT("job submitted"));
	RefreshTexts();
}

void UMotionPreviewPanelWidget::HandleImportDownloadedFbxClicked()
{
	PullWidgetValuesToFixedConfig();

	FString DownloadedFbxPath;
	if (!TryGetReadyDownloadedFbxPath(DownloadedFbxPath))
	{
		return;
	}

	if (!IsValidContentBrowserPath(FixedImportPath_1))
	{
		SetToolFeedback(TEXT("import skipped"), FString::Printf(TEXT("Invalid import path: %s"), *FixedImportPath_1));
		return;
	}

	USkeleton* ImportSkeleton = FixedImportSkeleton.LoadSynchronous();
	if (!IsValid(ImportSkeleton))
	{
		SetToolFeedback(TEXT("import skipped"), TEXT("Import Skeleton is null or failed to load."));
		return;
	}

	float ImportScale = DefaultImportScale;
	if (!FixedImportScale_1.IsEmpty())
	{
		if (!LexTryParseString(ImportScale, *FixedImportScale_1))
		{
			SetToolFeedback(TEXT("import skipped"), FString::Printf(TEXT("Import scale is invalid: %s"), *FixedImportScale_1));
			return;
		}
	}

	UAssetImportTask* ImportTask = NewObject<UAssetImportTask>(this);
	UFbxImportUI* ImportOptions = NewObject<UFbxImportUI>(ImportTask);
	if (!ImportTask || !ImportOptions)
	{
		SetToolFeedback(TEXT("import skipped"), TEXT("Failed to create FBX import task."));
		return;
	}

	if (!ImportOptions->AnimSequenceImportData)
	{
		ImportOptions->AnimSequenceImportData = NewObject<UFbxAnimSequenceImportData>(ImportOptions);
	}

	ImportTask->Filename = DownloadedFbxPath;
	ImportTask->DestinationPath = FixedImportPath_1;
	ImportTask->DestinationName = FPaths::GetBaseFilename(DownloadedFbxPath);
	ImportTask->bReplaceExisting = true;
	ImportTask->bReplaceExistingSettings = true;
	ImportTask->bAutomated = true;
	ImportTask->bSave = true;
	ImportTask->bAsync = false;
	ImportTask->Options = ImportOptions;

	ImportOptions->OriginalImportType = FBXIT_Animation;
	ImportOptions->MeshTypeToImport = FBXIT_Animation;
	ImportOptions->bAutomatedImportShouldDetectType = false;
	ImportOptions->bImportAsSkeletal = true;
	ImportOptions->bImportMesh = false;
	ImportOptions->Skeleton = ImportSkeleton;
	ImportOptions->bImportAnimations = true;
	ImportOptions->bImportMaterials = false;
	ImportOptions->bImportTextures = false;

	ImportOptions->AnimSequenceImportData->ImportUniformScale = ImportScale;
	ImportOptions->AnimSequenceImportData->bImportBoneTracks = true;
	ImportOptions->AnimSequenceImportData->bSnapToClosestFrameBoundary = bFixedSnapToClosestFrameBoundary;
	ImportOptions->AnimSequenceImportData->bUseDefaultSampleRate = bFixedUseDefaultSampleRate;
	ImportOptions->AnimSequenceImportData->CustomSampleRate = FixedCustomSampleRate;

	TArray<UAssetImportTask*> Tasks;
	Tasks.Add(ImportTask);
	FAssetToolsModule::GetModule().Get().ImportAssetTasks(Tasks);

	UAnimSequence* ImportedAnimSequence = nullptr;
	for (UObject* ImportedObject : ImportTask->GetObjects())
	{
		if (UAnimSequence* ImportedAnim = Cast<UAnimSequence>(ImportedObject))
		{
			ImportedAnimSequence = ImportedAnim;
			break;
		}
	}

	if (!ImportedAnimSequence)
	{
		ImportedAnimSequence = FindImportedAnimSequenceByName(FixedImportPath_1, ImportTask->DestinationName);
	}

	if (!ImportedAnimSequence)
	{
		SetToolFeedback(TEXT("import finished"), TEXT("FBX import did not produce an Animation Sequence."));
		return;
	}

	LastImportedAnimation = ImportedAnimSequence;
	SetToolFeedback(FString::Printf(TEXT("imported animation: %s"), *ImportedAnimSequence->GetName()));
}

void UMotionPreviewPanelWidget::HandleRetargetImportedAnimClicked()
{
	if (!IsValid(LastImportedAnimation))
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("No imported Animation Sequence is ready. Import the downloaded FBX first."));
		return;
	}

	if (!IsValidContentBrowserPath(FixedRetargetOutputPath))
	{
		SetToolFeedback(TEXT("retarget skipped"), FString::Printf(TEXT("Invalid retarget output path: %s"), *FixedRetargetOutputPath));
		return;
	}

	UIKRetargeter* Retargeter = FixedIKRetargeter.LoadSynchronous();
	if (!IsValid(Retargeter))
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("IK Retargeter is null or failed to load."));
		return;
	}

	USkeletalMesh* SourceMesh = Retargeter->GetPreviewMesh(ERetargetSourceOrTarget::Source);
	USkeletalMesh* TargetMesh = Retargeter->GetPreviewMesh(ERetargetSourceOrTarget::Target);
	if (!IsValid(SourceMesh) || !IsValid(TargetMesh))
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("Retargeter preview meshes are missing."));
		return;
	}

	USkeleton* SourceSkeleton = SourceMesh->GetSkeleton();
	if (!IsValid(SourceSkeleton) || !IsValid(LastImportedAnimation->GetSkeleton()))
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("Source skeleton is invalid."));
		return;
	}

	if (LastImportedAnimation->GetSkeleton() != SourceSkeleton)
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("Imported animation skeleton does not match the retarget source mesh skeleton."));
		return;
	}

	FIKRetargetBatchOperationContext BatchContext;
	BatchContext.AssetsToRetarget.Add(LastImportedAnimation);
	BatchContext.SourceMesh = SourceMesh;
	BatchContext.TargetMesh = TargetMesh;
	BatchContext.IKRetargetAsset = Retargeter;
	BatchContext.NameRule.FolderPath = FixedRetargetOutputPath;
	BatchContext.bOverwriteExistingFiles = true;
	BatchContext.bIncludeReferencedAssets = false;

	if (!BatchContext.IsValid())
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("Retarget batch context is invalid."));
		return;
	}

	UIKRetargetBatchOperation* BatchOperation = NewObject<UIKRetargetBatchOperation>(this);
	if (!BatchOperation)
	{
		SetToolFeedback(TEXT("retarget skipped"), TEXT("Failed to create IK retarget batch operation."));
		return;
	}

	BatchOperation->RunRetarget(BatchContext);

	UAnimSequence* RetargetedAnimSequence = FindImportedAnimSequenceByName(FixedRetargetOutputPath, LastImportedAnimation->GetName());
	if (!RetargetedAnimSequence)
	{
		SetToolFeedback(TEXT("retarget finished"), TEXT("Retarget completed but the resulting Animation Sequence was not found."));
		return;
	}

	LastRetargetedAnimation = RetargetedAnimSequence;
	SetToolFeedback(FString::Printf(TEXT("retargeted animation: %s"), *RetargetedAnimSequence->GetName()));

	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditorSubsystem->OpenEditorForAsset(RetargetedAnimSequence);
		}
	}
}

void UMotionPreviewPanelWidget::ApplyFixedConfigToWidgets()
{
	if (IsValid(Ebx_ExpName))
	{
		Ebx_ExpName->SetText(FText::FromString(FixedExpName));
	}

	if (IsValid(Ebx_DownloadDir_1))
	{
		Ebx_DownloadDir_1->SetText(FText::FromString(FixedDownloadDir));
	}

	if (IsValid(Ebx_ImportPath_1))
	{
		Ebx_ImportPath_1->SetText(FText::FromString(FixedImportPath_1));
	}

	if (IsValid(Ebx_ImportScale_1))
	{
		Ebx_ImportScale_1->SetText(FText::FromString(FixedImportScale_1));
	}

	if (IsValid(Spn_MotionIndex))
	{
		Spn_MotionIndex->SetValue(static_cast<float>(FixedMotionIndex));
	}
}

void UMotionPreviewPanelWidget::PullWidgetValuesToFixedConfig()
{
	if (IsValid(Ebx_ExpName))
	{
		FixedExpName = Ebx_ExpName->GetText().ToString();
	}

	if (IsValid(Ebx_DownloadDir_1))
	{
		FixedDownloadDir = Ebx_DownloadDir_1->GetText().ToString();
	}

	if (IsValid(Ebx_ImportPath_1))
	{
		FixedImportPath_1 = Ebx_ImportPath_1->GetText().ToString();
	}

	if (IsValid(Ebx_ImportScale_1))
	{
		FixedImportScale_1 = Ebx_ImportScale_1->GetText().ToString();
	}

	if (IsValid(Spn_MotionIndex))
	{
		FixedMotionIndex = FMath::RoundToInt(Spn_MotionIndex->GetValue());
	}
}

void UMotionPreviewPanelWidget::SyncClientFixedConfig() const
{
	if (!Client)
	{
		return;
	}

	Client->ServerBaseUrl = FixedServerBaseUrl;
	Client->DownloadDir = FixedDownloadDir;
}

void UMotionPreviewPanelWidget::SetToolFeedback(const FString& InStatusMessage, const FString& InErrorMessage)
{
	ToolStatusMessage = InStatusMessage;
	ToolErrorMessage = InErrorMessage;
}

bool UMotionPreviewPanelWidget::TryGetReadyDownloadedFbxPath(FString& OutFbxPath) const
{
	OutFbxPath.Reset();

	if (!Client)
	{
		const_cast<UMotionPreviewPanelWidget*>(this)->SetToolFeedback(TEXT("import skipped"), TEXT("MotionPreviewClient is not ready."));
		return false;
	}

	if (Client->bIsBusy)
	{
		const_cast<UMotionPreviewPanelWidget*>(this)->SetToolFeedback(TEXT("import skipped"), TEXT("The download job is still running."));
		return false;
	}

	if (Client->LastDownloadedFilePath.IsEmpty())
	{
		const_cast<UMotionPreviewPanelWidget*>(this)->SetToolFeedback(TEXT("import skipped"), TEXT("No downloaded FBX file is available yet."));
		return false;
	}

	if (!IFileManager::Get().FileExists(*Client->LastDownloadedFilePath))
	{
		const_cast<UMotionPreviewPanelWidget*>(this)->SetToolFeedback(TEXT("import skipped"), FString::Printf(TEXT("Downloaded file does not exist: %s"), *Client->LastDownloadedFilePath));
		return false;
	}

	OutFbxPath = Client->LastDownloadedFilePath;
	return true;
}

bool UMotionPreviewPanelWidget::IsValidContentBrowserPath(const FString& InPath) const
{
	return !InPath.IsEmpty() && FPackageName::IsValidLongPackageName(InPath) && InPath.StartsWith(TEXT("/Game"));
}

UAnimSequence* UMotionPreviewPanelWidget::FindImportedAnimSequenceByName(const FString& InPackagePath, const FString& InAssetName) const
{
	if (!IsValidContentBrowserPath(InPackagePath) || InAssetName.IsEmpty())
	{
		return nullptr;
	}

	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), *InPackagePath, *InAssetName, *InAssetName);
	if (UAnimSequence* DirectLoad = LoadObject<UAnimSequence>(nullptr, *ObjectPath))
	{
		return DirectLoad;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.PackagePaths.Add(*InPackagePath);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = false;

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);

	for (const FAssetData& Asset : Assets)
	{
		if (Asset.AssetName.ToString() == InAssetName)
		{
			return Cast<UAnimSequence>(Asset.GetAsset());
		}
	}

	return nullptr;
}

void UMotionPreviewPanelWidget::RefreshTexts()
{
	if (!Client)
	{
		if (Txt_Status)
		{
			Txt_Status->SetText(FText::FromString(TEXT("Status: (client null)")));
		}
		if (Txt_Stage)
		{
			Txt_Stage->SetText(FText::FromString(TEXT("Stage:")));
		}
		if (Txt_Message)
		{
			const FString MessageText = ToolStatusMessage.IsEmpty() ? TEXT("Message:") : FString::Printf(TEXT("Message: %s"), *ToolStatusMessage);
			Txt_Message->SetText(FText::FromString(MessageText));
		}
		if (Txt_Elapsed)
		{
			Txt_Elapsed->SetText(FText::FromString(TEXT("Elapsed:")));
		}
		if (Txt_File)
		{
			Txt_File->SetText(FText::FromString(TEXT("File:")));
		}
		if (Txt_Error)
		{
			const FString ErrorText = ToolErrorMessage.IsEmpty() ? TEXT("Error:") : FString::Printf(TEXT("Error: %s"), *ToolErrorMessage);
			Txt_Error->SetText(FText::FromString(ErrorText));
		}
		if (Txt_ErrorDetail)
		{
			Txt_ErrorDetail->SetText(FText::FromString(TEXT("Detail:")));
		}
		return;
	}

	if (Txt_Status)
	{
		Txt_Status->SetText(FText::FromString(FString::Printf(TEXT("Status: %s"), *Client->Status)));
	}

	if (Txt_Stage)
	{
		Txt_Stage->SetText(FText::FromString(FString::Printf(TEXT("Stage: %s"), *Client->Stage)));
	}

	if (Txt_Message)
	{
		const FString CombinedMessage = ToolStatusMessage.IsEmpty()
			? Client->Message
			: FString::Printf(TEXT("%s | Tool: %s"), *Client->Message, *ToolStatusMessage);
		Txt_Message->SetText(FText::FromString(FString::Printf(TEXT("Message: %s"), *CombinedMessage)));
	}

	if (Txt_Elapsed)
	{
		Txt_Elapsed->SetText(FText::FromString(FString::Printf(TEXT("Elapsed: %.1f s"), Client->ElapsedSeconds)));
	}

	if (Txt_File)
	{
		Txt_File->SetText(FText::FromString(FString::Printf(TEXT("File: %s"), *Client->LastDownloadedFilePath)));
	}

	if (Txt_Error)
	{
		const FString CombinedError = ToolErrorMessage.IsEmpty()
			? Client->ErrorMessage
			: FString::Printf(TEXT("%s%s%s"), *Client->ErrorMessage, Client->ErrorMessage.IsEmpty() ? TEXT("") : TEXT(" | "), *ToolErrorMessage);
		Txt_Error->SetText(FText::FromString(FString::Printf(TEXT("Error: %s"), *CombinedError)));
	}

	if (Txt_ErrorDetail)
	{
		Txt_ErrorDetail->SetText(FText::FromString(FString::Printf(TEXT("Detail: %s"), *Client->ErrorDetail)));
	}
}
