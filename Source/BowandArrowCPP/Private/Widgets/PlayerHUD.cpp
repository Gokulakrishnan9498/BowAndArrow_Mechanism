// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerHUD.h"

#include "BowMechanics.h"
#include "Widgets/CrossHairWidget.h"

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();

	UBowMechanics* BowMech = Cast<UBowMechanics>(OwningPawn->GetComponentByClass<UBowMechanics>());
	if (IsValid(BowMech))
	{
		CrossHairWidget->SetVisibility(ESlateVisibility::Collapsed);

		BowMech->OnAimBegin.AddDynamic(this, &UPlayerHUD::OnAimBegin);
		BowMech->OnAimEnd.AddDynamic(this, &UPlayerHUD::OnAimEnd);
	}
}

void UPlayerHUD::OnAimBegin()
{
	CrossHairWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::OnAimEnd()
{
	CrossHairWidget->SetVisibility(ESlateVisibility::Collapsed);
}
