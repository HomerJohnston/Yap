// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"

#include "IYapSpeaker.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapSpeaker : public UInterface
{
    GENERATED_BODY()
};

class YAP_API IYapSpeaker
{
    GENERATED_BODY()
    
protected:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap Speaker", DisplayName = "Get Speaker Name")
    FText K2Yap_GetSpeakerName() const;
    FText K2Yap_GetSpeakerName_Implementation() const { return Yap_GetSpeakerName(); }
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap Speaker", DisplayName = "Get Speaker Color")
    FLinearColor K2Yap_GetSpeakerColor() const;
    FLinearColor K2Yap_GetSpeakerColor_Implementation() const { return Yap_GetSpeakerColor(); };
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap Speaker", DisplayName = "Get Speaker Tag")
    FGameplayTag K2Yap_GetSpeakerTag() const;
    FGameplayTag K2Yap_GetSpeakerTag_Implementation() const { return Yap_GetSpeakerTag(); };

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Yap Speaker", DisplayName = "Get Speaker Portrait")
    const UTexture2D* K2Yap_GetSpeakerPortrait(const FGameplayTag& MoodTag) const;
    const UTexture2D* K2Yap_GetSpeakerPortrait_Implementation(const FGameplayTag& MoodTag) const { return Yap_GetSpeakerPortrait(MoodTag); };

public:

    virtual FText Yap_GetSpeakerName() const;

    virtual FLinearColor Yap_GetSpeakerColor() const;

    virtual FGameplayTag Yap_GetSpeakerTag() const;
    
    virtual const UTexture2D* Yap_GetSpeakerPortrait(const FGameplayTag& MoodTag) const;
};