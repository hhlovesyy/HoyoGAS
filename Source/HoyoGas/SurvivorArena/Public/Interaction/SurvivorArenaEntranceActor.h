#pragma once

#include "CoreMinimal.h"
#include "Data/SurvivorArenaTypes.h"
#include "Interaction/HoyoInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "SurvivorArenaEntranceActor.generated.h"

class APlayerController;
class USceneComponent;

UCLASS()
class HOYOGAS_API ASurvivorArenaEntranceActor : public AActor, public IHoyoInteractableInterface
{
	GENERATED_BODY()

public:
	ASurvivorArenaEntranceActor();

	virtual bool CanInteract_Implementation(APlayerController* InteractingController) const override;
	virtual FText GetInteractionText_Implementation() const override;
	virtual void Interact_Implementation(APlayerController* InteractingController) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FSurvivorRunStartConfig StartConfigOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SurvivorArena")
	FText InteractionText = INVTEXT("进入生存试炼");
};
