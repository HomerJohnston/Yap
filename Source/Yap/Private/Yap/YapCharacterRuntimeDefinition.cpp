// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCharacterRuntimeDefinition.h"

#include "Yap/YapCharacterAsset.h"

void FYapCharacterRuntimeDefinition::InitializeCharacter(UYapCharacterAsset* Character) const
{
	Character->SetName(Name);
	Character->SetPortrait(DefaultPortrait);
	Character->SetColor(Color);
}
