// Fill out your copyright notice in the Description page of Project Settings.


#include "TindalosAnimInstance.h"

UTindalosAnimInstance::UTindalosAnimInstance()
{
	Forward = 0.0f;
	Slide = 0.0f;
}

void UTindalosAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("tindalos speed %f %f"),Forward,Slide);
}