// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Arrow.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UCameraComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;

UCLASS()
class BOWANDARROWCPP_API AArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrow();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* KillCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* TrialEffectLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MinSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MinGravityScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxGravityScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	USoundBase* WhooshSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	USoundBase* ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UNiagaraSystem* TrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ImpactEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UNiagaraComponent* TrailEffectRef;

	UPROPERTY()
	bool bHasFired;
	
	UPROPERTY()
	FVector Velocity;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

	//Arrow spin Timeline setup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spin")
	UTimelineComponent* SpinTimeline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spin")
	UCurveFloat* SpinCurve;

	FOnTimelineFloat OnSpinningProgress;
	FOnTimelineEvent OnSpinCompleted;

	UFUNCTION()
	void OnSpinUpdate(float XRotation);

	UFUNCTION()
	void OnSpinFinished();

	UFUNCTION()
	void SetupTimeline();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Fire(FVector Direction, float Strength);

};
