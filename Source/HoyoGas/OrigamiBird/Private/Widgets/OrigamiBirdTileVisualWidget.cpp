#include "Widgets/OrigamiBirdTileVisualWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogOrigamiBirdTileVisualWidget, Log, All);

void UOrigamiBirdTileVisualWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HitButton)
	{
		HitButton->OnClicked.RemoveDynamic(this, &UOrigamiBirdTileVisualWidget::HandleHitButtonClicked);
		HitButton->OnClicked.AddDynamic(this, &UOrigamiBirdTileVisualWidget::HandleHitButtonClicked);
	}
	else
	{
		UE_LOG(LogOrigamiBirdTileVisualWidget, Error, TEXT("TileVisualWidget requires HitButton to be bound in the widget blueprint."));
	}

	RefreshVisualState();
}

void UOrigamiBirdTileVisualWidget::ApplyTileVisual(const FOrigamiBirdTile& InTile, UTexture2D* InIconTexture, FLinearColor InDebugColor)
{
	Tile = InTile;
	IconTexture = InIconTexture;
	DebugColor = InDebugColor;
	bSelected = InTile.bIsSelected;

	RefreshVisualState();
}

void UOrigamiBirdTileVisualWidget::SetSelected(bool bInSelected)
{
	if (bSelected == bInSelected)
	{
		return;
	}

	bSelected = bInSelected;
	RefreshVisualState();
}

bool UOrigamiBirdTileVisualWidget::IsSelected() const
{
	return bSelected;
}

void UOrigamiBirdTileVisualWidget::SetInputEnabled(bool bInEnabled)
{
	if (HitButton)
	{
		HitButton->SetIsEnabled(bInEnabled);
	}
	else
	{
		SetIsEnabled(bInEnabled);
	}
}

void UOrigamiBirdTileVisualWidget::SetTileSize(float InTileSize)
{
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(InTileSize);
		RootSizeBox->SetHeightOverride(InTileSize);
	}
}

int32 UOrigamiBirdTileVisualWidget::GetTileId() const
{
	return Tile.TileId;
}

FIntPoint UOrigamiBirdTileVisualWidget::GetBoardPosition() const
{
	return Tile.BoardPosition;
}

EOrigamiBirdTileType UOrigamiBirdTileVisualWidget::GetTileType() const
{
	return Tile.TileType;
}

void UOrigamiBirdTileVisualWidget::ResetVisualTransform()
{
	SetRenderTranslation(FVector2D::ZeroVector);
	SetRenderScale(FVector2D(1.0f, 1.0f));
	SetRenderOpacity(1.0f);
}

void UOrigamiBirdTileVisualWidget::HandleHitButtonClicked()
{
	OnTileClicked.Broadcast(Tile.BoardPosition);
}

void UOrigamiBirdTileVisualWidget::RefreshVisualState()
{
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(FLinearColor(0.07f, 0.08f, 0.09f, 1.0f));
	}

	if (IconImage)
	{
		if (IconTexture)
		{
			IconImage->SetBrushFromTexture(IconTexture, true);
			IconImage->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			IconImage->SetColorAndOpacity(DebugColor);
		}
	}

	if (SelectionBorder)
	{
		SelectionBorder->SetBrushColor(bSelected
			? FLinearColor(0.95f, 0.78f, 0.25f, 1.0f)
			: FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	}
}
