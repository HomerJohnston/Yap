// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapAudioPriority.generated.h"

/**
 * Controls how Yap behaves when dialogue is set to use Audio Time, but audio is not set.
 */
UENUM()
enum class EYapAudioPriority : uint8
{
	Optional	UMETA(ToolTip = ""),
	Preferred	UMETA(ToolTip = "Write warnings in dev, highlight the node yellow"),
	Forced		UMETA(ToolTip = "Write errors in dev, prevent packaging"),
};