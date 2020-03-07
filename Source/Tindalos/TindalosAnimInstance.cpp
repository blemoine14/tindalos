// Fill out your copyright notice in the Description page of Project Settings.


#include "TindalosAnimInstance.h"

UTindalosAnimInstance::UTindalosAnimInstance()
{
	Speed = 0.0f;
	Slide = 0.0f;
}

void UTindalosAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);
	AActor* OwningActor = GetOwningActor();
	if (OwningActor) {
		Speed = OwningActor->GetVelocity().Size();
		UE_LOG(LogTemp, Warning, TEXT("tindalos speed %f"),Speed);
	}
}