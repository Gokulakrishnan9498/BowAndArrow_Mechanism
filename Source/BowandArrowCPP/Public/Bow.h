// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bow.generated.h"

UENUM(BlueprintType)
enum class EBowState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Aiming UMETA(DisplayName = "Aiming"),
	Drawing UMETA(DisplayName = "Drawing"),
};

class USkeletalMeshComponent;
class USoundBase;
class UAudioComponent;

UCLASS()
class BOWANDARROWCPP_API ABow : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ABow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
	EBowState BowState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow")
	float MaxDrawTime;

	UFUNCTION()
	void DrawBegin();

	UFUNCTION()
	void DrawEnd();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* BowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sound")
	UAudioComponent* DrawSoundRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* DrawSound;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
