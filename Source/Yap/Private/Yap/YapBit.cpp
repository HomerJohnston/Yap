// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapBit.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"

#define LOCTEXT_NAMESPACE "Yap"

// --------------------------------------------------------------------------------------------

const FText& FYapBit::GetDialogueText() const
{
	return DialogueText.Get();
}

// --------------------------------------------------------------------------------------------

const FText& FYapBit::GetTitleText() const
{
	return TitleText.Get();
}

// --------------------------------------------------------------------------------------------

bool FYapBit::HasAudioAsset() const
{
	return !AudioAsset.IsNull();
}

// --------------------------------------------------------------------------------------------

TOptional<float> FYapBit::GetSpeechTime(EYapTimeMode TimeMode, EYapLoadContext LoadContext) const
{
	// TODO clamp minimums from project settings?
	TOptional<float> Time;
	
	switch (TimeMode)
	{
		case EYapTimeMode::ManualTime:
		{
			Time = ManualTime;
			break;
		}
		case EYapTimeMode::AudioTime:
		{
			Time = GetAudioTime(LoadContext);
			break;
		}
		case EYapTimeMode::TextTime:
		{
			Time = GetTextTime();
			break;
		}
		default:
		{
			Time = NullOpt;
		}
	}

	float Value = Time.Get(0);
	float MinAllowableTime = UYapProjectSettings::GetMinimumFragmentTime();

	return FMath::Max(Value, MinAllowableTime);
}

// --------------------------------------------------------------------------------------------

void FYapBit::LoadContent(EYapLoadContext LoadContext) const
{
	if (!AudioAsset.IsPending())
	{
		return;
	}
	
	switch (LoadContext)
	{
		case EYapLoadContext::Sync:
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading audio asset. This should not happen during gameplay! Try loading the flow asset sooner, or delaying dialogue.\nAsset: %s"), *AudioAsset->GetPathName());
			(const_cast<FYapBit*>(this))->AudioAssetHandle = FYapStreamableManager::Get().RequestSyncLoad(AudioAsset.ToSoftObjectPath());
			break;
		}
		case EYapLoadContext::Async:
		{
			(const_cast<FYapBit*>(this))->AudioAssetHandle = FYapStreamableManager::Get().RequestAsyncLoad(AudioAsset.ToSoftObjectPath());
			break;
		}
		case EYapLoadContext::AsyncEditorOnly:
		{
#if WITH_EDITOR
			if (!GEditor->IsPlaySessionInProgress())
			{
				FYapStreamableManager::Get().RequestAsyncLoad(AudioAsset.ToSoftObjectPath());
			}
#endif
			break;
		}
	}
}

// --------------------------------------------------------------------------------------------

TOptional<float> FYapBit::GetTextTime() const
{
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	int32 WordCount = DialogueText.GetWordCount();
	double SecondsPerWord = 60.0 / (double)TWPM;
	double MinTextTimeLength = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	
	return FMath::Max(WordCount * SecondsPerWord, MinTextTimeLength);
}

// --------------------------------------------------------------------------------------------

TOptional<float> FYapBit::GetAudioTime(EYapLoadContext LoadContext) const
{
	if (AudioAsset.IsNull())
	{
		return NullOpt;
	}

	LoadContent(LoadContext);

	UObject* Asset = AudioAsset.Get();

	if (!IsValid(Asset))
	{
		return NullOpt;
	}
	
	UYapBroker* Broker = nullptr;

#if WITH_EDITOR
	if (GEditor && GEditor->IsPlayingSessionInEditor())
	{
		Broker = UYapSubsystem::GetBroker();
	}
	else
	{
		// This is running at editor time only
		const TSoftClassPtr<UYapBroker>& BrokerClass = UYapProjectSettings::GetBrokerClass();
	
		if (BrokerClass.IsNull())
		{
			UE_LOG(LogYap, Warning, TEXT("No broker found in project settings! Cannot determine audio time!")); 
			return NullOpt;
		}
		
		if (!BrokerClass.IsNull())
		{
			Broker = BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();
		}
	}
#else
	Broker = UYapSubsystem::GetBroker();
#endif

	if (!Broker)
	{
		UE_LOG(LogYap, Warning, TEXT("No broker found! Cannot determine audio time!")); 
		return NullOpt;
	}
		
	return Broker->GetAudioAssetDuration(GetAudioAsset<UObject>());
}

// --------------------------------------------------------------------------------------------

/*

FYapBit& FYapBit::operator=(const FYapBitReplacement& Replacement)
{
#define FLOWYAP_REPLACE(X) if (Replacement.X.IsSet()) {X = Replacement.X.GetValue(); }  

	FLOWYAP_REPLACE(SpeakerAsset);
	FLOWYAP_REPLACE(DirectedAtAsset);
	FLOWYAP_REPLACE(MatureTitleText);

	if (Replacement.bOverrideMatureDialogueText)
	{
		MatureDialogueText = Replacement.MatureDialogueText;
	}
	
	//FLOWYAP_REPLACE(MatureDialogueText);
	FLOWYAP_REPLACE(SafeTitleText);
	FLOWYAP_REPLACE(SafeDialogueText);
	FLOWYAP_REPLACE(MatureAudioAsset);
	FLOWYAP_REPLACE(SafeAudioAsset);
	FLOWYAP_REPLACE(MoodTag);
	FLOWYAP_REPLACE(TimeMode);
	FLOWYAP_REPLACE(ManualTime);
	
#undef FLOWYAP_REPLACE
	return *this;
}
*/

// --------------------------------------------------------------------------------------------
// EDITOR API

#if WITH_EDITOR
void FYapBit::SetTitleText(const FText& NewText)
{
	TitleText = NewText;	
}
#endif

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
void FYapBit::SetDialogueText(const FText& NewText)
{
	DialogueText.Set(NewText);
}
#endif

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
void FYapBit::RecacheSpeakingTime()
{
	// TODO
	//RecalculateTextWordCount(MatureDialogueText, CachedMatureWordCount);
}
#endif

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
void FYapBit::RecalculateTextWordCount(FText& Text, float& CachedWordCount)
{
	int32 WordCount = -1;

	if (UYapProjectSettings::CacheFragmentWordCountAutomatically())
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		if (IsValid(Broker))
		{
			WordCount = Broker->CalculateWordCount(Text);
		}
	}

	if (WordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	CachedWordCount = WordCount;
}
#endif

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	AudioAsset = NewAudio;
}
#endif

// --------------------------------------------------------------------------------------------

#if WITH_EDITOR
void FYapBit::RecalculateAudioTime(TOptional<float>& CachedTime)
{
	const TSoftClassPtr<UYapBroker>& BrokerClass = UYapProjectSettings::GetBrokerClass();
	
	if (BrokerClass.IsNull())
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!")); 
		CachedTime.Reset();
		return;
	}
	
	UYapBroker* BrokerCDO = BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();

	float NewCachedTime = BrokerCDO->GetAudioAssetDuration(AudioAsset.LoadSynchronous());

	if (NewCachedTime > 0)
	{
		CachedTime = NewCachedTime;
	}
	else
	{
		CachedTime.Reset();
	}
}

void FYapBit::ClearAllData()
{
	DialogueText.Clear();
	TitleText = FText::GetEmpty();
	AudioAsset.Reset();
}
#endif

// --------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE