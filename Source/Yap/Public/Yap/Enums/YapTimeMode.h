// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapTimeMode.generated.h"

UENUM()
enum class EYapTimeMode : uint8
{
	/** Use project default setting */
	Default UMETA(Hidden),
	/** Dialogue will not have any time settings. Conversation dialogue must be manually advanced, and free speech must be manually cancelled, by your own code. */
	None,
	/** Dialogue will use audio asset time. If an audio asset is not available, Yap will silently fallback to text time. Use this if you have a mixture of speech with and without audio. */
	AudioTime_TextFallback,
	/** Dialogue will calculate time based on the text. Use this if you do not intend to have any audio. */
	TextTime,
	/** Dialogue will automatically progress after a fixed time. This requires you to set a time for every fragment manually. */
	ManualTime,
};
