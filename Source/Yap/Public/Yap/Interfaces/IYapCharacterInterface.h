// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"

#include "IYapCharacterInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UYapCharacterInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Apply this interface to any class or blueprint which you want to make usable as a speaker in Yap.
 *
 * The NAME, COLOR, and PORTRAIT are arbitrary pieces of data you can use however you want in your game (or ignore).
 *
 * The TAG property is special; Yap can use it to try and find the character's Actor in the world (see UYapCharacterComponent).
 */
class YAP_API IYapCharacterInterface
{
    GENERATED_BODY()

    // -----------------------------------------------------
    // Blueprint Interface - Override these in a blueprint on which you've added this interface, and call these in your Blueprint Graphs
    // -----------------------------------------------------
protected:

    /** Implement this on a blueprint. Do not override this in C++. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Name")
    FText K2_GetYapCharacterName() const;
    virtual FText K2_GetYapCharacterName_Implementation() const;

    /** Implement this on a blueprint. Do not override this in C++. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Color")
    FLinearColor K2_GetYapCharacterColor() const;
    virtual FLinearColor K2_GetYapCharacterColor_Implementation() const;

    /** Implement this on a blueprint. Do not override this in C++. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Tag")
    FGameplayTag K2_GetYapCharacterTag() const;
    virtual FGameplayTag K2_GetYapCharacterTag_Implementation() const;

    /** Implement this on a blueprint. Do not override this in C++. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Portrait")
    const UTexture2D* K2_GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;
    virtual const UTexture2D* K2_GetYapCharacterPortrait_Implementation(const FGameplayTag& MoodTag) const;

    // -----------------------------------------------------
    // C++ Interface - Override these in a C++ class which inherits this interface
    // -----------------------------------------------------
protected:
    
    /** Override this on a C++ class. */
    virtual FText GetYapCharacterName() const;

    /** Override this on a C++ class. */
    virtual FLinearColor GetYapCharacterColor() const;

    /** Override this on a C++ class. */
    virtual FGameplayTag GetYapCharacterTag() const;
    
    /** Override this on a C++ class. */
    virtual const UTexture2D* GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;

    // -----------------------------------------------------
    // Public API - Use these in your C++ code (these will automatically call the C++ or K2 functions above, as appropriate)
    // -----------------------------------------------------
public:
    
    /** Try to get the supplied character's name. Will return empty text if the character does not implement the interface. */
    static FText GetName(const UObject* Character);

    /** Try to get the supplied character's color. Will return gray if the character does not implement the interface. */
    static FLinearColor GetColor(const UObject* Character);

    /** Try to get the supplied character's ID tag. Will return an empty tag if the character does not implement the interface. */
    static FGameplayTag GetTag(const UObject* Character);
    
    /** Try to get the supplied character's portrait. Will return a nullptr if the character does not implement the interface. */
    static const UTexture2D* GetPortrait(const UObject* Character, FGameplayTag MoodTag);
    
    /** Try to get the supplied character's portrait (this calls GetPortrait(...) with an empty mood tag). Will return a nullptr if the character does not implement the interface. */
    static const UTexture2D* GetPortrait(const UObject* Character);
};