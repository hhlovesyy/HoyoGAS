#pragma once

#include "CoreMinimal.h"
#include "CharacterPanelTypes.generated.h"

UENUM(BlueprintType)
enum class EHoyoCharacterPanelTab : uint8
{
	Details UMETA(DisplayName = "Details"),
	LightCone UMETA(DisplayName = "Light Cone"), //光锥
	Traces UMETA(DisplayName = "Traces"), //行迹
	Relics UMETA(DisplayName = "Relics"), //圣遗物
	Eidolons UMETA(DisplayName = "Eidolons"), //星魂
	Info UMETA(DisplayName = "Info")
};
