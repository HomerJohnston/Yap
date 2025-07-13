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

FYapSpeechHandle UYapBlueprintFunctionLibrary::K2_RunSpeech(TScriptInterface<IYapCharacterInterface> Speaker, TScriptInterface<IYapCharacterInterface> DirectedAt, FText DialogueText, float SpeechTime, UObject* DialogueAudioAsset, FGameplayTag MoodTag, FText TitleText, FGameplayTag Conversation, bool bSkippable, UObject* WorldContext)
{
	UObject* WCO = WorldContext ? WorldContext : Speaker.GetObject();

	if (!IsValid(WCO))
	{
		UE_LOG(LogYap, Error, TEXT("K2_RunSpeech called with invalid WorldContext!"));
		return FYapSpeechHandle();
	}

	UYapSubsystem* Subsystem = UYapSubsystem::Get(WCO);
	
	FYapSpeechHandle NewSpeechHandle = Subsystem->GetNewSpeechHandle(FGuid::NewGuid());

	FYapData_SpeechBegins SpeechData;
	SpeechData.Speaker = Speaker;
	SpeechData.DirectedAt = DirectedAt;
	SpeechData.DialogueText = DialogueText;
	SpeechData.SpeechTime = SpeechTime;
	SpeechData.DialogueAudioAsset = DialogueAudioAsset;
	SpeechData.MoodTag = MoodTag;
	SpeechData.TitleText = TitleText;
	SpeechData.Conversation = Conversation;
	SpeechData.bSkippable = bSkippable;
	
	Subsystem->RunSpeech(SpeechData, UFlowNode_YapDialogue::StaticClass(), NewSpeechHandle);

	return NewSpeechHandle;
}

/*
FYapSpeechHandle UYapBlueprintFunctionLibrary::K2_RunSpeech(FGameplayTag Conversation, TScriptInterface<IYapCharacterInterface> DirectedAt, TScriptInterface<IYapCharacterInterface> Speaker, FGameplayTag MoodTag, FText DialogueText, FText TitleText, float SpeechTime, UObject* DialogueAudioAsset,	bool bSkippable)
{
	FYapSpeechHandle NewSpeechHandle = UYapSubsystem::Get(this)->GetNewSpeechHandle(FGuid::NewGuid());
	
	RunSpeech(SpeechData, NodeType, NewSpeechHandle);

	return NewSpeechHandle;
}
*/

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
