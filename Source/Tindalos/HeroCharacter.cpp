// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"

const FName AHeroCharacter::MoveForwardBinding("MoveForward");
const FName AHeroCharacter::MoveRightBinding("MoveRight");
const FName AHeroCharacter::FireBinding("Fire");

// Sets default values
AHeroCharacter::AHeroCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Fire sound
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;
	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	CameraMaxOffSet = 4.0f;
	bCanFire = true;
	bFiring = false;
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void AHeroCharacter::CalculateHealth(float Delta)
{
	Health += Delta;
	CalculateDead();
}

void AHeroCharacter::CalculateDead()
{
	if (Health <= 0)
		isDead = true;
	else
		isDead = false;

}

FVector AHeroCharacter::CalculateCameraPosition() {
	FVector HitPosition = GetHitResultUnderCursor().ImpactPoint;



	FVector ActorPosition = GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("tindalosPawn actor : %f %f %f"), ActorPosition.X, ActorPosition.Y, ActorPosition.Z);
	FVector ProjectedHit = FVector(HitPosition.X, HitPosition.Y, ActorPosition.Z);
	UE_LOG(LogTemp, Warning, TEXT("tindalosPawn Hit : %f %f %f"), ProjectedHit.X, ProjectedHit.Y, ProjectedHit.Z);

	return (ProjectedHit - ActorPosition) / CameraMaxOffSet;
}

FHitResult AHeroCharacter::GetHitResultUnderCursor() {
	UWorld* const World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FHitResult TraceResult(ForceInit);

			int32 height, width;
			FVector mouse;

			PlayerController->GetViewportSize(height,width);
			PlayerController->GetMousePosition(mouse.X,mouse.Y);
			UE_LOG(LogTemp, Warning, TEXT("tindalosPawn mouse on screen : %f %f %f %f"), height, width, (height-mouse.X)/2.0f, (width-mouse.Y)/2.0f);
			
			PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, TraceResult);
			return TraceResult;
		}
	}
	return FHitResult();
}

void AHeroCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAction(FireBinding, IE_Pressed, this, &AHeroCharacter::StartFireShot);
	PlayerInputComponent->BindAction(FireBinding, IE_Released, this, &AHeroCharacter::StopFireShot);


	UWorld* world = GetWorld();
	APlayerController* playerController = world->GetFirstPlayerController();
	if (playerController) {
		playerController->bShowMouseCursor = true;
		UE_LOG(LogTemp, Warning, TEXT("tindalosPawn mouse enable"));
	}
}

void AHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
		RootComponent->MoveComponent(Movement, FRotator(0.0f,0.0f,0.0f), true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}

	FireShot();

}

void AHeroCharacter::StartFireShot() {
	bFiring = true;
}

void AHeroCharacter::StopFireShot() {
	bFiring = false;
}

void AHeroCharacter::FireShot(){

	FHitResult TraceResult = GetHitResultUnderCursor();

	FVector LineStart = GetActorLocation();
	FVector LineStop = FVector(TraceResult.ImpactPoint.X, TraceResult.ImpactPoint.Y, LineStart.Z);

	const FVector FireDirection = LineStop - LineStart;

	const FRotator NewRotation = FireDirection.Rotation();
	FHitResult Hit(1.f);
	RootComponent->MoveComponent(FVector(0.0f, 0.0f, 0.0f), NewRotation, true, &Hit);

	// If it's ok to fire again
	if (bCanFire && bFiring){

		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

				
			UWorld* const World = GetWorld();
			if (World) {
				// spawn the projectile
				World->SpawnActor<ATindalosProjectile>(SpawnLocation, FireRotation);

				bCanFire = false;
				World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AHeroCharacter::ShotTimerExpired, FireRate);
			}

			// try and play the sound if specified
			if (FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void AHeroCharacter::ShotTimerExpired()
{
	bCanFire = true;
}