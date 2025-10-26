// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CrossHairWidget.h"

#include "BowMechanics.h"
#include "Kismet/GameplayStatics.h"

void UCrossHairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();

	UBowMechanics* BowMech = Cast<UBowMechanics>(OwningPawn->GetComponentByClass<UBowMechanics>());
	if (IsValid(BowMech))
	{
		BowMech->OnDrawUpdate.AddDynamic(this,&UCrossHairWidget::OnDrawUpdate);
		BowMech->OnDrawEnd.AddDynamic(this,&UCrossHairWidget::OnDrawEnd);
	}

	bHasPlayedFullDraw = false;
	
}

void UCrossHairWidget::OnDrawUpdate(float DrawTime, float MaxDrawTime)
{
	float DrawPercentage = DrawTime / MaxDrawTime;
	DrawPercentage = FMath::Clamp(DrawPercentage, 0.f, 1.f);
	float InversePercentage = 1.0f - DrawPercentage;

	OuterCircle->SetRenderScale(FVector2D(InversePercentage));

	//bDrawing = true;
	const float FullThreshold = 0.999f;
	if (DrawPercentage >= FullThreshold && !bHasPlayedFullDraw)
	{
		if (PulseAnimation)
		{
			PlayAnimation(PulseAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		}
		if (FullDrawnSound && IsValid(GetWorld()))
		{
			UGameplayStatics::PlaySound2D(GetWorld(), FullDrawnSound);
		}
	
			bHasPlayedFullDraw = true;
		
	}
}

void UCrossHairWidget::OnDrawEnd()
{
	bHasPlayedFullDraw = false;
	OuterCircle->SetRenderScale(FVector2D(1.0f));
}
