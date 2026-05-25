// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Object.h"
#include "Containers/Ticker.h"
#include "MotionPreviewClient.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class HOYOGAS_API UMotionPreviewClient : public UObject
{
	GENERATED_BODY()
	public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion Preview")
        FString ServerBaseUrl = TEXT("http://127.0.0.1:8000");
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Motion Preview")
        FString DownloadDir = TEXT("D:/GraduateStudent/Research/2026_03/0331/motion/fbx_out");
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString JobId;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString Status = TEXT("idle");
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString Stage = TEXT("");
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString Message = TEXT("");
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        float ElapsedSeconds = 0.f;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString FileName;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString ErrorMessage;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString ErrorDetail;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        FString LastDownloadedFilePath;
    
        UPROPERTY(BlueprintReadOnly, Category="Motion Preview")
        bool bIsBusy = false;
    
        UFUNCTION(BlueprintCallable, Category="Motion Preview")
        void StartJob(const FString& InExpName, int32 InMotionIndex);
    
        UFUNCTION(BlueprintCallable, Category="Motion Preview")
        void StartFixedTestJob();
    
        UFUNCTION(BlueprintCallable, Category="Motion Preview")
        void StopPolling();
    
    protected:
        virtual void BeginDestroy() override;
    
    private:
        FString CurrentExpName;
        int32 CurrentMotionIndex = 0;
    
        FTSTicker::FDelegateHandle PollTickerHandle;
        bool bPollRequestInFlight = false;
        bool bDownloadRequestInFlight = false;
    
        void SubmitRequest(const FString& InExpName, int32 InMotionIndex);
        void PollOnce();
        void DownloadResult();
        void ClearTicker();
    
        FString BuildTimestampedSavePath() const;
};
