// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapCharacterComponent.h"

#include "UObject/PropertyAccessUtil.h"
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

	if (bAutoRegister)
	{
		Register();
	}
}

void UYapCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegistered)
	{
		Unregister();
	}
	
	Super::EndPlay(EndPlayReason);
}

const FGameplayTag& UYapCharacterComponent::GetIdentityRootTag()
{
	return UYapProjectSettings::GetCharacterRootTag();
}

void UYapCharacterComponent::Register()
{
	GetWorld()->GetSubsystem<UYapSubsystem>()->RegisterCharacterComponent(this);
	bRegistered = true;
}

void UYapCharacterComponent::Unregister()
{
	GetWorld()->GetSubsystem<UYapSubsystem>()->UnregisterCharacterComponent(this);
	bRegistered = false;
}

#undef LOCTEXT_NAMESPACE
