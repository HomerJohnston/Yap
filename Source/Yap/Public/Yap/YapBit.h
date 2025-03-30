// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapLog.h"
#include "YapText.h"
#include "Yap/Globals/YapEditorWarning.h"

#include "YapBit.generated.h"

struct FStreamableHandle;
class UFlowNode_YapDialogue;
struct FYapBitReplacement;
enum class EYapTimeMode : uint8;
enum class EYapLoadContext : uint8;
struct FGameplayTag;

#define LOCTEXT_NAMESPACE "Yap"

/** A 'Bit' is a data container of actual dialogue information to be spoken. Each Fragment contains two bits - one for normal mature dialogue, and one for child-safe dialogue. */
USTRUCT(BlueprintType)
struct YAP_API FYapBit
{
#if WITH_EDITOR
	friend class SFlowGraphNode_YapFragmentWidget;
#endif
	
	GENERATED_BODY()
	
	// --------------------------------------------------------------------------------------------
	// SETTINGS
	// --------------------------------------------------------------------------------------------
protected:
	
	/** Actual text to be spoken/displayed. */
	UPROPERTY()
	FYapText DialogueText;

	/** Optional title text, this is typically used by player prompts to show different text that the player will select. You can enable it to be included on all Talk nodes in project settings. */
	UPROPERTY()
	FYapText TitleText;
	
	/** Speech audio. Any asset. Filter the allowable asset type(s) using project settings. */
	UPROPERTY()
	TSoftObjectPtr<UObject> AudioAsset;

	/** Optional time override. You can use this if you want some dialogue to run for a length of time different than what was automatically calculated from the audio or text data. */
	UPROPERTY()
	float ManualTime = 0;

#if WITH_EDITORONLY_DATA
	/** Whether the dialogue data of this bit can be edited. Dialogue should be locked after exporting a .PO file for translators to make it harder to accidentally edit source text. */
	UPROPERTY()
	bool bLocked = false;
	
	/** Optional field to type in extra localization comments. For .PO export these will be prepended with a #. symbol.*/
	UPROPERTY()
	FString DialogueLocalizationComments;

	/** Optional field to type in extra localization comments. For .PO export these will be prepended with a #. symbol.*/
	UPROPERTY()
	FString TitleTextLocalizationComments;

	/** Optional field to type in directions for recording dialogue audio. */
	UPROPERTY()
	FString StageDirections;

	/** The most recent of these will be exported into .PO files as a #| msgctxt entry. */
	UPROPERTY()
	TArray<FString> PreviousMsgctxt;
	
	/** The most recent of these will be exported into .PO files as a #| msgid entry. */
	UPROPERTY()
	TArray<FString> PreviousMsgid;
#endif

	// --------------------------------------------------------------------------------------------
	// STATE
	// --------------------------------------------------------------------------------------------
protected:
	
	/** Handle to keep async-loaded audio alive. */
	TSharedPtr<FStreamableHandle> AudioAssetHandle;
	
	// --------------------------------------------------------------------------------------------
	// PUBLIC API
	// --------------------------------------------------------------------------------------------
public:
	
	/** Getter for dialogue text. */
	const FText& GetDialogueText() const;

	bool HasDialogueText() const { return !DialogueText.Get().IsEmpty(); }
	
	/** Getter for title text. */
	const FText& GetTitleText() const;

	bool HasTitleText() const { return !TitleText.Get().IsEmpty(); }

	/** Getter for audio asset, raw access to the soft pointer. */
	template<class T>
	const TSoftObjectPtr<T> GetDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(AudioAsset); }

	bool HasAudioAsset() const;

	/** Getter for audio asset. This function will force a sync-load if the audio asset isn't loaded yet! */
	template<class T>
	const T* GetAudioAsset() const;

	/** Loads the audio asset. */
	void LoadContent(EYapLoadContext LoadContext) const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	TOptional<float> GetSpeechTime(EYapTimeMode TimeMode, EYapLoadContext LoadContext, const FGameplayTag& TypeGroup) const;

	// --------------------------------------------------------------------------------------------
	// INTERNAL API
	// --------------------------------------------------------------------------------------------
protected:
	
	/** Getter for manual time setting value. */
	TOptional<float> GetManualTime() const { return ManualTime; }

	/** Calculates the current text time. */
	TOptional<float> GetTextTime(const FGameplayTag& TypeGroup) const;

	/** Gets the current time of the audio asset. */
	TOptional<float> GetAudioTime(EYapLoadContext LoadContext) const;

#if WITH_EDITOR
	// --------------------------------------------------------------------------------------------
	// EDITOR API
	// --------------------------------------------------------------------------------------------
public:

	void SetTitleText(const FText& NewText);

	void SetDialogueText(const FText& NewText);
	
	void SetDialogueAudioAsset(UObject* NewAudio);
	
	void SetManualTime(float NewValue) { ManualTime = NewValue; }

private:
	void RecacheSpeakingTime();

	void RecalculateTextWordCount(FText& Text, float& CachedTime);

	void RecalculateAudioTime(TOptional<float>& CachedTime);

	void ClearAllData();
#endif
};

// --------------------------------------------------------------------------------------------

template <class T>
const T* FYapBit::GetAudioAsset() const
{
	// Asset is unset. Return nothing.
	if (AudioAsset.IsNull())
	{
		return nullptr;
	}

#if WITH_EDITOR
	if (AudioAsset.IsPending())
	{
		if (GEditor->PlayWorld && (GEditor->PlayWorld->WorldType == EWorldType::Game || GEditor->PlayWorld->WorldType == EWorldType::PIE))
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset. Try loading the flow asset sooner, or putting in a delay before running dialogue."))

			Yap::Editor::PostNotificationInfo_Warning
			(
				LOCTEXT("SyncLoadAudioWarning_Title", "Sync Loading Audio"),
				LOCTEXT("SyncLoadAudioWarning_Description", "Synchronously loading dialogue audio asset.")
			);
		}
	}
#endif

	return AudioAsset.LoadSynchronous();
}

#undef LOCTEXT_NAMESPACE