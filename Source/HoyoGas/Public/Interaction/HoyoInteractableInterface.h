// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HoyoInteractableInterface.generated.h"

class APlayerController;

UINTERFACE(BlueprintType)
class HOYOGAS_API UHoyoInteractableInterface : public UInterface
{
	GENERATED_BODY()
	//U 类 (继承自 UInterface)： 这是一个“空壳”，主要是给虚幻引擎的反射系统（UHT - Unreal Header Tool）看的。它让引擎在编辑器里知道有这么个接口，以便在蓝图中显示。
};

class HOYOGAS_API IHoyoInteractableInterface
{
	GENERATED_BODY()
	//这是真正的接口主体。所有的功能声明（如 CanInteract）都写在这里，C++ 类继承的也是这个 I 类。
public:
	//BlueprintNativeEvent 的含义： 它意味着这个函数可以在 C++ 中提供默认实现，同时允许蓝图完全覆盖（Override）它
	//为什么加 _Implementation？ 因为当你声明了 BlueprintNativeEvent 后，UE 的自动代码生成工具（UHT）会在后台悄悄生成一个同名的函数实体（比如 CanInteract）。为了不产生冲突，C++ 中的实际逻辑必须写在带有 _Implementation 后缀的方法里。你的代码里完美执行了这一点（例如 CanInteract_Implementation，详见实现了这个接口的类）。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(APlayerController* InteractingController) const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionText() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(APlayerController* InteractingController);
};
