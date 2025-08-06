// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/K2/YapRunSpeechLatentNode.h"

#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"

/*
void UYapLatentLibrary::RunSpeechLatent(UObject* WorldContext, FLatentActionInfo LatentInfo)
{
	if (UWorld* World = WorldContext->GetWorld())
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		if (LatentActionManager.FindExistingAction<FYapRunSpeechLatent>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			
			
		}
		else
		{
			
		}
	}
}
*/

UYapRunSpeechLatent2* UYapRunSpeechLatent2::RunSpeechLatent(
		UObject* SpeechOwner,
		FName CharacterID, FText DialogueText,
	UObject* DialogueAudioAsset, FGameplayTag MoodTag, float SpeechTime, FText TitleText,
	FName DirectedAt, bool bSkippable,
	TSubclassOf<UFlowNode_YapDialogue> NodeType,
	UPARAM(ref) FYapSpeechHandle& Handle)
{
	UYapRunSpeechLatent2* Node = NewObject<UYapRunSpeechLatent2>();

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
	
	Node->Data.SpeakerID = CharacterID;
	Node->Data.DialogueText = DialogueText;
	Node->Data.DialogueAudioAsset =  DialogueAudioAsset;
	Node->Data.MoodTag = MoodTag;
	Node->Data.SpeechTime = SpeechTime;
	Node->Data.TitleText = TitleText;
	Node->Data.DirectedAtID = DirectedAt;
	Node->Data.bSkippable = bSkippable;
	Node->_NodeType = NodeType;
	Node->_SpeechOwner = SpeechOwner;

	UYapSubsystem* Subsystem = UYapSubsystem::Get(SpeechOwner);

	Handle = Subsystem->GetNewSpeechHandle(CharacterID, SpeechOwner, nullptr);
	Node->_Handle = Handle;

	return Node;
}

void UYapRunSpeechLatent2::OnSpeechCompleteFunc(UObject* Broadcaster, const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result)
{
	SetReadyToDestroy();

	UE_LOG(LogYap, VeryVerbose, TEXT("RunSpeechLatent completed! <%s>"), *Handle.ToString());

	AnyCompletion.Broadcast();
	
	switch (Result)
	{
		case EYapSpeechCompleteResult::Normal:
		{
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

	// I would prefer to invalidate the handle here, but I can't. Blueprint won't modify the original reference at the end of a latent node, it just treats it like a copy.
}

void UYapRunSpeechLatent2::Activate()
{
	UYapSubsystem* Subsystem = UYapSubsystem::Get(_SpeechOwner);

	UE_LOG(LogYap, VeryVerbose, TEXT("RunSpeechLatent activate... Running speech: %s <%s>"), *Data.DialogueText.ToString(), *_Handle.ToString());
	
	Subsystem->RunSpeech(Data, _NodeType, _Handle);

	OnSpeechComplete.BindDynamic(this, &ThisClass::OnSpeechCompleteFunc);

	//UYapSubsystem::Get(_WorldContext)->OnCancelDelegate.AddDynamic(this, &ThisClass::OnSpeechCancelledFunc);

	UYapSpeechHandleBFL::BindToOnSpeechComplete(_SpeechOwner, _Handle, OnSpeechComplete);
}
