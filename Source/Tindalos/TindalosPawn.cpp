// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "tindalosPawn.h"
#include "tindalosProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "EngineGlobals.h"
#include "DrawDebugHelpers.h"


const FName ATindalosPawn::MoveForwardBinding("MoveForward");
const FName ATindalosPawn::MoveRightBinding("MoveRight");
const FName ATindalosPawn::FireBinding("Fire");

ATindalosPawn::ATindalosPawn()
{	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannequinMesh(TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin"));
	// Create the mesh component
	MannequinMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MannequinMesh"));
	RootComponent = MannequinMeshComponent;
	MannequinMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	MannequinMeshComponent->SetSkeletalMesh(MannequinMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when Mannequin does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
	bFiring = false;
}

void ATindalosPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAction(FireBinding, IE_Pressed, this, &ATindalosPawn::StartFireShot);
	PlayerInputComponent->BindAction(FireBinding, IE_Released, this, &ATindalosPawn::StopFireShot);

	//GetHitresultUnderCursor
	//GetHitResultAtScreenPosition

	UWorld* world = GetWorld();
	if (world) {
		APlayerController* playerController = world->GetFirstPlayerController();
		playerController->bShowMouseCursor = true;
		UE_LOG(LogTemp, Warning, TEXT("tindalosPawn mouse enable"), this);
	}
}

void ATindalosPawn::Tick(float DeltaSeconds)
{


	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);
		
		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}

	FireShot();
	
}

void ATindalosPawn::StartFireShot() {
	bFiring = true;
}

void ATindalosPawn::StopFireShot() {
	bFiring = false;
}

void ATindalosPawn::FireShot()
{
	// If it's ok to fire again
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	if (playerController && bCanFire && bFiring)
	{
		FHitResult TraceResult(ForceInit);
		playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, TraceResult);

		FVector LineStart = GetActorLocation();
		FVector LineStop = FVector(TraceResult.ImpactPoint.X, TraceResult.ImpactPoint.Y, LineStart.Z);

		DrawDebugLine(GetWorld(), LineStart, LineStop, FColor::Red, true);

		const FVector FireDirection = LineStop - LineStart;
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<ATindalosProjectile>(SpawnLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ATindalosPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void ATindalosPawn::ShotTimerExpired()
{
	bCanFire = true;
}

