// Fill out your copyright notice in the Description page of Project Settings.


#include "MotionPreviewClient.h"

#include "MotionPreviewClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UMotionPreviewClient::BeginDestroy()
{
    ClearTicker();
    Super::BeginDestroy();
}

void UMotionPreviewClient::ClearTicker()
{
    if (PollTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(PollTickerHandle);
        PollTickerHandle.Reset();
    }
}

void UMotionPreviewClient::StopPolling()
{
    bIsBusy = false;
    bPollRequestInFlight = false;
    bDownloadRequestInFlight = false;
    ClearTicker();
    Status = TEXT("stopped");
    Stage = TEXT("stopped");
    Message = TEXT("polling stopped");
}

void UMotionPreviewClient::StartFixedTestJob()
{
    StartJob(TEXT("TestButton"), 4);
}

void UMotionPreviewClient::StartJob(const FString& InExpName, int32 InMotionIndex)
{
    // 清空旧状态
    ClearTicker();

    CurrentExpName = InExpName;
    CurrentMotionIndex = InMotionIndex;

    JobId = TEXT("");
    Status = TEXT("submitting");
    Stage = TEXT("submit");
    Message = TEXT("submitting job");
    ElapsedSeconds = 0.f;
    FileName = TEXT("");
    ErrorMessage = TEXT("");
    ErrorDetail = TEXT("");
    LastDownloadedFilePath = TEXT("");

    bIsBusy = true;
    bPollRequestInFlight = false;
    bDownloadRequestInFlight = false;

    SubmitRequest(InExpName, InMotionIndex);
}

void UMotionPreviewClient::SubmitRequest(const FString& InExpName, int32 InMotionIndex)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("exp_name"), InExpName);
    JsonObject->SetNumberField(TEXT("motion_index"), InMotionIndex);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ServerBaseUrl + TEXT("/submit"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(RequestBody);

    TWeakObjectPtr<UMotionPreviewClient> WeakThis(this);

    Request->OnProcessRequestComplete().BindLambda(
        [WeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!WeakThis.IsValid())
            {
                return;
            }

            UMotionPreviewClient* Self = WeakThis.Get();

            if (!bSuccess || !Resp.IsValid())
            {
                Self->bIsBusy = false;
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("submit");
                Self->Message = TEXT("submit request failed");
                Self->ErrorMessage = TEXT("HTTP submit failed");
                Self->ErrorDetail = TEXT("No valid HTTP response.");
                return;
            }

            TSharedPtr<FJsonObject> JsonResponse;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());

            if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
            {
                Self->bIsBusy = false;
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("submit");
                Self->Message = TEXT("submit response parse failed");
                Self->ErrorMessage = TEXT("JSON parse failed");
                Self->ErrorDetail = Resp->GetContentAsString();
                return;
            }

            JsonResponse->TryGetStringField(TEXT("job_id"), Self->JobId);
            JsonResponse->TryGetStringField(TEXT("status"), Self->Status);
            JsonResponse->TryGetStringField(TEXT("stage"), Self->Stage);
            JsonResponse->TryGetStringField(TEXT("message"), Self->Message);

            if (Self->JobId.IsEmpty())
            {
                Self->bIsBusy = false;
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("submit");
                Self->Message = TEXT("job_id missing");
                Self->ErrorMessage = TEXT("submit ok but job_id missing");
                Self->ErrorDetail = Resp->GetContentAsString();
                return;
            }

            // 开始轮询
            TWeakObjectPtr<UMotionPreviewClient> PollWeakThis(Self);
            Self->PollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateLambda(
                    [PollWeakThis](float DeltaTime)
                    {
                        if (!PollWeakThis.IsValid())
                        {
                            return false;
                        }

                        UMotionPreviewClient* Client = PollWeakThis.Get();

                        if (!Client->bIsBusy)
                        {
                            return false;
                        }

                        Client->PollOnce();
                        return true;
                    }),
                2.0f // 每 2 秒轮询一次
            );
        }
    );

    Request->ProcessRequest();
}

void UMotionPreviewClient::PollOnce()
{
    if (!bIsBusy || JobId.IsEmpty() || bPollRequestInFlight)
    {
        return;
    }

    bPollRequestInFlight = true;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ServerBaseUrl + TEXT("/status/") + JobId);
    Request->SetVerb(TEXT("GET"));

    TWeakObjectPtr<UMotionPreviewClient> WeakThis(this);

    Request->OnProcessRequestComplete().BindLambda(
        [WeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!WeakThis.IsValid())
            {
                return;
            }

            UMotionPreviewClient* Self = WeakThis.Get();
            Self->bPollRequestInFlight = false;

            if (!bSuccess || !Resp.IsValid())
            {
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("poll");
                Self->Message = TEXT("status request failed");
                Self->ErrorMessage = TEXT("HTTP poll failed");
                Self->ErrorDetail = TEXT("No valid HTTP response.");
                Self->bIsBusy = false;
                Self->ClearTicker();
                return;
            }

            TSharedPtr<FJsonObject> JsonResponse;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());

            if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
            {
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("poll");
                Self->Message = TEXT("status parse failed");
                Self->ErrorMessage = TEXT("JSON parse failed");
                Self->ErrorDetail = Resp->GetContentAsString();
                Self->bIsBusy = false;
                Self->ClearTicker();
                return;
            }

            JsonResponse->TryGetStringField(TEXT("status"), Self->Status);
            JsonResponse->TryGetStringField(TEXT("stage"), Self->Stage);
            JsonResponse->TryGetStringField(TEXT("message"), Self->Message);
            JsonResponse->TryGetStringField(TEXT("file_name"), Self->FileName);
            JsonResponse->TryGetStringField(TEXT("error_message"), Self->ErrorMessage);
            JsonResponse->TryGetStringField(TEXT("error_detail"), Self->ErrorDetail);

            double Elapsed = 0.0;
            if (JsonResponse->TryGetNumberField(TEXT("elapsed_seconds"), Elapsed))
            {
                Self->ElapsedSeconds = static_cast<float>(Elapsed);
            }

            if (Self->Status == TEXT("done"))
            {
                Self->ClearTicker();
                Self->Stage = TEXT("download");
                Self->Message = TEXT("downloading fbx");
                Self->DownloadResult();
            }
            else if (Self->Status == TEXT("failed"))
            {
                Self->bIsBusy = false;
                Self->ClearTicker();
            }
        }
    );

    Request->ProcessRequest();
}

FString UMotionPreviewClient::BuildTimestampedSavePath() const
{
    const FString SafeDir = DownloadDir;
    IFileManager::Get().MakeDirectory(*SafeDir, true);

    FString SourceFileName = FileName;
    if (SourceFileName.IsEmpty())
    {
        SourceFileName = JobId + TEXT(".fbx");
    }

    const FString BaseName = FPaths::GetBaseFilename(SourceFileName);
    const FString Extension = FPaths::GetExtension(SourceFileName, false);

    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    FString FinalFileName;
    if (Extension.IsEmpty())
    {
        FinalFileName = FString::Printf(TEXT("%s_%s.fbx"), *BaseName, *Timestamp);
    }
    else
    {
        FinalFileName = FString::Printf(TEXT("%s_%s.%s"), *BaseName, *Timestamp, *Extension);
    }

    return FPaths::Combine(SafeDir, FinalFileName);
}

void UMotionPreviewClient::DownloadResult()
{
    if (JobId.IsEmpty() || bDownloadRequestInFlight)
    {
        return;
    }

    bDownloadRequestInFlight = true;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ServerBaseUrl + TEXT("/download/") + JobId);
    Request->SetVerb(TEXT("GET"));

    TWeakObjectPtr<UMotionPreviewClient> WeakThis(this);

    Request->OnProcessRequestComplete().BindLambda(
        [WeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!WeakThis.IsValid())
            {
                return;
            }

            UMotionPreviewClient* Self = WeakThis.Get();
            Self->bDownloadRequestInFlight = false;

            if (!bSuccess || !Resp.IsValid())
            {
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("download");
                Self->Message = TEXT("download failed");
                Self->ErrorMessage = TEXT("HTTP download failed");
                Self->ErrorDetail = TEXT("No valid HTTP response.");
                Self->bIsBusy = false;
                return;
            }

            const FString SavePath = Self->BuildTimestampedSavePath();

            if (!FFileHelper::SaveArrayToFile(Resp->GetContent(), *SavePath))
            {
                Self->Status = TEXT("failed");
                Self->Stage = TEXT("download");
                Self->Message = TEXT("save file failed");
                Self->ErrorMessage = TEXT("SaveArrayToFile failed");
                Self->ErrorDetail = SavePath;
                Self->bIsBusy = false;
                return;
            }

            Self->LastDownloadedFilePath = SavePath;
            Self->Status = TEXT("downloaded");
            Self->Stage = TEXT("finished");
            Self->Message = TEXT("fbx downloaded");
            Self->bIsBusy = false;
        }
    );

    Request->ProcessRequest();
}