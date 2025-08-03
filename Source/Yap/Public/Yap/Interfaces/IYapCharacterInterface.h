// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IYapCharacterInterface.generated.h"

class UFlowNode_YapDialogue;

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UYapCharacterInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Apply this interface to any class or blueprint which you want to make usable as a speaker in Yap.
 * The NAME, COLOR, and PORTRAIT are arbitrary pieces of data you can use however you want in your game (or ignore). // TODO - change name to a FName
 * The TAG property is special; Yap can use it to try and find the character's Actor in the world (see UYapCharacterComponent). // TODO - consider removing tag?
 */
class YAP_API IYapCharacterInterface
{
    GENERATED_BODY()

public:
    // -----------------------------------------------------
    // Public C++ API - Use these in your C++ code (these will automatically call the C++ or K2 functions below, as appropriate)
    // -----------------------------------------------------
    
    /** Try to get the supplied character's name. Will log an error if the character does not implement the interface. */
    static FText GetName(const UObject* Character);

    /** Try to get the supplied character's color. Will log an error if the character does not implement the interface. */
    static FLinearColor GetColor(const UObject* Character);

    /** Try to get the supplied character's portrait. Will log an error if the character does not implement the interface. */
    static const UTexture2D* GetPortrait(const UObject* Character, FGameplayTag MoodTag = FGameplayTag::EmptyTag);

public:
    // -----------------------------------------------------
    // Public C++ Interface - Override these in a C++ class which inherits this interface, and use if you implement the interface solely in C++ in your game.
    // -----------------------------------------------------
    
    /** Override this on a C++ class. */
    virtual FText GetCharacterName() const;

    /** Override this on a C++ class. */
    virtual FLinearColor GetCharacterColor() const;

    /** Override this on a C++ class. Pass in nullptr for the dialogue node type to use the default Yap Node type. */
    virtual const UTexture2D* GetCharacterPortrait(const FGameplayTag& MoodTag) const;

protected:
    // -----------------------------------------------------
    // Blueprint Interface - Override these in a blueprint on which you've added this interface
    // -----------------------------------------------------

    /** Implement this on a blueprint. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Yap|Character", DisplayName = "Get Name")
    FText K2_GetYapCharacterName() const;

    /** Implement this on a blueprint. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Yap|Character", DisplayName = "Get Color")
    FLinearColor K2_GetYapCharacterColor() const;

    /** Implement this on a blueprint. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Yap|Character", DisplayName = "Get Portrait")
    const UTexture2D* K2_GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;

#if WITH_EDITOR
public:
    static bool IsAsset_YapCharacter(const TSoftObjectPtr<UObject> Asset);
    
    static bool IsAsset_YapCharacter(const TSoftClassPtr<UObject> Class);
    
    static bool IsAsset_YapCharacter(const FAssetData& AssetData);
#endif
};

/**
 * Use these functions in blueprint to read Yap Characters.
 */
UCLASS(DisplayName = "Yap Character Function Library")
class UYapCharacterBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    /** Get the supplied character's name. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Yap|Character")
    static FText GetName(const TScriptInterface<IYapCharacterInterface> Character)
    {
        return IYapCharacterInterface::GetName(Character.GetObject());
    }

    /** Get the supplied character's color. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Yap|Character")
    static FLinearColor GetColor(const TScriptInterface<IYapCharacterInterface> Character)
    {
        return IYapCharacterInterface::GetColor(Character.GetObject());
    }
    
    /** Get the supplied character's portrait. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Yap|Character")
    static const UTexture2D* GetPortrait(const TScriptInterface<IYapCharacterInterface> Character, FGameplayTag MoodTag)
    {
        return IYapCharacterInterface::GetPortrait(Character.GetObject(), MoodTag);
    }
};