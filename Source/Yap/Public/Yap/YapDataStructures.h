// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "Yap/Interfaces/IYapCharacterInterface.h"

class UYapCharacterAsset;
struct FYapPromptHandle;
struct FYapRunningFragment;
struct FYapBit;

#include "Yap/Handles/YapConversationHandle.h"

#include "YapDataStructures.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// We will pass data into the conversation handlers via structs.
// This makes it easier for users to (optionally) build blueprint functions which accept the whole chunk of data in one pin.

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conversation Opened")
struct FYapData_ConversationOpened
{
	GENERATED_BODY()

	/** Conversation name. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Speech Begins")
struct FYapData_SpeechBegins
{
	GENERATED_BODY()

	/** Conversation name. This will be empty for speech occurring outside of a conversation. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FGameplayTag Conversation;
	
	/** Who is being speaked towards. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FName DirectedAtID;

	/** Who is speaking. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	TScriptInterface<IYapCharacterInterface> Speaker;

	/** Who is speaking. This is a Name. It will match the Gameplay Tag in project settings for predefined speakers. */
	UPROPERTY(BlueprintReadWrite)
	FName SpeakerID;

	/** Mood of the speaker. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FGameplayTag MoodTag;

	/** Text being spoken. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FText DialogueText;

	/** Optional title text representing the dialogue. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FText TitleText;
	
	/** How long this dialogue is expected to play for. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	float SpeechTime = 0;

	// TODO: I'm not sure if this data belongs in here. It's here because it might be useful for UI to have... but it's irrelevant to the subsystem and actual dialogue engine, only the flow graph.
	// I'm going to try taking it out for now and I can put it back in later if it seems necessary for any real purpose.
	/** Delay after this dialogue completes before carrying on. */
    //UPROPERTY(BlueprintReadOnly, Category = "Default")
	//float FragmentTime = 0;

	/** Audio asset, you are responsible to cast to your proper type to use. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	TObjectPtr<const UObject> DialogueAudioAsset;

	/** Can this dialogue be skipped? */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	bool bSkippable = false;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompt Created")
struct FYapData_PlayerPromptCreated
{
	GENERATED_BODY()

	/** Conversation name. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FYapConversationHandle Conversation;
	
	/** Who will be spoken to. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	TScriptInterface<IYapCharacterInterface> DirectedAt;

	/** Who is going to speak. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	TScriptInterface<IYapCharacterInterface> Speaker;

	/** Who is going to speak. This is a Name. It will match the Gameplay Tag in project settings for predefined speakers. */
	UPROPERTY(BlueprintReadWrite)
	FName SpeakerName;

	/** Mood of the speaker. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FGameplayTag MoodTag;
	 
	/** Text that will be spoken. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FText DialogueText;

	/** Optional title text representing the dialogue. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FText TitleText;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompts Ready")
struct FYapData_PlayerPromptsReady
{
	GENERATED_BODY()

	/** Conversation name. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FYapConversationHandle Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompt Chosen")
struct FYapData_PlayerPromptChosen
{
	GENERATED_BODY()

	/** Conversation name. */
    UPROPERTY(BlueprintReadOnly, Category = "Default")
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE