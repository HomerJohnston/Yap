// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapText.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"

#if WITH_EDITOR
void FYapText::Set(const FText& InText)
{
	Text = InText;

	// TODO I have this setting... but if it's turned off, there's no way to set the word count. Add a way to configure word count.
	if (UYapProjectSettings::CacheFragmentWordCountAutomatically())
	{
		UpdateInternalWordCount();
	}
}
#endif

#if WITH_EDITOR
void FYapText::UpdateInternalWordCount()
{
	if (Text.IsEmptyOrWhitespace())
	{
		WordCount = 0;
		return;
	}
	
	const UYapBroker& Broker = UYapSubsystem::GetBroker_Editor();

	const int32 NewWordCount = Broker.CalculateWordCount(Text);

	if (NewWordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	WordCount = NewWordCount;
}
#endif

#if WITH_EDITOR
void FYapText::Clear()
{
	Text = FText::GetEmpty();
	WordCount = 0;
}
#endif
