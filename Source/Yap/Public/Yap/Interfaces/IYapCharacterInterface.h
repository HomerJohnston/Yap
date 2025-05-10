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
    
protected:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Name")
    FText K2_GetYapCharacterName() const;
    FText K2_GetYapCharacterName_Implementation() const { return GetYapCharacterName(); }
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Color")
    FLinearColor K2_GetYapCharacterColor() const;
    FLinearColor K2_GetYapCharacterColor_Implementation() const { return GetYapCharacterColor(); };
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Tag")
    FGameplayTag K2_GetYapCharacterTag() const;
    FGameplayTag K2_GetYapCharacterTag_Implementation() const { return GetYapCharacterTag(); };

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap|Character", DisplayName = "Get Portrait")
    const UTexture2D* K2_GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;
    const UTexture2D* K2_GetYapCharacterPortrait_Implementation(const FGameplayTag& MoodTag) const { return GetYapCharacterPortrait(MoodTag); };

public:

    virtual FText GetYapCharacterName() const;

    virtual FLinearColor GetYapCharacterColor() const;

    virtual FGameplayTag GetYapCharacterTag() const;
    
    virtual const UTexture2D* GetYapCharacterPortrait(const FGameplayTag& MoodTag) const;
};