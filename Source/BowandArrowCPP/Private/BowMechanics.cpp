// Fill out your copyright notice in the Description page of Project Settings.


#include "BowMechanics.h"

#include "Arrow.h"
#include "Bow.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UBowMechanics::UBowMechanics()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bIsDrawingBow = false;
	bIsFiringArrow = false;
	bIsAiming = false;
	bIsWaitingToDraw = false;

	DrawIncrementTime = 0.03333f;
	DrawTime = 0.0f;

}


// Called when the game starts
void UBowMechanics::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterRef = Cast<ACharacter>(GetOwner());

	InitialRotationRate = CharacterRef->GetCharacterMovement()->RotationRate;

	UE_LOG(LogTemp, Warning, TEXT("Initial Rotation Rate: %s"), *InitialRotationRate.ToString());

	EquipBow();
}

void UBowMechanics::EquipBow()
{
	if (!CharacterRef || !BowClass)
	{
		UE_LOG(LogTemp, Log, TEXT("CharacterRef or BowClass is null in BowMechanics"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = CharacterRef;

	FTransform SpawnTransform = FTransform::Identity;
	
	
	BowRef = GetWorld()->SpawnActor<ABow>(BowClass, SpawnTransform, SpawnParams);

	BowRef->AttachToComponent(CharacterRef->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, BowSocketName);
	BowRef->SetActorScale3D(BowRef->GetActorScale());
}


void UBowMechanics::AimBegin()
{
    bIsAiming = true;
	CharacterRef->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterRef->GetCharacterMovement()->bUseControllerDesiredRotation = true;

	FRotator AimRotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	CharacterRef->GetCharacterMovement()->RotationRate = AimRotationRate;
	UE_LOG(LogTemp, Warning, TEXT("Aim Rotation Rate: %s"), *AimRotationRate.ToString());

	BowRef->BowState = EBowState::Aiming;

	//Spawn Arrow
	if (!IsValid(ArrowRef))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = CharacterRef;

		FTransform SpawnTransform = FTransform::Identity;

		ArrowRef = GetWorld()->SpawnActor<AArrow>(ArrowClass, SpawnTransform, SpawnParams);

		ArrowRef->AttachToComponent(CharacterRef->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, ArrowSocketName);

		OnAimBegin.Broadcast();
	}
}

void UBowMechanics::AimEnd()
{
	bIsAiming = false;
	CharacterRef->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterRef->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	CharacterRef->GetCharacterMovement()->RotationRate = InitialRotationRate;

	BowRef->BowState = EBowState::Idle;

	//Destroy Arrow
	if (ArrowRef)
	{
		ArrowRef->Destroy();
		ArrowRef = nullptr;
	}
	
	OnAimEnd.Broadcast();
}

void UBowMechanics::DrawBegin()
{
	bool bCanDrawBow = !bIsDrawingBow && !bIsFiringArrow && bIsAiming;
	if (!bCanDrawBow)
	{
		bIsWaitingToDraw = true;
	}
	if (bCanDrawBow)
	{
		bIsWaitingToDraw = false;
		bIsDrawingBow = true;

		GetWorld()->GetTimerManager().SetTimer(DrawTimerHandle, this, &UBowMechanics::DrawTimeUpdate, DrawIncrementTime, true);

		BowRef->DrawBegin();
	}
}

void UBowMechanics::DrawTimeUpdate()
{
	DrawTime += DrawIncrementTime;
/**	if (DrawTime >= BowRef->MaxDrawTime)
	{
		GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
	}*/
	OnDrawUpdate.Broadcast(DrawTime, BowRef->MaxDrawTime);
}

void UBowMechanics::DrawEnd()
{
	bIsDrawingBow = false;
	bIsWaitingToDraw = false;
	DrawTime = 0.0f;

	GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
	BowRef->DrawEnd();
	OnDrawEnd.Broadcast();
}

void UBowMechanics::FireArrowBegin()
{
	if (!bIsDrawingBow && !ArrowRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Drawing bow is set to false, cannot fire arrow"));
		
		return;
	}
	bIsFiringArrow = true;

	if (IsValid(ArrowRef))
	{
		ArrowRef->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	

		float NormalizedValue = UKismetMathLibrary::NormalizeToRange(DrawTime, 0.0f, BowRef->MaxDrawTime);
		float ClampedStrength = FMath::Clamp(NormalizedValue, 0.0f, 1.0f);

		ArrowRef->Fire(CalculateAimDirection(), ClampedStrength);
		FiredArrowRef = ArrowRef;
	}
	ArrowRef = nullptr;

	DrawEnd();

	UAnimInstance* AnimInstance = CharacterRef->GetMesh()->GetAnimInstance();
	if (AnimInstance && FireArrowMontage)
	{
		float duration = AnimInstance->Montage_Play(FireArrowMontage, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Fire Arrow Montage Duration: %f"), duration);
		// Bind to montage end event

		//float FullDuration = FireArrowMontage->GetPlayLength();
		//UE_LOG(LogTemp, Log, TEXT("Fire Arrow Montage Full Duration: %f"), FullDuration);
		if (duration > 0.f)
		{
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &UBowMechanics::OnMontageCompleted);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, FireArrowMontage);
		}
	}
	
}

void UBowMechanics::OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemp, Log, TEXT("Fire Arrow Montage was interrupted"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Fire Arrow Montage completed successfully"));
	}
	FireArrowEnd();
}


void UBowMechanics::FireArrowEnd()
{
	bIsFiringArrow = false;
	if (!bIsAiming)
	{
		UE_LOG(LogTemp, Log, TEXT("bIsAiming is in false state, cannot aim again" ));
		return;
	}
	if (bIsAiming)
	{
		AimBegin();
		
		if (bIsWaitingToDraw)
		{
			DrawBegin();
		}
	}
}

FVector UBowMechanics::CalculateAimDirection()
{
	if (!CharacterRef)
	{
		return FVector::ForwardVector;
	}

	UCameraComponent* CameraComp = CharacterRef->FindComponentByClass<UCameraComponent>();

	if (!CameraComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Component not found on Character"));
		return FVector::ForwardVector;
	}

	FHitResult Hit;
	FVector StartLocation = CameraComp->GetComponentLocation();
	FVector EndLocation = StartLocation + (CameraComp->GetForwardVector() * 10000.0f);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterRef);
	

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

	FVector TargetLocation;
	
	if (bHit && Hit.bBlockingHit)
	{
		TargetLocation = Hit.Location;

		UE_LOG(LogTemp, Log, TEXT("Aim Trace Hit at location: %s"), *Hit.Location.ToString());
	}

	else
	{
		TargetLocation = Hit.TraceEnd;

		UE_LOG(LogTemp, Log, TEXT("Aim Trace is missed. Using Trace end: %s"), *Hit.TraceEnd.ToString());
	}

	
	if (!IsValid(ArrowRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("ArrowRef is not valid in CalculateAimDirection"));
		return FVector::ForwardVector;
	}

	FVector ArrowLocation = ArrowRef->GetActorLocation();
	FVector AimDirection = UKismetMathLibrary::GetDirectionUnitVector(ArrowLocation, TargetLocation);

	UE_LOG(LogTemp, Log, TEXT("Calculated aim direction: %s"), *AimDirection.ToString())

	return AimDirection;
}



// Called every frame
void UBowMechanics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

