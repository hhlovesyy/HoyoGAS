#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GameplayDemoListView.generated.h"

UCLASS()
class HOYOGAS_API UGameplayDemoListView : public UListView
{
	GENERATED_BODY()

public:
	void SetEntryWidgetClassPublic(TSubclassOf<UUserWidget> InEntryWidgetClass)
	{
		EntryWidgetClass = InEntryWidgetClass;
	}

	void SetListItemsPublic(const TArray<UObject*>& InItems)
	{
		SetListItems<UObject*>(InItems);
	}
};
