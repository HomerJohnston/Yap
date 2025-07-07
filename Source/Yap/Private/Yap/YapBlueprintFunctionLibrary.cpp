// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#endif

#include "Yap/YapCharacterAsset.h"
#include "Yap/YapLog.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UYapBlueprintFunctionLibrary::PlaySoundInEditor(USoundBase* Sound)
{
	if (Sound)
	{
		GEditor->PlayPreviewSound(Sound);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Sound was null"));
	}
}
#endif

// ------------------------------------------------------------------------------------------------

float UYapBlueprintFunctionLibrary::GetSoundLength(USoundBase* Sound)
{
	return Sound->Duration;
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::RegisterConversationHandler(UObject* NewHandler, TSubclassOf<UFlowNode_YapDialogue> NodeType)
{
	UYapSubsystem::RegisterConversationHandler(NewHandler, NodeType);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::RegisterFreeSpeechHandler(UObject* NewHandler, TSubclassOf<UFlowNode_YapDialogue> NodeType)
{
	UYapSubsystem::RegisterFreeSpeechHandler(NewHandler, NodeType);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::UnregisterConversationHandler(UObject* HandlerToUnregister, TSubclassOf<UFlowNode_YapDialogue> NodeType)
{
	UYapSubsystem::UnregisterConversationHandler(HandlerToUnregister, NodeType);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::UnregisterFreeSpeechHandler(UObject* HandlerToUnregister, TSubclassOf<UFlowNode_YapDialogue> NodeType)
{
	UYapSubsystem::UnregisterFreeSpeechHandler(HandlerToUnregister, NodeType);
}

// ------------------------------------------------------------------------------------------------

AActor* UYapBlueprintFunctionLibrary::FindYapCharacterActor(UObject* WorldContext, TScriptInterface<IYapCharacterInterface> Speaker)
{
	if (!IsValid(Speaker.GetObject()))
	{
		return nullptr;
	}

	FGameplayTag Tag = IYapCharacterInterface::GetTag(Speaker.GetObject());
	
	if (!Tag.IsValid())
	{
		return nullptr;
	}
	
	UYapCharacterComponent* Comp = UYapSubsystem::FindCharacterComponent(WorldContext->GetWorld(), Tag);

	if (!IsValid(Comp))
	{
		return nullptr;
	}

	return Comp->GetOwner();
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
