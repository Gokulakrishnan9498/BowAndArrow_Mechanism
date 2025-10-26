// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "BowandArrowCPPCharacter.generated.h"

class UBowMechanics;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ABowandArrowCPPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Bow, meta = (AllowPrivateAccess = "true"))
	UBowMechanics* BowMechanicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* AimTimeline;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta =(AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* KillCamAction;

public:
	ABowandArrowCPPCharacter();
	

protected:

	//Aim Variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	float Initial_FOV;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	float Aim_FOV;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	FVector IntialBoomOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	FVector AimedBoomOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	FRotator InitialMeshRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aim)
	FRotator AimedMeshRotation;

	//Curve for the timeline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Aim)
	UCurveFloat* AimCurve;

	//Timeline Delegate
	FOnTimelineFloat OnAimTimelineProgress;
	FOnTimelineEvent OnAimTimelineFinished;

	// Timeline callback functions
	UFUNCTION()
	void AimTimelineUpdate(float Alpha);

	UFUNCTION()
	void AimTimelineFinished();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void AimCameraBegin();

	UFUNCTION()
	void AimCameraEnd();

	UFUNCTION()
	void SetupTimeline();

	UFUNCTION()
	void ActivateKillCam();

	UFUNCTION()
	void DeActivateKillCam();
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();



public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

