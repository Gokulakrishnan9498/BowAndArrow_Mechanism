// Copyright Epic Games, Inc. All Rights Reserved.

#include "BowandArrowCPPCharacter.h"

#include "BowMechanics.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ABowandArrowCPPCharacter

ABowandArrowCPPCharacter::ABowandArrowCPPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	BowMechanicsComponent = CreateDefaultSubobject<UBowMechanics>(TEXT("BowMechanics"));

	AimTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimTimeline"));

	Aim_FOV = 60.0f;

	AimedBoomOffset = FVector(100.0f, 75.0f, 50.0f);

	AimedMeshRotation = FRotator(0.0f, 285.0f, 0.0f);
}

void ABowandArrowCPPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (FollowCamera)
	{
		Initial_FOV = FollowCamera->FieldOfView;
	}

	if (CameraBoom)
	{
		IntialBoomOffset = CameraBoom->SocketOffset;
	}

	InitialMeshRotation = GetMesh()->GetRelativeRotation();
		
	SetupTimeline();
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABowandArrowCPPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABowandArrowCPPCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABowandArrowCPPCharacter::Look);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, BowMechanicsComponent, &UBowMechanics::AimBegin);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABowandArrowCPPCharacter::AimCameraBegin);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, BowMechanicsComponent, &UBowMechanics::AimEnd);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABowandArrowCPPCharacter::AimCameraEnd);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, BowMechanicsComponent, &UBowMechanics::DrawBegin);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, BowMechanicsComponent, &UBowMechanics::FireArrowBegin);

		// Kill Cam
		EnhancedInputComponent->BindAction(KillCamAction, ETriggerEvent::Started, this, &ABowandArrowCPPCharacter::ActivateKillCam);
		EnhancedInputComponent->BindAction(KillCamAction, ETriggerEvent::Completed, this, &ABowandArrowCPPCharacter::DeActivateKillCam);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABowandArrowCPPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABowandArrowCPPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void ABowandArrowCPPCharacter::SetupTimeline()
{
	if (AimTimeline && AimCurve)
	{
		OnAimTimelineProgress.BindUFunction(this, FName("AimTimelineUpdate"));
		OnAimTimelineFinished.BindUFunction(this, FName("AimTimelineFinished"));

		AimTimeline->AddInterpFloat(AimCurve, OnAimTimelineProgress);

		AimTimeline->SetTimelineFinishedFunc(OnAimTimelineFinished);

		AimTimeline->SetLooping(false);
		AimTimeline->SetIgnoreTimeDilation(false);
	}
	
}


void ABowandArrowCPPCharacter::AimTimelineUpdate(float Alpha)
{
	if (!FollowCamera || !CameraBoom)
	{
		return;
	}

	// Interpolate FOV between initial and aim FOV
	float NewFov = UKismetMathLibrary::Lerp(Initial_FOV, Aim_FOV, Alpha);
	FollowCamera->SetFieldOfView(NewFov);

	// Interpolate camera boom socket offset
	FVector NewSocketOffset = UKismetMathLibrary::VLerp(IntialBoomOffset, AimedBoomOffset, Alpha);
	CameraBoom->SocketOffset = NewSocketOffset;

	// You can add more interpolations here (camera position, rotation, etc.)
    
	UE_LOG(LogTemp, Warning, TEXT("Aim timeline update - Alpha: %f, FOV: %f"), Alpha, NewFov);
}

void ABowandArrowCPPCharacter::AimTimelineFinished()
{
		UE_LOG(LogTemp, Log, TEXT("Aim transition completed - Now aiming"));
}


void ABowandArrowCPPCharacter::AimCameraBegin()
{
	if (!AimTimeline || !AimCurve)
	{
		return;
	}
	GetMesh()->SetRelativeRotation(AimedMeshRotation);
	AimTimeline->PlayFromStart();

	UE_LOG(LogTemp, Log, TEXT("Starting aim camera transition"));

	
	
}

void ABowandArrowCPPCharacter::AimCameraEnd()
{
	if (!AimTimeline || !AimCurve)
	{
		return;
	}
	// Reverse timeline to return to normal camera
	AimTimeline->Reverse();
    
	UE_LOG(LogTemp, Log, TEXT("Ending aim camera transition"));

	GetMesh()->SetRelativeRotation(InitialMeshRotation);
}

void ABowandArrowCPPCharacter::ActivateKillCam()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 0.3f);

	if (IsValid(BowMechanicsComponent->FiredArrowRef))
	{
		UGameplayStatics::GetPlayerController(this, 0)->SetViewTargetWithBlend(BowMechanicsComponent->FiredArrowRef, 0.1f, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, false);
	}
}

void ABowandArrowCPPCharacter::DeActivateKillCam()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	UGameplayStatics::GetPlayerController(this, 0)->SetViewTargetWithBlend(this, 0.1f, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, false);
}
