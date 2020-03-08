// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "tindalosProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "TindalosAnimInstance.h"
#include "HeroCharacter.generated.h"

UCLASS(Blueprintable)
class TINDALOS_API AHeroCharacter : public ACharacter
{
	GENERATED_BODY()


public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hero Character")
		int Health;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hero Character")
		bool isDead;

	/** Offset from the Mannequins location to spawn projectiles */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Gameplay)
		FVector GunOffset;

	/** Offset from the Mannequins location to spawn projectiles */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Gameplay)
		float CameraMaxOffSet;

	/* How fast the weapon will fire */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Gameplay)
		float FireRate;

	/* The speed our Mannequin moves around the level */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Gameplay)
		float MoveSpeed;

	/** Sound to play each time we fire */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Audio)
		class USoundBase* FireSound;

	/** Sound to play each time we got hit */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Audio)
		class USoundBase* HitSound;

	UFUNCTION(BlueprintCallable, Category = "Hero Character")
		virtual void CalculateHealth(int delta);

	UFUNCTION(BlueprintCallable, Category = "Hero Character")
		virtual void CalculateDead();


	UFUNCTION(BlueprintCallable, Category = "Hero Character")
		virtual FVector CalculateCameraPosition();


	//Utils
	FHitResult GetHitResultUnderCursor();

	/* Move character and update animation */
	void Aim(const FVector Movement, const FVector FireDirection);

	/* Fire a shot in the specified direction */
	void FireShot(const FVector FireDirection);
	void StartFireShot();
	void StopFireShot();

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireBinding;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	/* Flag to control firing  */
	uint32 bCanFire : 1;
	uint32 bFiring : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;


	// Sets default values for this character's properties
	AHeroCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};