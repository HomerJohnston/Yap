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
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UYapCharacterAsset>> SpeakerAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UYapCharacterAsset>> DirectedAtAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FYapText> MatureTitleText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FYapText> SafeTitleText;

	UPROPERTY(EditAnywhere)
	bool bOverrideMatureDialogueText = false;
	
	/**  */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideMatureDialogueText", EditConditionHides))
	FYapText MatureDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FYapText> SafeDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UObject>> MatureAudioAsset;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UObject>> SafeAudioAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FGameplayTag> MoodTag = FGameplayTag::EmptyTag;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EYapTimeMode> TimeMode = EYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(EditAnywhere)
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
