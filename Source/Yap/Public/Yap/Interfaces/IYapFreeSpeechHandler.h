// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "Yap/Handles/YapRunningFragment.h"
#include "Yap/YapCharacter.h"
#include "Yap/Handles/YapSpeechHandle.h"
#include "Yap/YapDataStructures.h"

#include "IYapFreeSpeechHandler.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

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
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "Talk Speech Begins")
    void K2_TalkSpeechBegins(FYapData_SpeechBegins In, FYapSpeechHandle Handle);
    
public:
    virtual void OnTalkSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle)
    {
	    K2_TalkSpeechBegins(Data, Handle);
    }
};

#undef LOCTEXT_NAMESPACE