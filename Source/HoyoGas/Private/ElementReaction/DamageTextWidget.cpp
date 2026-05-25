#include "ElementReaction/DamageTextWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/MyPlayerUISubsystem.h"

void UDamageTextWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UDamageTextWidget::InitDamage(float Damage, const FText& ReactionText, const FLinearColor& Color, EDamageTextStyleType AnimType)
{
	CachedAnimType = AnimType;

	if (TXT_Damage)
	{
		TXT_Damage->SetText(FText::AsNumber(FMath::RoundToInt(Damage)));
		TXT_Damage->SetColorAndOpacity(FSlateColor(Color));
	}

	if (TXT_Reaction)
	{
		if (!ReactionText.IsEmpty())
		{
			TXT_Reaction->SetText(ReactionText);
			TXT_Reaction->SetColorAndOpacity(FSlateColor(Color));
			TXT_Reaction->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TXT_Reaction->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UWidgetAnimation* AnimToPlay = nullptr;

	switch (CachedAnimType)
	{
	case EDamageTextStyleType::Vaporize:
		AnimToPlay = Anim_Vaporize;
		break;
	case EDamageTextStyleType::Melt:
		AnimToPlay = Anim_Melt;
		break;
	case EDamageTextStyleType::Frozen:
		AnimToPlay = Anim_Frozen;
		break;
	default:
		AnimToPlay = Anim_Normal;
		break;
	}

	if (AnimToPlay)
	{
		UnbindAllFromAnimationFinished(AnimToPlay);
		FWidgetAnimationDynamicEvent AnimFinishedDelegate;
		AnimFinishedDelegate.BindDynamic(this, &UDamageTextWidget::HandleAnimFinished);
		BindToAnimationFinished(AnimToPlay, AnimFinishedDelegate);
		PlayAnimation(AnimToPlay);
	}
	else
	{
		SetRenderOpacity(1.0f);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				RequestClose();
			},
			1.0f,
			false);
	}
}

void UDamageTextWidget::SetScreenPosition(FVector2D InScreenPosition)
{
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(InScreenPosition);
	}
	else
	{
		SetRenderTranslation(InScreenPosition);
	}
}

void UDamageTextWidget::HandleAnimFinished()
{
	RequestClose();
}

void UDamageTextWidget::RequestClose()
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			LocalPlayer = OwningPlayer->GetLocalPlayer();
		}
	}

	if (LocalPlayer)
	{
		if (UMyPlayerUISubsystem* PlayerUISubsystem = ULocalPlayer::GetSubsystem<UMyPlayerUISubsystem>(LocalPlayer))
		{
			if (PlayerUISubsystem->RemoveOverlayWidget(this))
			{
				return;
			}
		}
	}

	RemoveFromParent();
}
