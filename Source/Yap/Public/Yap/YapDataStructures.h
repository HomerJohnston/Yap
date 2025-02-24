// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

class UYapCharacter;
struct FYapPromptHandle;
struct FYapRunningFragment;
struct FYapBit;

#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Handles/YapRunningFragment.h"

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
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Speech Begins")
struct FYapData_SpeechBegins
{
	GENERATED_BODY()

	/** Conversation name. This will be empty for speech occurring outside of a conversation. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;

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
	float FragmentTime = 0;

	/** Audio asset, you are responsible to cast to your proper type to use. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UObject> DialogueAudioAsset;

	/** Can this dialogue be skipped? */
	UPROPERTY(BlueprintReadOnly)
	bool bSkippable = false;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompt Created")
struct FYapData_PlayerPromptCreated
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;

	/** Who will be spoken to. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> DirectedAt;

	/** Who is going to speak. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> Speaker;

	/** Mood of the speaker. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodTag;
	 
	/** Text that will be spoken. */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	/** Optional title text representing the dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FText TitleText;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompts Ready")
struct FYapData_PlayerPromptsReady
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Player Prompt Chosen")
struct FYapData_PlayerPromptChosen
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE