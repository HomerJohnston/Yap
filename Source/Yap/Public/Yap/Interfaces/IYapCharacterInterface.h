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

class YAP_API IYapCharacterInterface
{
    GENERATED_BODY()

    // -----------------------------------------------------
    // Blueprint Interface - Override these in a blueprint on which you've added this interface, and call these in your Blueprint Graphs
    // -----------------------------------------------------
protected:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Name")
    FText K2_GetYapCharacterName() const;
    virtual FText K2_GetYapCharacterName_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Color")
    FLinearColor K2_GetYapCharacterColor() const;
    virtual FLinearColor K2_GetYapCharacterColor_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Tag")
    FGameplayTag K2_GetYapCharacterTag() const;
    virtual FGameplayTag K2_GetYapCharacterTag_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Portrait")
    const UTexture2D* K2_GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;
    virtual const UTexture2D* K2_GetYapCharacterPortrait_Implementation(const FGameplayTag& MoodTag) const;

    // -----------------------------------------------------
    // C++ Interface - Override these in a C++ class which inherits this interface
    // -----------------------------------------------------
protected:
    virtual FText GetYapCharacterName() const;

    virtual FLinearColor GetYapCharacterColor() const;

    virtual FGameplayTag GetYapCharacterTag() const;
    
    virtual const UTexture2D* GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;

    // -----------------------------------------------------
    // Public API - Use these in your C++ code (these will automatically call the C++ or K2 functions above, as appropriate)
    // -----------------------------------------------------
public:
    static FText GetName(const UObject* Character);

    static FLinearColor GetColor(const UObject* Character);

    static FGameplayTag GetTag(const UObject* Character);

    static const UTexture2D* GetPortrait(const UObject* Character, FGameplayTag MoodTag);
};