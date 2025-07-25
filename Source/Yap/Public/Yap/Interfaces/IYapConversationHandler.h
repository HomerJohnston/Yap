// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/Handles/YapConversationHandle.h"
#include "Yap/Handles/YapSpeechHandle.h"
#include "UObject/Interface.h"

class UYapCharacterAsset;
struct FYapPromptHandle;
struct FYapRunningFragment;
struct FYapBit;

#include "Yap/YapDataStructures.h"
#include "Yap/Handles/YapPromptHandle.h"

#include "IYapConversationHandler.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

UINTERFACE(MinimalAPI, Blueprintable)
class UYapConversationHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * A Conversation Handler is an interface you can apply to anything to help it respond to Yap dialogue.
 * This ONLY responds to dialogue nodes that are running on a Flow Graph which has had an "Open Conversation" node run.
 * To respond to dialogue nodes running on a graph without opening a conversation see IYapFreeSpeechHandler.
 * Use UYapSubsystem::RegisterConversationHandler(...) to register your class for events. 
 */
class IYapConversationHandler
{
	GENERATED_BODY()
	
	// -----------------------------------------------------
	// Blueprint Interface - Override these in a blueprint on which you've added this interface
	// -----------------------------------------------------
protected:
	
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Chat Opened")
	void K2_ConversationOpened(FYapData_ConversationOpened Data, FYapConversationHandle Handle);
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Speech Begins")
	void K2_ConversationSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompt Created")
	void K2_ConversationPlayerPromptCreated(FYapData_PlayerPromptCreated Data, FYapPromptHandle Handle);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompts Ready")
	void K2_ConversationPlayerPromptsReady(FYapData_PlayerPromptsReady Data);

	/** Code to run when a player prompt is ran. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompt Chosen")
	void K2_ConversationPlayerPromptChosen(FYapData_PlayerPromptChosen Data, FYapPromptHandle Handle);
	
	// -----------------------------------------------------
	// C++ Interface - Override these in a C++ class which inherits this interface
	// -----------------------------------------------------
public:
	
	/** Code to run when a conversation begins. Do NOT call Super when overriding. */
	YAP_API virtual void OnConversationOpened(FYapData_ConversationOpened Data, FYapConversationHandle Handle)
	{
		K2_ConversationOpened(Data, Handle);
	};
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Super when overriding. */
	YAP_API virtual void OnConversationSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle)
	{
		K2_ConversationSpeechBegins(Data, Handle);
	}
	
	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Super when overriding. */
	YAP_API virtual void OnConversationPlayerPromptCreated(FYapData_PlayerPromptCreated Data, FYapPromptHandle Handle)
	{
		K2_ConversationPlayerPromptCreated(Data, Handle);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	YAP_API virtual void OnConversationPlayerPromptsReady(FYapData_PlayerPromptsReady Data)
	{
		K2_ConversationPlayerPromptsReady(Data);
	}
	
	/** Code to run when a player prompt is ran. Do NOT call Super when overriding. */
	YAP_API virtual void OnConversationPlayerPromptChosen(FYapData_PlayerPromptChosen Data, FYapPromptHandle Handle)
	{
		K2_ConversationPlayerPromptChosen(Data, Handle);
	}
};

#undef LOCTEXT_NAMESPACE