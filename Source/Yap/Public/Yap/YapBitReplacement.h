// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "GameplayTagContainer.h"
#include "Yap/YapBit.h"
#include "Yap/Enums/YapTimeMode.h"
#include "YapBitReplacement.generated.h"

class UObject;
class UYapCharacterAsset;

// TODO -- URGENT -- we need a details customization for FYapText. So that whenever you set the text, it caches the length.
USTRUCT()
struct FYapBitReplacement
{
	GENERATED_BODY()

	FYapBitReplacement();
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<TSoftObjectPtr<UYapCharacterAsset>> SpeakerAsset;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<TSoftObjectPtr<UYapCharacterAsset>> DirectedAtAsset;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<FYapText> MatureTitleText;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<FYapText> SafeTitleText;

	UPROPERTY(EditAnywhere, Category = "Default")
	bool bOverrideMatureDialogueText = false;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default", meta = (EditCondition = "bOverrideMatureDialogueText", EditConditionHides))
	FYapText MatureDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<FYapText> SafeDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<TSoftObjectPtr<UObject>> MatureAudioAsset;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<TSoftObjectPtr<UObject>> SafeAudioAsset;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<FGameplayTag> MoodTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, Category = "Default")
	FGameplayTag Test = FGameplayTag::EmptyTag;
	
	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<EYapTimeMode> TimeMode = EYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Default")
	TOptional<float> ManualTime = 0;
};

inline FYapBitReplacement::FYapBitReplacement()
{
	SpeakerAsset.Reset();
	DirectedAtAsset.Reset();

	// TODO investigate this more, why is FText TOptional crashing if I don't do this? x 4
	//MatureTitleText->Set(FText::GetEmpty());
	MatureTitleText.Reset();

	//SafeTitleText->Set(FText::GetEmpty());
	SafeTitleText.Reset();

	//MatureDialogueText->Set(FText::GetEmpty());
	//MatureDialogueText.Reset();
	
	//SafeDialogueText->Set(FText::GetEmpty());
	SafeDialogueText.Reset();
	
	MatureAudioAsset.Reset();
	SafeAudioAsset.Reset();
	
	MoodTag.Reset();
	TimeMode.Reset();

	ManualTime.Reset();
}
