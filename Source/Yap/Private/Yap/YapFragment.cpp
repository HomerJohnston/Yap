// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapFragment.h"

#include "Yap/YapCharacterAsset.h"
#include "Yap/YapCondition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Engine/Blueprint.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapFragment::FYapFragment()
{
	Guid = FGuid::NewGuid();
	TimeMode = EYapTimeMode::Default;
}

bool FYapFragment::CanRun() const
{
	return CheckActivationLimit() && CheckConditions();
}

bool FYapFragment::CheckConditions() const
{
	for (TObjectPtr<UYapCondition> Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}

		if (!Condition->EvaluateCondition_Internal())
		{
			return false;
		}
	}
	
	return true;
}

void FYapFragment::ResetOptionalPins()
{
	bShowOnStartPin = false;
	bShowOnEndPin = false;
}

void FYapFragment::PreloadContent(UWorld* World, EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext)
{
	ResolveMaturitySetting(World, MaturitySetting);

	// TODO: somehow, a flow graph should be able to work with "self" as the speaker, to make it possible to code flows for dynamic NPCs/dynamic speakers
#if WITH_EDITOR
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		SpeakerHandle = UYapSubsystem::GetCharacterManager(World).RequestLoadAsync(Speaker.GetTagName());
		DirectedAtHandle = UYapSubsystem::GetCharacterManager(World).RequestLoadAsync(DirectedAt.GetTagName());
	}
	else if (GEditor)
	{
		UYapProjectSettings::FindCharacter(Speaker, SpeakerHandle, EYapLoadContext::AsyncEditorOnly);
		UYapProjectSettings::FindCharacter(DirectedAt, DirectedAtHandle, EYapLoadContext::AsyncEditorOnly);
	}
#else
	SpeakerHandle = UYapSubsystem::GetCharacterManager(World).RequestLoadAsync(Speaker.GetTagName());
	DirectedAtHandle = UYapSubsystem::GetCharacterManager(World).RequestLoadAsync(DirectedAt.GetTagName());
#endif
	
	if (MaturitySetting == EYapMaturitySetting::ChildSafe && bEnableChildSafe)
	{
		ChildSafeBit.LoadContent(LoadContext);
	}
	else
	{
		MatureBit.LoadContent(LoadContext);
	}
}

const FGameplayTag& FYapFragment::GetSpeakerTag() const
{
	return Speaker;
}

const FGameplayTag& FYapFragment::GetDirectedAtTag() const
{
	return DirectedAt;
}

const TScriptInterface<IYapCharacterInterface> FYapFragment::GetSpeakerCharacter(UWorld* World, EYapLoadContext LoadContext)
{
	return GetCharacter_Internal(World, Speaker, SpeakerHandle, LoadContext);
}

bool FYapFragment::HasDeprecatedSpeakerAsset()
{
	return !SpeakerAsset.IsNull();
}

bool FYapFragment::HasSpeakerAssigned()
{
	return Speaker.IsValid();
}

/*
const TSoftObjectPtr<const UObject> FYapFragment::GetCharacterAsset(UWorld* World, const FName& CharacterID, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext) const
{
	if (CharacterID == NAME_None)
	{
		return nullptr;
	}

	UYapSubsystem::GetCharacterManager(World).FindCharacter(CharacterID);
	return UYapProjectSettings::FindCharacter(CharacterID, Handle, LoadContext);
}
*/

const TScriptInterface<IYapCharacterInterface> FYapFragment::GetDirectedAt(UWorld* World, EYapLoadContext LoadContext)
{
	return GetCharacter_Internal(World, DirectedAt, DirectedAtHandle, LoadContext);
}

const TScriptInterface<IYapCharacterInterface> FYapFragment::GetCharacter_Internal(UWorld* World, const FGameplayTag& CharacterTag, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext)
{
	if (!CharacterTag.IsValid())
	{
		return nullptr;
	}

	return UYapSubsystem::GetCharacterManager(World).FindCharacter(CharacterTag.GetTagName()).GetObject();
}

const FText& FYapFragment::GetDialogueText(UWorld* World, EYapMaturitySetting MaturitySetting) const
{
	const FYapBit& Preferredbit = GetBit(World, MaturitySetting);
	const FYapBit& SecondaryBit = MatureBit; // Always fall back to the mature bit. Never fall back to the child-safe bit.

	if (Preferredbit.HasDialogueText())
	{
		return Preferredbit.GetDialogueText();
	}

	return SecondaryBit.GetDialogueText();
}

const FText& FYapFragment::GetTitleText(UWorld* World, EYapMaturitySetting MaturitySetting) const
{
	const FYapBit& Preferredbit = GetBit(World, MaturitySetting);
	const FYapBit& SecondaryBit = MatureBit; // Always fall back to the mature bit. Never fall back to the child-safe bit.

	if (Preferredbit.HasTitleText())
	{
		return Preferredbit.GetTitleText();
	}

	return SecondaryBit.GetTitleText();
}

const UObject* FYapFragment::GetAudioAsset(UWorld* World, EYapMaturitySetting MaturitySetting) const
{
	const FYapBit& Preferredbit = GetBit(World, MaturitySetting);
	const FYapBit& SecondaryBit = MatureBit; // Always fall back to the mature bit. Never fall back to the child-safe bit.

	if (Preferredbit.HasAudioAsset())
	{
		return Preferredbit.GetAudioAsset<UObject>();
	}

	return SecondaryBit.GetAudioAsset<UObject>();
}

const FYapBit& FYapFragment::GetBit(UWorld* World) const
{
	return GetBit(World, UYapSubsystem::GetCurrentMaturitySetting(World));
}

const FYapBit& FYapFragment::GetBit(UWorld* World, EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(World, MaturitySetting);

	if (MaturitySetting == EYapMaturitySetting::ChildSafe)
	{
		return ChildSafeBit;
	}
	
	return MatureBit;
}

TOptional<float> FYapFragment::GetSpeechTime(UWorld* World, const UYapNodeConfig& NodeConfig) const
{
	return GetSpeechTime(World, UYapSubsystem::GetCurrentMaturitySetting(World), EYapLoadContext::Sync, NodeConfig);
}

bool FYapFragment::IsAwaitingManualAdvance() const
{
	return bFragmentAwaitingManualAdvance;
}

void FYapFragment::SetAwaitingManualAdvance()
{
	bFragmentAwaitingManualAdvance = true;
}

void FYapFragment::ClearAwaitingManualAdvance()
{
	bFragmentAwaitingManualAdvance = false;
}

TOptional<float> FYapFragment::GetSpeechTime(UWorld* World, EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext, const UYapNodeConfig& NodeConfig) const
{
	EYapTimeMode EffectiveTimeMode = GetTimeMode(World, MaturitySetting, NodeConfig);

	if (EffectiveTimeMode == EYapTimeMode::None)
	{
		return NullOpt;
	}
	
	return GetBit(World, MaturitySetting).GetSpeechTime(World, EffectiveTimeMode, LoadContext, NodeConfig);
}

float FYapFragment::GetPaddingValue(UWorld* World, const UYapNodeConfig& NodeConfig) const
{	
	if (IsTimeModeNone())
	{
		return 0;
	}
	
	if (Padding.IsSet())
	{
		float RawPadding = Padding.GetValue();
		
		TOptional<float> SpeechTime = GetSpeechTime(World, NodeConfig);

		return FMath::Max(-SpeechTime.Get(0.0f), RawPadding);
	}
	
	return NodeConfig.GetDefaultFragmentPaddingTime();
}

bool FYapFragment::GetUsesPadding(UWorld* World, const UYapNodeConfig& NodeConfig) const
{
	float PaddingValue = GetPaddingValue(World, NodeConfig);
	
	return !FMath::IsNearlyZero(PaddingValue);
}

float FYapFragment::GetProgressionTime(UWorld* World, const UYapNodeConfig& NodeConfig) const
{
	float SpeechTime = GetSpeechTime(World, NodeConfig).Get(0.0f);
	
	float PaddingTime;
	
	if (Padding.IsSet())
	{
		PaddingTime = GetPaddingValue(World, NodeConfig);
	}
	else
	{
		PaddingTime = NodeConfig.GetDefaultFragmentPaddingTime();
	}

	return FMath::Max(SpeechTime + PaddingTime, 0.0f);
}

void FYapFragment::IncrementActivations()
{
	ActivationCount++;
}

/*
void FYapFragment::ReplaceBit(EYapMaturitySetting MaturitySetting, const FYapBitReplacement& ReplacementBit)
{
	// TODO: I'd rather use some sort of layers system rather than bulldoze the original data.
	//Bit = ReplacementBit;
}
*/

FFlowPin FYapFragment::GetPromptPin() const
{
	if (!PromptPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->PromptPin = FName("Prompt_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Out";
	}
	
	return PromptPin;
}

FFlowPin FYapFragment::GetEndPin() const
{
	if (!EndPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->EndPin = FName("End_" + Guid.ToString());
		MutableThis->PromptPin.PinToolTip = "Runs before end-padding time begins";
	}
	
	return EndPin;
}

FFlowPin FYapFragment::GetStartPin() const
{
	if (!StartPin.IsValid())
	{
		FYapFragment* MutableThis = const_cast<FYapFragment*>(this);
		MutableThis->StartPin = FName("Start_" + Guid.ToString());
		MutableThis->StartPin.PinToolTip = "Runs when fragment starts playback";
	}
	
	return StartPin;
}

void FYapFragment::ResolveMaturitySetting(UWorld* World, EYapMaturitySetting& MaturitySetting) const
{
	if (!bEnableChildSafe)
	{
		MaturitySetting = EYapMaturitySetting::Mature;
		return;
	}
	
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		if (IsValid(UYapSubsystem::Get(World)))
		{
			MaturitySetting = UYapSubsystem::GetCurrentMaturitySetting(World);
		}
		else
		{
			MaturitySetting = EYapMaturitySetting::Mature;
		}
	}
}

bool FYapFragment::GetSkippable(bool Default) const
{
	return Skippable.Get(Default);
}

EYapTimeMode FYapFragment::GetTimeMode(UWorld* World, const UYapNodeConfig& NodeConfig) const
{
	return GetTimeMode(World, UYapSubsystem::GetCurrentMaturitySetting(World), NodeConfig);
}

EYapTimeMode FYapFragment::GetTimeMode(UWorld* World, EYapMaturitySetting MaturitySetting, const UYapNodeConfig& NodeConfig) const
{
	EYapTimeMode EffectiveTimeMode = (TimeMode == EYapTimeMode::Default) ? NodeConfig.GetDefaultTimeModeSetting() : TimeMode;

	if (EffectiveTimeMode == EYapTimeMode::AudioTime)
	{
		if (!GetBit(World, MaturitySetting).HasAudioAsset())
		{
			EYapMissingAudioErrorLevel MissingAudioBehavior = NodeConfig.GetMissingAudioErrorLevel();
			
			if (MissingAudioBehavior == EYapMissingAudioErrorLevel::Error)
			{
				// Help force developers to notice and assign missing audio assets, by hindering or preventing dialogue progression
				EffectiveTimeMode = EYapTimeMode::None;
			}
			else if (MissingAudioBehavior == EYapMissingAudioErrorLevel::Warning)
			{
				EffectiveTimeMode = EYapTimeMode::TextTime;
			}
			else
			{
				EffectiveTimeMode = EYapTimeMode::TextTime;
			}
		}
	}

	return EffectiveTimeMode;
}

bool FYapFragment::IsTimeModeNone() const
{
	return TimeMode == EYapTimeMode::None;
}

bool FYapFragment::HasAudio() const
{
	return MatureBit.HasAudioAsset() || ChildSafeBit.HasAudioAsset();
}

bool FYapFragment::HasData() const
{
	return Data.Num() > 0;
}

#if WITH_EDITOR

FYapBit& FYapFragment::GetBitMutable(EYapMaturitySetting MaturitySetting)
{
	if (MaturitySetting == EYapMaturitySetting::ChildSafe && bEnableChildSafe)
	{
		return ChildSafeBit;
	}
	else
	{
		return MatureBit;
	}
}
#endif

#if WITH_EDITOR
void FYapFragment::InvalidateFragmentTag(UFlowNode_YapDialogue* OwnerNode)
{
	// TODO can this function be removed?
	FragmentID = NAME_None;
}
#endif


#if WITH_EDITOR
void FYapFragment::SetSpeaker(const FGameplayTag& CharacterTag)
{
	Speaker = CharacterTag;
	SpeakerHandle = nullptr;
}

void FYapFragment::SetDirectedAt(const FGameplayTag& CharacterTag)
{
	DirectedAt = CharacterTag;
	DirectedAtHandle = nullptr;
}
#endif

#undef LOCTEXT_NAMESPACE