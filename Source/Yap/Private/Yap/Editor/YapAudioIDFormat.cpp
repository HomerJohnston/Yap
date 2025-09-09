// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Editor/YapAudioIDFormat.h"

FString FYapAudioIDFormat::ParseFragmentID(int32 FragmentID) const
{
	FString IntAsString = FString::FromInt(FragmentID);

	int32 Padding = FMath::Max(MinDigits - IntAsString.Len(), 0);

	if (Padding > 0)
	{
		IntAsString = FString::ChrN(Padding, '0') + IntAsString;
	}

	return IntAsString;
}
