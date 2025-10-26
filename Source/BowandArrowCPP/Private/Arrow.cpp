// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrow.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AArrow::AArrow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);
	BoxCollision->SetBoxExtent(FVector(50.0f,3.0f,3.0f));
	
	BoxCollision->SetGenerateOverlapEvents(true);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	//BoxCollision->SetNotifyRigidBodyCollision(true); // REQUIRED for hit events

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this,&AArrow::OnBoxOverlap);


	KillCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("KillCamera"));
	KillCamera->SetupAttachment(BoxCollision);
	KillCamera->SetRelativeLocation(FVector(-75.0f, 0.0f, 0.0f));
	KillCamera->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(BoxCollision);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TrialEffectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("TrialEffectLocation"));
	TrialEffectLocation->SetupAttachment(ArrowMesh);
	TrialEffectLocation->SetRelativeLocation(FVector(45.0f, 0.0f, 0.0f));

	SpinTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SpinTimeline"));

	MinSpeed = 1000.0f;
	MaxSpeed = 3000.0f;
	MinGravityScale = 0.0f;
	MaxGravityScale = 0.5f;

	bHasFired = false;

}

// Called when the game starts or when spawned
void AArrow::BeginPlay()
{
	Super::BeginPlay();

	if (!bHasFired)
	{
		//SetActorEnableCollision(false);
		BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Warning, TEXT("Arrow BeginPlay - Collision initially disabled"));
	}

	SetupTimeline();
}


void AArrow::SetupTimeline()
{
	if (SpinTimeline && SpinCurve)
	{
		//Binding timeline functions
		OnSpinningProgress.BindUFunction(this, FName("OnSpinUpdate"));
		OnSpinCompleted.BindUFunction(this, FName("OnSpinFinished"));

		//Adding float curve to timeline
		SpinTimeline->AddInterpFloat(SpinCurve, OnSpinningProgress);
		SpinTimeline->SetTimelineFinishedFunc(OnSpinCompleted);

		//Set Timeline properties
		SpinTimeline->SetLooping(true);
		SpinTimeline->SetIgnoreTimeDilation(false);
	}
}



void AArrow::OnSpinUpdate(float XRotation)
{
	ArrowMesh->SetRelativeRotation(FRotator(0.0f,0.0f,XRotation));
}

void AArrow::OnSpinFinished()
{
	UE_LOG(LogTemp,  Log, TEXT("Arrow Spin Timeline Finished"));
}

// Called every frame
void AArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArrow::Fire(FVector Direction, float Strength)
{
	if (!ProjectileMovement)
	{
		ProjectileMovement = NewObject<UProjectileMovementComponent>(this,UProjectileMovementComponent::StaticClass(),TEXT("ProjectileMovementComp"));
		ProjectileMovement->RegisterComponent();
		ProjectileMovement->UpdatedComponent = BoxCollision;
		ProjectileMovement->bRotationFollowsVelocity = true;
		ProjectileMovement->bInitialVelocityInLocalSpace = true;
		ProjectileMovement->ProjectileGravityScale = 0.5f;

		BoxCollision->UpdateOverlaps();

		float NewSpeed = UKismetMathLibrary::Lerp(MinSpeed, MaxSpeed, Strength);
		ProjectileMovement->InitialSpeed = NewSpeed;
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
		Velocity = ProjectileMovement->Velocity;

		UE_LOG(LogTemp, Warning, TEXT("Arrow Fired with Strength: %f, Speed: %f, Velocity: %s"), Strength, NewSpeed, *Velocity.ToString());

		ProjectileMovement->ProjectileGravityScale = UKismetMathLibrary::Lerp(MaxGravityScale, MinGravityScale, Strength);

		UE_LOG(LogTemp, Warning, TEXT("Arrow Gravity Scale set to: %f"), ProjectileMovement->ProjectileGravityScale);
	}

	if (ensure(WhooshSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, WhooshSound, GetActorLocation());
	}

	if (ensure(TrailEffect))
	{
		TrailEffectRef = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailEffect, TrialEffectLocation, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}

	bHasFired = true;
	//Enable BoxCollision
	if (bHasFired)
	{
		/*SetActorEnableCollision(true);
		BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);*/
		//BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));


		BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//BoxCollision->SetNotifyRigidBodyCollision(true);
		BoxCollision->RecreatePhysicsState(); // Important!

		BoxCollision->IgnoreActorWhenMoving(GetOwner(), true);
		BoxCollision->IgnoreActorWhenMoving(GetInstigator(), true);

		UE_LOG(LogTemp, Warning, TEXT("Arrow collision : %d"), BoxCollision->GetCollisionEnabled());

		SetLifeSpan(5.0f);

		SpinTimeline->PlayFromStart();
	}
	
}



void AArrow::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		UE_LOG(LogTemp, Warning, TEXT("Arrow hit: %s"), *OtherActor->GetName());
		UE_LOG(LogTemp, Warning, TEXT("Arrow hit: %s"), *GetInstigator()->GetName());
		if (IsValid(ProjectileMovement))
		{
			ProjectileMovement->DestroyComponent();
			BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SetActorEnableCollision(false);
			UE_LOG(LogTemp, Warning, TEXT("Arrow collision disabled after hit"));
			SpinTimeline->Stop();
			AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);

			if (ensure(ImpactSound))
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
			}

			if (ensure(ImpactEffect))
			{
				UGameplayStatics::SpawnEmitterAtLocation(this, ImpactEffect, SweepResult.Location, FRotator::ZeroRotator, FVector(1.0f), true);
			}
			if (OtherComp->IsSimulatingPhysics())
			{
				OtherComp->AddImpulse(Velocity, NAME_None, true);
			}
			UKismetSystemLibrary::Delay(this, 0.5f, FLatentActionInfo());
			if (IsValid(TrailEffectRef))
			{
				TrailEffectRef->DestroyComponent();
			}
		}
	}
}

/*void AArrow::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	
}*/
