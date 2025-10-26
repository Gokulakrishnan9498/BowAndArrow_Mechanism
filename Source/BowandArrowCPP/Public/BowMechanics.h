// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Arrow.h"
#include "Bow.h"
#include "Components/ActorComponent.h"
#include "BowMechanics.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAimEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDrawEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDrawUpdate, float, DrawTime, float, MaxDrawTime);

class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BOWANDARROWCPP_API UBowMechanics : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBowMechanics();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Bow8Arrow)
	ACharacter* CharacterRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Bow8Arrow)
	ABow* BowRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Bow8Arrow)
	AArrow* ArrowRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Bow8Arrow)
	TSubclassOf<AActor> BowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Bow8Arrow)
	TSubclassOf<AArrow> ArrowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	FName BowSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Bow8Arrow)
	FName ArrowSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	FRotator InitialRotationRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	bool bIsDrawingBow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	bool bIsFiringArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	bool bIsWaitingToDraw;

	UFUNCTION()
	void EquipBow();

	UFUNCTION()
	void DrawEnd();

	FTimerHandle DrawTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	float DrawTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	float DrawIncrementTime;

	UFUNCTION()
	void DrawTimeUpdate();

	UFUNCTION()
	void FireArrowEnd();

	UFUNCTION()
	FVector CalculateAimDirection();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow8Arrow)
	UAnimMontage* FireArrowMontage;

	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);
	
public:

	UPROPERTY()
	AArrow* FiredArrowRef;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Bow)
	bool bIsAiming;

	UFUNCTION()
	void DrawBegin();
	
	UFUNCTION()
	void AimBegin();

	UFUNCTION()
	void AimEnd();

	UPROPERTY(BlueprintAssignable, Category = "Bow8Arrow")
	FOnAimEvent OnAimBegin;

	UPROPERTY(BlueprintAssignable, Category = "Bow8Arrow")
	FOnAimEvent OnAimEnd;

	UPROPERTY(BlueprintAssignable, Category = "Bow8Arrow")
	FOnDrawUpdate OnDrawUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Bow8Arrow")
	FOnDrawEnd OnDrawEnd;

	UFUNCTION()
	void FireArrowBegin();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
