// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapCharacterComponent.h"

#include "UObject/PropertyAccessUtil.h"
#include "Yap/YapCharacterManager.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

UYapCharacterComponent::UYapCharacterComponent()
{
	AddGameplayTagFilter(GET_MEMBER_NAME_CHECKED(ThisClass, Identity), FGameplayTagFilterDelegate::CreateStatic(&GetIdentityRootTag));

	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UYapCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bComponentRegistered)
	{
		GetWorld()->GetSubsystem<UYapSubsystem>()->RegisterCharacterComponent(this);
		bComponentRegistered = true;
	}
	
	if (!bPreventAutoRegisterCharacterDefinition && Identity.UsesCustomID())
	{
		RegisterCharacterDefinition();
	}
}

void UYapCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bComponentRegistered)
	{
		GetWorld()->GetSubsystem<UYapSubsystem>()->UnregisterCharacterComponent(this);
		bComponentRegistered = false;
	}
	
	if (!bPreventAutoRegisterCharacterDefinition && Identity.UsesCustomID())
	{
		UnregisterCharacterDefinition();
	}
	
	Super::EndPlay(EndPlayReason);
}

const FGameplayTag& UYapCharacterComponent::GetIdentityRootTag()
{
	return UYapProjectSettings::GetCharacterRootTag();
}

void UYapCharacterComponent::RegisterCharacterDefinition()
{
	if (bDefinitionRegistered)
	{
		return;
	}
	
	UYapCharacterManager& CharacterManager = UYapSubsystem::GetCharacterManager(this);
	CharacterManager.RegisterCharacter(Identity.Get(), CharacterDefinition, true);
	bDefinitionRegistered = true;
}

void UYapCharacterComponent::UnregisterCharacterDefinition()
{
	if (!bDefinitionRegistered)
	{
		return;
	}
	
	UYapCharacterManager& CharacterManager = UYapSubsystem::GetCharacterManager(this);
	CharacterManager.UnregisterCharacter(Identity.Get());
	bDefinitionRegistered = false;
}

#undef LOCTEXT_NAMESPACE
