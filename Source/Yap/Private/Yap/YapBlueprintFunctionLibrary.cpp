// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#endif

#include "Yap/YapCharacterAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
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

AActor* UYapBlueprintFunctionLibrary::FindYapCharacterActor(UObject* WorldContext, FName Speaker)
{
	if (Speaker == NAME_None)
	{
		return nullptr;
	}
	
	UYapCharacterComponent* Comp = UYapSubsystem::FindCharacterComponent(WorldContext->GetWorld(), Speaker);

	if (!IsValid(Comp))
	{
		return nullptr;
	}

	return Comp->GetOwner();
}

FYapSpeechHandle UYapBlueprintFunctionLibrary::K2_RunSpeech(
	TScriptInterface<IYapCharacterInterface> Speaker,
	FName SpeakerName,
	FText DialogueText,
	UObject* DialogueAudioAsset,
	FGameplayTag MoodTag,
	float SpeechTime,
	FText TitleText,
	TScriptInterface<IYapCharacterInterface> DirectedAt,
	FGameplayTag Conversation,
	bool bSkippable,
	TSubclassOf<UFlowNode_YapDialogue> NodeType,
	UObject* WorldContext)
{
	UObject* WCO = WorldContext ? WorldContext : Speaker.GetObject();

	if (!IsValid(WCO))
	{
		UE_LOG(LogYap, Error, TEXT("K2_RunSpeech called with invalid WorldContext!"));
		return FYapSpeechHandle();
	}

	UYapSubsystem* Subsystem = UYapSubsystem::Get(WCO);
	
	FYapSpeechHandle NewSpeechHandle = Subsystem->GetNewSpeechHandle(FGuid::NewGuid());

	if (SpeechTime == 0.0f)
	{
		UYapBroker& Broker = UYapSubsystem::GetBroker(WCO);

		if (DialogueAudioAsset)
		{
			SpeechTime = Broker.GetAudioAssetDuration(DialogueAudioAsset);
		}
		else
		{
			const UYapNodeConfig& Config = NodeType.GetDefaultObject()->GetNodeConfig();

			int32 WordCount = Broker.CalculateWordCount(DialogueText);
			int32 CharCount = DialogueText.ToString().Len();
			SpeechTime = Broker.CalculateTextTime(WordCount, CharCount, Config);			
		}
	}
	
	FYapData_SpeechBegins SpeechData;
	SpeechData.Speaker = Speaker;
	SpeechData.SpeakerName = SpeakerName;
	SpeechData.DirectedAt = DirectedAt;
	SpeechData.DialogueText = DialogueText;
	SpeechData.SpeechTime = SpeechTime;
	SpeechData.DialogueAudioAsset = DialogueAudioAsset;
	SpeechData.MoodTag = MoodTag;
	SpeechData.TitleText = TitleText;
	SpeechData.Conversation = Conversation;
	SpeechData.bSkippable = bSkippable;
	
	Subsystem->RunSpeech(SpeechData, NodeType, NewSpeechHandle);

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
