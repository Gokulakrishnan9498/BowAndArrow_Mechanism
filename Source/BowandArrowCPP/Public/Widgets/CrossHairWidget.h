// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "CrossHairWidget.generated.h"

/**
 * 
 */
UCLASS()
class BOWANDARROWCPP_API UCrossHairWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnDrawUpdate(float DrawTime, float MaxDrawTime);

	UFUNCTION()
	void OnDrawEnd();

	UPROPERTY(meta=(BindWidget))
	UImage* OuterCircle;

	UPROPERTY(meta=(BindWidget))
	UImage* Dot;

	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* Canvas;

	UPROPERTY()
	bool bHasPlayedFullDraw;

	UPROPERTY(EditAnywhere, Transient, BlueprintReadWrite, meta=(BindWidgetAnim), Category="CrossHair")
	UWidgetAnimation* PulseAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CrossHair")
	USoundBase* FullDrawnSound;
	
};
