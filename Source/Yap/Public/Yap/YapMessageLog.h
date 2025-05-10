// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"

#include "YapMessageLog.generated.h"

class UYapCharacter;

// ================================================================================================

/** INCOMPLETE - This is supposed to eventually be usable in a custom UListView class. */
USTRUCT(BlueprintType)
struct FYapMessageEntry
{
    GENERATED_BODY()

    FYapMessageEntry() {}

    FYapMessageEntry(UYapCharacter* InSpeaker, const FText& InText);
    
protected:
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UYapCharacter> Speaker;

    UPROPERTY(BlueprintReadOnly)
    FText Text;

public:
    const UYapCharacter* GetSpeaker() const { return Speaker; }

    const FText& GetText() const { return Text; }
};

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapConversationLog
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly)
    TArray<FYapMessageEntry> MessageEntries;
};

// ================================================================================================

UCLASS()
class UYapMessageLog : public UGameInstanceSubsystem
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly)
    TMap<FGameplayTag, FYapConversationLog> LoggedConversations;
};
