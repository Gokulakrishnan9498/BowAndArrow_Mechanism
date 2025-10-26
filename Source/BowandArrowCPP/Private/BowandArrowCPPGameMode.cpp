// Copyright Epic Games, Inc. All Rights Reserved.

#include "BowandArrowCPPGameMode.h"
#include "BowandArrowCPPCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABowandArrowCPPGameMode::ABowandArrowCPPGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
