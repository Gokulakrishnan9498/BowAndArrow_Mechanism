// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
class UCrossHairWidget;
UCLASS()
class BOWANDARROWCPP_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MainCanvas;
	
	UPROPERTY(meta = (BindWidget))
	UCrossHairWidget* CrossHairWidget;

	UFUNCTION()
	void OnAimBegin();

	UFUNCTION()
	void OnAimEnd();
	
};
