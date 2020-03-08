// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "TindalosGameMode.h"
#include "TindalosPawn.h"
#include "UObject/ConstructorHelpers.h"

ATindalosGameMode::ATindalosGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Mannequin/MyHeroCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

