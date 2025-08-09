// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Yap/YapDataStructures.h"
#include "Yap/Handles/YapSpeechHandle.h"

#include "YapRunSpeechLatentNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelayOutputPin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSOnAdvancedSpawnPrefabAsyncActionCreatedOutputPin, UObject*, AdvancedSpawnPrefabAsyncAction);

UCLASS()
class YAP_API UYapRunSpeechLatentNode : public UBlueprintAsyncActionBase
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
	/** Executed when the node is either succeeded OR advanced. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Finished")
	FDelayOutputPin Finished;

	/** Executed ONLY if the speech finishes naturally. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Succeeded")
	FDelayOutputPin Completed;

	/** Executed ONLY if the speech was advanced. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Advanced")
	FDelayOutputPin Advanced;

	/** Executed ONLY if the speech was cancelled. */
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
	 * @param DialogueType What dialogue type is this? Used to read config settings.
	 * @param SpeechOwner Who owns this speech object? If left unset, Yap will use 'this'.
	 * @param Handle Resulting handle, optionally used for cancelling speech.
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Run Speech", meta = (BlueprintInternalUseOnly = "true", DefaultToSelf = "SpeechOwner",  AdvancedDisplay = 4))
	static UYapRunSpeechLatentNode* RunSpeechLatent(
		UObject* SpeechOwner,
		FName CharacterID,
		FText DialogueText,
		UPARAM(meta = (YapPin = "DialogueAudioAsset")) UObject* DialogueAudioAsset,
		FGameplayTag MoodTag,
		float SpeechTime,
		FText TitleText,
		FName DirectedAtID,
		bool bSkippable,
		TSubclassOf<UFlowNode_YapDialogue> DialogueType,
		FYapSpeechHandle& Handle);

	UFUNCTION()
	void OnSpeechCompleteFunc(UObject* Broadcaster, const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result);

	UFUNCTION()
	static TArray<TSoftClassPtr<UObject>> StaticTest();
	
	void Activate() override;
};
