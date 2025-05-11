// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/YapDataStructures.h"

#include "IYapFreeSpeechHandler.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================

/**
 * A Free Speech Handler is an interface you can apply to anything to help it respond to Yap dialogue.
 * This ONLY responds to dialogue nodes that are running on a Flow Graph which has NOT had an "Open Conversation" node run.
 * To respond to dialogue nodes that are running after opening a conversation, see IYapConversationHandler.
 * Use UYapSubsystem::RegisterFreeSpeechHandler(...) to register your class for events. 
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UYapFreeSpeechHandler : public UInterface
{
    GENERATED_BODY()
};

class IYapFreeSpeechHandler
{
    GENERATED_BODY()

    // -----------------------------------------------------
    // Blueprint Interface - Override these in a blueprint on which you've added this interface
    // -----------------------------------------------------
protected:
    
 	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Speech Begins")
    void K2_TalkSpeechBegins(FYapData_SpeechBegins In, FYapSpeechHandle Handle);
    
    // -----------------------------------------------------
    // C++ Interface - Override these in a C++ class which inherits this interface
    // -----------------------------------------------------
public:
    
    /** Code to run when a piece of dialogue (speech) begins. Do NOT call Super when overriding. */
    YAP_API virtual void OnTalkSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle)
    {
	    K2_TalkSpeechBegins(Data, Handle);
    }
};

#undef LOCTEXT_NAMESPACE