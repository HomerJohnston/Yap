// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapCharacter.h"
#include "Yap/Handles/YapSpeechHandle.h"

#include "IYapFreeSpeechHandler.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// We will pass data into the conversation handlers via structs.
// This makes it easier for users to (optionally) build blueprint functions which accept the whole chunk of data in one pin.

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Dialogue Node Entered")
struct FYapData_TalkDialogueNodeEntered
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Dialogue Node Exited")
struct FYapData_TalkDialogueNodeExited
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Dialogue Node Bypassed")
struct FYapData_TalkDialogueNodeBypassed
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Speech Begins")
struct FYapData_TalkSpeechBegins
{
	GENERATED_BODY()

	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapSpeechHandle DialogueHandleRef;

	/** Who is being speaked towards. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> DirectedAt = nullptr;

	/** Who is speaking. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> Speaker;

	/** Mood of the speaker. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodTag;

	/** Text being spoken. */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	/** Optional title text representing the dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FText TitleText;
	
	/** How long this dialogue is expected to play for. */
	UPROPERTY(BlueprintReadOnly)
	float SpeechTime = 0;

	/** Delay after this dialogue completes before carrying on. */
	UPROPERTY(BlueprintReadOnly)
	float PaddedTime = 0;

	/** Audio asset, you are responsible to cast to your proper type to use. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UObject> DialogueAudioAsset;

	/** Can this dialogue be skipped? */
	UPROPERTY(BlueprintReadOnly)
	bool bSkippable = false;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Speech Ends")
struct FYapData_TalkSpeechEnds
{
	GENERATED_BODY()

	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapSpeechHandle DialogueHandleRef;

	/** How long it is expected to wait before moving on to the next fragment or Flow Graph node. */
	UPROPERTY(BlueprintReadOnly)
	float PaddedTime = 0;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Free Speech Padding Ends")
struct FYapData_TalkSpeechPaddingEnds
{
	GENERATED_BODY()
	
	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapSpeechHandle DialogueHandleRef;

	/** Will manual advancement be required to progress? */
	UPROPERTY(BlueprintReadOnly)
	bool bManualAdvance = false;
};

// ================================================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UYapFreeSpeechHandler : public UInterface
{
    GENERATED_BODY()
};

class IYapFreeSpeechHandler
{
    GENERATED_BODY()

protected:
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Dialogue Node Entered")
    void K2_TalkDialogueNodeEntered(FYapData_TalkDialogueNodeEntered In);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Dialogue Node Exited")
    void K2_TalkDialogueNodeExited(FYapData_TalkDialogueNodeExited In);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Dialogue Node Bypassed")
    void K2_TalkDialogueNodeBypassed(FYapData_TalkDialogueNodeBypassed In);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Speech Begins")
    void K2_TalkSpeechBegins(FYapData_TalkSpeechBegins In);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Speech Ends")
    void K2_TalkSpeechEnds(FYapData_TalkSpeechEnds In);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Speech Padding Ends")
    void K2_TalkSpeechPaddingEnds(FYapData_TalkSpeechPaddingEnds In);
    
public:
    virtual void OnTalkDialogueNodeEntered(FYapData_TalkDialogueNodeEntered Event) { }

    virtual void OnTalkDialogueNodeExited(FYapData_TalkDialogueNodeExited Event) { }

    virtual void OnTalkDialogueNodeBypassed(FYapData_TalkDialogueNodeBypassed Event) { }

    virtual void OnTalkSpeechBegins(FYapData_TalkSpeechBegins Event) { }

    virtual void OnTalkSpeechEnds(FYapData_TalkSpeechEnds Event) { }

    virtual void OnTalkSpeechPaddingEnds(FYapData_TalkSpeechPaddingEnds Event) { }

};

#undef LOCTEXT_NAMESPACE