#include "ElementReaction/AuraBarWidget.h"
#include "ElementReaction/ElementDummyTarget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UAuraBarWidget::SetObservedTarget(AElementDummyTarget* InTarget)
{
	ObservedTarget = InTarget;
}

void UAuraBarWidget::RefreshFromTarget()
{
	if (!ObservedTarget)
	{
		return;
	}

	if (TXT_AuraName)
	{
		TXT_AuraName->SetText(ObservedTarget->GetAuraText());
	}

	if (PB_Aura)
	{
		PB_Aura->SetPercent(ObservedTarget->GetAuraPercent());
		PB_Aura->SetFillColorAndOpacity(ObservedTarget->GetAuraColor());
	}

	SetRenderOpacity(ObservedTarget->HasAura() ? 1.0f : 0.0f);
}

void UAuraBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("UAuraBarWidget::NativeConstruct"));
	UE_LOG(LogTemp, Warning, TEXT("TXT_AuraName = %s"), TXT_AuraName ? TEXT("Valid") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("PB_Aura = %s"), PB_Aura ? TEXT("Valid") : TEXT("NULL"));
}