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
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Mannequin/FPWeapon/Audio/M4A1_Single.M4A1_Single"));
	FireSound = FireAudio.Object;
	
	//Hit sound
	static ConstructorHelpers::FObjectFinder<USoundBase> HitAudio(TEXT("/Game/Mannequin/Audio/classic_hurt.classic_hurt"));
	HitSound = HitAudio.Object;
	
	// Movement
	MoveSpeed = 500.0f;
	// Weapon
	Health = 10;
	GunOffset = FVector(120.f, 15.f, 50.f);
	FireRate = 0.12f;
	CameraMaxOffSet = 4.0f;
	bCanFire = true;
	bFiring = false;
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void AHeroCharacter::CalculateHealth(int Delta)
{
	if (HitSound && !isDead){
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	Health += Delta;
	CalculateDead();
}

void AHeroCharacter::CalculateDead()
{
	if (Health <= 0){
		isDead = true;
		UTindalosAnimInstance* animInstance = static_cast<UTindalosAnimInstance*>(GetMesh()->GetAnimInstance());
		animInstance->isDead = true;
	}
	else
		isDead = false;

}

FVector AHeroCharacter::CalculateCameraPosition() {
	FVector HitPosition = GetHitResultUnderCursor().ImpactPoint;



	FVector ActorPosition = GetActorLocation();
	FVector ProjectedHit = FVector(HitPosition.X, HitPosition.Y, ActorPosition.Z);

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
	}
}

void AHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!isDead) {

		// Find movement direction
		const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
		const float RightValue = GetInputAxisValue(MoveRightBinding);

		// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
		const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

		// Calculate  movement
		const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

		FHitResult TraceResult = GetHitResultUnderCursor();

		FVector LineStart = GetActorLocation();
		FVector LineStop = FVector(TraceResult.ImpactPoint.X, TraceResult.ImpactPoint.Y, LineStart.Z);

		const FVector FireDirection = (LineStop - LineStart).GetClampedToMaxSize(1.0f);


		// If non-zero size, move this actor
		if (Movement.SizeSquared() > 0.0f)
		{
			FHitResult Hit(1.f);
			RootComponent->MoveComponent(Movement, FRotator(0.0f, 0.0f, 0.0f), true, &Hit);

			if (Hit.IsValidBlockingHit())
			{
				const FRotator NewRotation = Movement.Rotation();
				const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
				const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
				RootComponent->MoveComponent(Deflection, NewRotation, true);
			}
		}
		Aim(MoveDirection, FireDirection);
		FireShot(FireDirection);
		CalculateDead();
	}
}


void AHeroCharacter::Aim(const FVector MoveDirection, const FVector FireDirection) {
	UTindalosAnimInstance* animInstance = static_cast<UTindalosAnimInstance*>(GetMesh()->GetAnimInstance());
	FVector diffDirection = FireDirection - MoveDirection;
	double angle = FMath::Acos(FVector::DotProduct(FireDirection, MoveDirection));
	FVector cross = FVector::CrossProduct(FireDirection, MoveDirection);
	if (FVector::DotProduct(FVector(0.0f,0.0f,1.0f), cross) < 0) { // Or > 0
		angle = -angle;
	}
	if (MoveDirection.SizeSquared() > 0.0f) {
		animInstance->Forward = FMath::Cos(angle);
		animInstance->Slide = FMath::Sin(angle);
	}
	else {
		animInstance->Forward = 0.0f;
		animInstance->Slide = 0.0f;
	}
}


void AHeroCharacter::StartFireShot() {
	bFiring = true;
}

void AHeroCharacter::StopFireShot() {
	bFiring = false;
}

void AHeroCharacter::FireShot(const FVector FireDirection){

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