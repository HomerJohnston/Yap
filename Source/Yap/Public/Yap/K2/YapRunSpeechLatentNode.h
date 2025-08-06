// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Yap/YapDataStructures.h"
#include "Yap/Handles/YapSpeechHandle.h"

#include "YapRunSpeechLatentNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelayOutputPin);

// A comment above the delegate declaration
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSOnAdvancedSpawnPrefabAsyncActionCreatedOutputPin, UObject*, AdvancedSpawnPrefabAsyncAction);

UCLASS(BlueprintType)
class ULolOmg : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UYapRunSpeechLatent2 : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FYapData_SpeechBegins Data;

	UPROPERTY()
	TSubclassOf<UFlowNode_YapDialogue> _NodeType;

	UPROPERTY()
	TObjectPtr<UObject> _SpeechOwner;

	UPROPERTY()
	FYapSpeechHandle _Handle;
	
public:
	/** Executed when the node is done, regardless of whether speech ran fully or was cancelled. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Finished")
	FDelayOutputPin AnyCompletion;

	/** Executed ONLY if the speech finishes naturally. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Succeeded")
	FDelayOutputPin Completed;

	/** Executed ONLY if the speech was interrupted/cancelled. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Cancelled")
	FDelayOutputPin Cancelled;

protected:
	FYapSpeechEventDelegate OnSpeechComplete;
	
	/**
	 * Run speech
	 * @param CharacterID Who will be speaking? If left unset, Yap will look for a Yap Character Component on 'this' to use.
	 * @param DialogueText Text to be spoken.
	 * @param DialogueAudioAsset Audio asset to play.
	 * @param MoodTag Mood tag.
	 * @param SpeechTime How long to play the speech. If left unset, Yap will calculate length from the Audio Asset or the Dialogue Text. // TODO 
	 * @param TitleText Title text info to include in speech data.
	 * @param DirectedAtID Who we are speaking to, if applicable. 
	 * @param bSkippable Can this speech be interrupted/cancelled/skipped?
	 * @param NodeType What dialogue type is this? Used to read config settings.
	 * @param SpeechOwner Who owns this speech object? If left unset, Yap will use 'this'.
	 * @param Handle Resulting handle, optionally used for cancelling speech.
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "SpeechOwner",  AdvancedDisplay = 4))
	static UYapRunSpeechLatent2* RunSpeechLatent(
		UObject* SpeechOwner,
		FName CharacterID,
		FText DialogueText,
		UObject* DialogueAudioAsset,
		FGameplayTag MoodTag,
		float SpeechTime,
		FText TitleText,
		FName DirectedAtID,
		bool bSkippable,
		TSubclassOf<UFlowNode_YapDialogue> NodeType,
		FYapSpeechHandle& Handle);

	UFUNCTION()
	void OnSpeechCompleteFunc(UObject* Broadcaster, const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result);

	void Activate() override;
};
