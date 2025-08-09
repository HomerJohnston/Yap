// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/K2/YapRunSpeechLatentNode.h"

#include "Yap/YapCharacterManager.h"
#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"

UYapRunSpeechLatentNode* UYapRunSpeechLatentNode::RunSpeechLatent(
	UObject* SpeechOwner,
	FName CharacterID,
	FText DialogueText,
	UObject* DialogueAudioAsset,
	// ADVANCED DISPLAY
	FGameplayTag MoodTag,
	float SpeechTime,
	FText TitleText,
	FName DirectedAt,
	bool bSkippable,
	TSubclassOf<UFlowNode_YapDialogue> DialogueType,
	UPARAM(ref) FYapSpeechHandle& Handle)
{
	UYapRunSpeechLatentNode* Node = NewObject<UYapRunSpeechLatentNode>();

	Node->RegisterWithGameInstance(SpeechOwner);

	// Attempt to pull an ID off of a Yap Character Component on the incoming object
	if (CharacterID == NAME_None)
	{
		if (AActor* Actor = Cast<AActor>(SpeechOwner))
		{
			if (UYapCharacterComponent* CharacterComponent = Actor->FindComponentByClass<UYapCharacterComponent>())
			{
				CharacterID = CharacterComponent->GetCharacterID();
			}
		}
	}

	if (CharacterID == NAME_None)
	{
		return nullptr;
	}

	if (SpeechTime == 0.0f)
	{
		// TODO calculate time from audio or text using broker
	}

	Node->Data.SpeakerID = CharacterID;
	Node->Data.Speaker = UYapSubsystem::GetCharacterManager(SpeechOwner).FindCharacter(CharacterID);
	Node->Data.DialogueText = DialogueText;
	Node->Data.DialogueAudioAsset =  DialogueAudioAsset;
	Node->Data.MoodTag = MoodTag;
	Node->Data.SpeechTime = SpeechTime;
	Node->Data.TitleText = TitleText;
	Node->Data.DirectedAtID = DirectedAt;
	Node->Data.bSkippable = bSkippable;
	Node->_NodeType = DialogueType;
	Node->_SpeechOwner = SpeechOwner;

	UYapSubsystem* Subsystem = UYapSubsystem::Get(SpeechOwner);

	Handle = Subsystem->GetNewSpeechHandle(CharacterID, SpeechOwner, nullptr);
	Node->_Handle = Handle;

	return Node;
}

void UYapRunSpeechLatentNode::Activate()
{
	UYapSubsystem* Subsystem = UYapSubsystem::Get(_SpeechOwner);

	UE_LOG(LogYap, VeryVerbose, TEXT("RunSpeechLatent activate... Running speech: %s <%s>"), *Data.DialogueText.ToString(), *_Handle.ToString());
	
	Subsystem->RunSpeech(Data, _NodeType, _Handle);

	OnSpeechComplete.BindDynamic(this, &ThisClass::OnSpeechCompleteFunc);

	//UYapSubsystem::Get(_WorldContext)->OnCancelDelegate.AddDynamic(this, &ThisClass::OnSpeechCancelledFunc);

	UYapSpeechHandleBFL::BindToOnSpeechComplete(_SpeechOwner, _Handle, OnSpeechComplete);
}

void UYapRunSpeechLatentNode::OnSpeechCompleteFunc(UObject* Broadcaster, const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result)
{
	SetReadyToDestroy();

	UE_LOG(LogYap, VeryVerbose, TEXT("RunSpeechLatent completed! <%s>"), *Handle.ToString());
	
	switch (Result)
	{
		case EYapSpeechCompleteResult::Advanced:
		{
			Finished.Broadcast();
			Advanced.Broadcast();
			break;
		}
		case EYapSpeechCompleteResult::Normal:
		{
			Finished.Broadcast();
			Completed.Broadcast();
			break;
		}
		case EYapSpeechCompleteResult::Cancelled:
		{
			Cancelled.Broadcast();
			break;
		}
		default:
		{
			UE_LOG(LogYap, Error, TEXT("Run Speech Latent node finished with undefined result! This should never happen!"));
		}
	}

	// I would prefer to invalidate the incoming handle here for correctness, but I can't. Blueprint won't modify the original reference at the end of a latent node, it just treats it like a copy.
	_Handle.Invalidate();
}
