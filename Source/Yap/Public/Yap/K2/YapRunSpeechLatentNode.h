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
	TObjectPtr<UObject> _WorldContext;

	UPROPERTY()
	FYapSpeechHandle _Handle;
	
public:
	UPROPERTY(BlueprintAssignable)
	FDelayOutputPin Completed;
	
	UPROPERTY(BlueprintAssignable)
	FDelayOutputPin Cancelled;

protected:
	FYapSpeechEventDelegate OnSpeechComplete;
	
	/**
	 * Run speech
	 * @param CharacterID Thing
	 * @param DialogueText Thing
	 * @param DialogueAudioAsset 
	 * @param MoodTag 
	 * @param SpeechTime 
	 * @param TitleText 
	 * @param DirectedAt 
	 * @param Conversation 
	 * @param bSkippable 
	 * @param NodeType 
	 * @param WorldContext A stupid WC
	 * @param Handle A stupid handle
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext",  AdvancedDisplay = 4))
	static UYapRunSpeechLatent2* RunSpeechLatent(
		FName CharacterID,
		FText DialogueText,
		UObject* DialogueAudioAsset,
		FGameplayTag MoodTag,
		float SpeechTime,
		FText TitleText,
		TScriptInterface<IYapCharacterInterface> DirectedAt,
		FGameplayTag Conversation,
		bool bSkippable,
		TSubclassOf<UFlowNode_YapDialogue> NodeType,
		UObject* WorldContext,
		UPARAM(Ref) FYapSpeechHandle& Handle);

	UFUNCTION()
	void OnSpeechCompleteFunc(UObject* Broadcaster, const FYapSpeechHandle& Handle, EYapSpeechCompleteResult Result);

	void Activate() override;
};
