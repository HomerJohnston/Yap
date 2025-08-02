// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EYapInterruptibleFlags : uint8
{
	None			= 0 UMETA(Hidden),
	FreeSpeech		= 1 << 0,
	Conversation	= 1 << 1,
};

ENUM_CLASS_FLAGS(EYapInterruptibleFlags);