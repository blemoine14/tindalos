// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "tindalosAnimInstance.generated.h"

/**
 * 
 */
UCLASS(transient, Blueprintable, hideCategories=AnimInstance, BlueprintType)
class TINDALOS_API UTindalosAnimInstance: public UAnimInstance
{
	GENERATED_BODY()

public:
	UTindalosAnimInstance();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroCharacter")
		float Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroCharacter")
		float Slide;
};
