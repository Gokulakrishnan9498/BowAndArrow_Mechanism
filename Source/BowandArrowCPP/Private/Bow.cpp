// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABow::ABow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BowMesh"));
	RootComponent = BowMesh;

	MaxDrawTime = 1.0f;
	DrawSoundRef = nullptr;

	//BowMesh->SetRelativeScale3D(FVector(1.25f, 1.25f, 1.25f));

}


// Called when the game starts or when spawned
void ABow::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABow::DrawBegin()
{
	BowState = EBowState::Drawing;
	if (ensure(DrawSound))
	{
		DrawSoundRef = UGameplayStatics::SpawnSoundAtLocation(this, DrawSound, GetActorLocation());
	}
}

void ABow::DrawEnd()
{
	BowState = EBowState::Idle;
	if (DrawSoundRef)
	{
		DrawSoundRef->Stop();
	}
}

// Called every frame
void ABow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

