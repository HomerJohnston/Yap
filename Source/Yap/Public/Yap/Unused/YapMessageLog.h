// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

// The goal here was to build a listview helper that uses structs. I may just delete this eventually rather than build it.

#if 0

#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YapMessageLog.generated.h"

class UYapCharacterAsset;


// ================================================================================================

/** INCOMPLETE - This is supposed to eventually be usable in a custom UListView class. */
USTRUCT(BlueprintType)
struct FYapMessageEntry
{
    GENERATED_BODY()

    FYapMessageEntry() {}

    FYapMessageEntry(UYapCharacterAsset* InSpeaker, const FText& InText);
    
protected:
    UPROPERTY(BlueprintReadOnly, Category = "Default")
    TObjectPtr<UYapCharacterAsset> Speaker;

    UPROPERTY(BlueprintReadOnly, Category = "Default")
    FText Text;

public:
    const UYapCharacterAsset* GetSpeaker() const { return Speaker; }

    const FText& GetText() const { return Text; }
};

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapConversationLog
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Default")
    TArray<FYapMessageEntry> MessageEntries;
};

// ================================================================================================

UCLASS()
class UYapMessageLog : public UGameInstanceSubsystem
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Default")
    TMap<FGameplayTag, FYapConversationLog> LoggedConversations;
};

#endif