// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBroker.h"

#include "Components/AudioComponent.h"
#include "Internationalization/BreakIterator.h"
#include "Yap/YapRunningFragment.h" 
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Handles/YapPromptHandle.h"
#include "Yap/Enums/YapMaturitySetting.h"
#include "Sound/SoundBase.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================

TOptional<bool> UYapBroker::bImplemented_Initialize = false;
TOptional<bool> UYapBroker::bImplemented_GetMaturitySetting = false;
TOptional<bool> UYapBroker::bImplemented_GetPlaybackSpeed = false;
TOptional<bool> UYapBroker::bImplemented_GetAudioAssetDuration = false;
#if WITH_EDITOR
TOptional<bool> UYapBroker::bImplemented_PreviewAudioAsset = false;
TOptional<bool> UYapBroker::bImplemented_GetNewNodeID = false;
TOptional<bool> UYapBroker::bImplemented_GetFragmentIDs = false;
#endif

bool UYapBroker::bWarned_Initialize = false;
bool UYapBroker::bWarned_GetMaturitySetting = false;
bool UYapBroker::bWarned_GetPlaybackSpeed = false;
bool UYapBroker::bWarned_GetAudioAssetDuration = false;
#if WITH_EDITOR
bool UYapBroker::bWarned_PreviewAudioAsset = false;
#endif

#define YAP_QUOTE(X) #X

#define YAP_CALL_K2(FUNCTION, SHOW_UNIMPLEMENTED_WARNING, ...) CallK2Function<&UYapBroker::K2_##FUNCTION>(YAP_QUOTE(FUNCTION), bImplemented_##FUNCTION, bWarned_##FUNCTION, SHOW_UNIMPLEMENTED_WARNING __VA_OPT__(,) __VA_ARGS__)

// ------------------------------------------------------------------------------------------------

void UYapBroker::Initialize()
{
	YAP_CALL_K2(Initialize, false);
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapBroker::GetMaturitySetting() const
{
	bool bSuppressDefaultMatureWarning = !UYapProjectSettings::GetSuppressBrokerWarnings();

	EYapMaturitySetting MaturitySetting = YAP_CALL_K2(GetMaturitySetting, bSuppressDefaultMatureWarning);

	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		return EYapMaturitySetting::Mature;
	}

	return MaturitySetting;
}

// ------------------------------------------------------------------------------------------------

float UYapBroker::GetPlaybackSpeed() const
{
	return YAP_CALL_K2(GetPlaybackSpeed, false);
}

// ------------------------------------------------------------------------------------------------

int32 UYapBroker::CalculateWordCount(const FText& Text) const
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateWordCount))) // TODO cache this? Switch To YAP_CALL_K2?
	{
		return K2_CalculateWordCount(Text);
	}

	return CalculateWordCount_DefaultImpl(Text);
}

// ------------------------------------------------------------------------------------------------

float UYapBroker::CalculateTextTime(int32 WordCount, int32 CharCount, const UYapNodeConfig& NodeConfig) const
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateTextTime))) // TODO cache this? Switch To YAP_CALL_K2?
	{
		return K2_CalculateTextTime(WordCount, CharCount, &NodeConfig);
	}
	
	int32 TWPM = NodeConfig.DialoguePlayback.TimeSettings.TextWordsPerMinute;
	float SecondsPerWord = 60.0 / (float)TWPM;
	float TalkTime = WordCount * SecondsPerWord * GetPlaybackSpeed();

	float Min = NodeConfig.GetMinimumAutoTextTimeLength();
		
	return FMath::Max(TalkTime, Min);
}

// ------------------------------------------------------------------------------------------------

float UYapBroker::GetAudioAssetDuration(const UObject* AudioAsset) const
{
	float Time = -1;

	if (!AudioAsset)
	{
		return Time;
	}
	
#if WITH_EDITOR
	const TArray<TSoftClassPtr<UObject>>& AudioAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

	bool bFoundClassMatch = false;
	
	for (const TSoftClassPtr<UObject>& Class : AudioAssetClasses)
	{
		if (Class.IsPending())
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading audio class asset - this should not happen!"));
		}
		
		if (AudioAsset->IsA(Class.LoadSynchronous()))
		{
			bFoundClassMatch = true;
			break;
		}
	}

	if (!bFoundClassMatch)
	{
		FString ProjectAudioClassesString;
		
		for (int32 i = 0; i < AudioAssetClasses.Num(); ++i)
		{
			const TSoftClassPtr<UObject>& Class = AudioAssetClasses[i];
			
			ProjectAudioClassesString += Class->GetName();

			if (i < AudioAssetClasses.Num() - 1)
			{
				ProjectAudioClassesString += ", ";
			}
		}
		
		UE_LOG(LogYap, Error, TEXT("Failed to match [%s] to a valid audio asset class! Asset type: [%s], project asset types: [%s]"), *AudioAsset->GetPathName(), *AudioAsset->GetClass()->GetName(), *ProjectAudioClassesString);
	}
#endif
	
	if (UYapProjectSettings::HasCustomAudioAssetClasses())
	{
		bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
		Time = YAP_CALL_K2(GetAudioAssetDuration, bShowUnimplementedWarning, AudioAsset);
	}
	else
	{
		// ------------------------------------------
		// Default Implementation
		const USoundBase* AudioAssetAsSoundBase = Cast<USoundBase>(AudioAsset);

		if (AudioAssetAsSoundBase)
		{
			Time = AudioAssetAsSoundBase->GetDuration();
		}
	}
	
	if (Time < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Failed to determine audio asset duration, unknown error!"));
	}

	return Time;
}

#if WITH_EDITOR

// ------------------------------------------------------------------------------------------------

// Used to check to see if a derived class actually implemented PlayDialogueAudioAsset_Editor()
thread_local bool bPreviewAudioAssetOverridden = false;
thread_local bool bSuppressPreviewAudioAssetWarning = false;

bool UYapBroker::PreviewAudioAsset(const UObject* AudioAsset) const
{
	/*
	if (ImplementsPreviewAudioAsset_Internal())
	{
		bool bResult = PreviewAudioAsset_Internal(AudioAsset);

		if (!bResult)
		{
			Yap::Editor::PostNotificationInfo_Warning
			(
				LOCTEXT("AudioPreview_UnknownWarning_Title", "Cannot Play Audio Preview"),
				LOCTEXT("AudioPreview_UnknownWarning_Description", "Unknown error!")
			);
		}
	}
	else
	{
		Yap::Editor::PostNotificationInfo_Warning
		(
			LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"),
			LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Description", "Your Broker Class must implement the \"PlayDialogueAudioAssetInEditor\" function.")
		);
	}
	*/

	if (UYapProjectSettings::HasCustomAudioAssetClasses())
	{
		bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
		return YAP_CALL_K2(PreviewAudioAsset, bShowUnimplementedWarning, AudioAsset);
	}
	else
	{
		// ------------------------------------------
		// Default Implementation
		static TWeakObjectPtr<UAudioComponent> PreviewAudioComponent; 
		
		if (const USoundBase* AudioAssetAsSoundBase = Cast<USoundBase>(AudioAsset))
		{
			if (PreviewAudioComponent.IsValid())
			{
				PreviewAudioComponent->Stop();
			}
			
			PreviewAudioComponent = GEditor->PlayPreviewSound(const_cast<USoundBase*>(AudioAssetAsSoundBase));

			// TODO 
			//PreviewAudioComponent->OnAudioFinished

			return true;
		}
		else
		{
			if (!bSuppressPreviewAudioAssetWarning)
			{
				UE_LOG(LogYap, Warning, TEXT("Sound was null"));
			}
			return false;
		}
	}
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FString UYapBroker::GetNewNodeID(const FYapAudioIDFormat& IDFormat, const TSet<FString>& ExistingNodeIDs) const
{
	// This function might be called many hundreds of times per frame and I'd like for it to go quick so let's just build a quick little thing to check on the first run whether to use BP or C++
	static uint64 LastImplementedInBPCheck = 0;
	static bool bImplementedInBP = false;

	if (GFrameCounter != LastImplementedInBPCheck)
	{
		LastImplementedInBPCheck = GFrameCounter;
		bImplementedInBP = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED_TwoParams(UYapBroker, K2_GetNewNodeID, const FYapAudioIDFormat&, const TSet<FString>));
	}

	if (bImplementedInBP)
	{
		return K2_GetNewNodeID(IDFormat, ExistingNodeIDs);
	}

	return GetNewNodeID_DefaultImpl(IDFormat, ExistingNodeIDs);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapBroker::GetFragmentIDs(const FYapAudioIDFormat& IDFormat, TArray<int32>& ExistingFragmentIDs) const
{
	/* Algorithm description:
	 *
	 * When this function starts, ExistingFragmentIDs will have N entries (one for each fragment).
	 * Any fragments with audio assigned will KEEP their existing ID#
	 * Any fragments without audio assigned will be empty strings
	 *
	 * We start by turning this into an array of integers using -1 for empty values, so we might have
	 * [010,  -1,  -1, 050,  -1, 060,  -1,  -1, 063,  -1]
	 * ...where the first, fourth and fifth fragments had audio assigned.
	 * 
	 * For each entry that is unassigend, we count ahead to find an existing entry. If one is found, we
	 * check if we can increment normally or not.
	 *
	 * If there isn't enough room, we start "squashing" the increment value until the values fit, first by half,
	 * then by -1. If it won't fit even with an increment of 1 then we'll just abort and throw an error message.
	 *
	 * The above example should turn into:
	 * [010, 020, 030, 050, 055, 060, 061, 062, 063, 070]
	*/

	TArray<int32> Copy = ExistingFragmentIDs;
	
	if (!DevelopMissingIDs(Copy, IDFormat))
	{
		return false;
	}

	ExistingFragmentIDs = Copy;
	return true;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapBroker::GetFragmentIDs_Internal(const FYapAudioIDFormat& IDFormat, TArray<int32>& ExistingFragmentIDs) const
{
	static uint64 LastImplementedInBPCheck = 0;
	static bool bImplementedInBP = false;

	if (GFrameCounter != LastImplementedInBPCheck)
	{
		LastImplementedInBPCheck = GFrameCounter;
		bImplementedInBP = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED_TwoParams(UYapBroker, K2_GetFragmentIDs, const FYapAudioIDFormat&, TArray<int32>&));
	}

	bool bResult;
	
	if (bImplementedInBP)
	{
		bResult = K2_GetFragmentIDs(IDFormat, ExistingFragmentIDs);
	}
	else
	{
		bResult = GetFragmentIDs(IDFormat, ExistingFragmentIDs);
	}

	return bResult;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FString UYapBroker::GenerateDialogueAudioID(const UFlowNode_YapDialogue* InNode) const
{
	UFlowAsset* FlowAsset = InNode->GetFlowAsset();

	// Find all existing tags
	TSet<FString> ExistingAudioIDs;
	
	for (auto&[GUID, Node] : FlowAsset->GetNodes())
	{
		if (UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Node))
		{
			if (!DialogueNode->GetAudioIDRoot().IsEmpty())
			{
				ExistingAudioIDs.Add(DialogueNode->GetAudioIDRoot());
			}
		}
	}
	
	// Generate a new tag, making sure it isn't already in use in the graph by another dialogue node
	FString NewID;
	
	int32 Safety = 0;
	do 
	{
		if (Safety > 1000)
		{
			UE_LOG(LogYap, Error, TEXT("Failed to generate a unique dialogue tag after 1000 iterations!"));
			return "";
		}
		
		NewID = GenerateRandomDialogueAudioID();
		
	} while (ExistingAudioIDs.Contains(NewID));

	return NewID;
}

void UYapBroker::UpdateNodeFragmentIDs(UFlowNode_YapDialogue* InNode) const
{
	TArray<int32> ExistingFragmentIDs;
	ExistingFragmentIDs.Reserve(InNode->GetNumFragments());

	for (const FYapFragment& Fragment : InNode->GetFragments())
	{
		if (Fragment.HasAnyAudio())
		{
			ExistingFragmentIDs.Add(Fragment.GetAudioID());
		}
		else
		{
			ExistingFragmentIDs.Add(INDEX_NONE);
		}
	}

	if (GetFragmentIDs_Internal(InNode->GetNodeConfig().GetAudioIDFormat(), ExistingFragmentIDs))
	{
		for (int32 i = 0; i < InNode->GetFragments().Num(); ++i)
		{
			// TODO make sure all paths accessing this are wrapped in a transaction from editor code
			InNode->GetFragmentsMutable()[i].SetAudioID(ExistingFragmentIDs[i]);
		}
	}
	else
	{
		Yap::Editor::PostNotificationInfo_Warning(
			LOCTEXT("GenerateFragmentID_ErrorTitle", "Fragment ID Generation Error"),
			LOCTEXT("GenerateFragmentID_ErrorDescription", "Unable to generate fragment IDs, likely two existing IDs with audio assets are too close together and you'll need to fix up manually."));	
	}
}

// ------------------------------------------------------------------------------------------------

FString UYapBroker::GenerateRandomDialogueAudioID() const
{
	// TArray<char> AlphaNumerics {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H',/*'I',*/'J','K','L','M','N',/*'O',*/'P','Q','R','S','T','U','V','W','X','Y','Z'};
	TArray<char> AlphaNumerics {'A','B','C','D','E','F','G','H',/*'I',*/'J','K','L','M','N',/*'O',*/'P','Q','R','S','T','U','V','W','X','Y','Z'};

	const uint8 Size = 3;

	FString String;
	String.Reserve(Size);
	
	for (uint8 i = 0; i < Size; ++i)
	{
		uint8 RandIndex = FMath::RandHelper(AlphaNumerics.Num());
		String += AlphaNumerics[RandIndex];
	}

	return String;
}
#endif

// ------------------------------------------------------------------------------------------------

void UYapBroker::BeginPlay()
{
	bWarned_Initialize = false;
	bWarned_GetMaturitySetting = false;
	bWarned_GetAudioAssetDuration = false;
#if WITH_EDITOR
	bWarned_PreviewAudioAsset = false;
#endif

	UClass* Class = GetClass();
	
	bImplemented_Initialize = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_Initialize));
	bImplemented_GetMaturitySetting = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_GetMaturitySetting));
	bImplemented_GetAudioAssetDuration = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_GetAudioAssetDuration));
#if WITH_EDITOR
	bImplemented_PreviewAudioAsset = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_PreviewAudioAsset));
	bImplemented_GetNewNodeID = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED_TwoParams(UYapBroker, K2_GetNewNodeID, const FYapAudioIDFormat&, const TSet<FString>));
	bImplemented_GetFragmentIDs = Class->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED_TwoParams(UYapBroker, K2_GetFragmentIDs, const FYapAudioIDFormat&, TArray<int32>&));
#endif

	Initialize();
}

// ------------------------------------------------------------------------------------------------

UYapBroker& UYapBroker::Get(const UObject* WorldContext)
{
	check(WorldContext);
	check(WorldContext->GetWorld());
	
	UYapSubsystem* Subsystem = UYapSubsystem::Get(WorldContext);
	check(Subsystem);
	
	return Subsystem->GetBroker();
}

#if WITH_EDITOR
UYapBroker& UYapBroker::GetInEditor()
{
	return *UYapProjectSettings::GetBrokerClass().GetDefaultObject();	
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapBroker::PreviewAudioAsset_Internal(const UObject* AudioAsset) const
{
	bPreviewAudioAssetOverridden = true;

	return PreviewAudioAsset(AudioAsset);
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapBroker::ImplementsPreviewAudioAsset_Internal() const
{
	bSuppressPreviewAudioAssetWarning = true;
	(void)PreviewAudioAsset_Internal(nullptr);
	bSuppressPreviewAudioAssetWarning = false;

	return bPreviewAudioAssetOverridden;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
int32 UYapBroker::CalculateWordCount_DefaultImpl(const FText& Text) const
{
	// Utility to count the number of words within a string (we use a line-break iterator to avoid counting the whitespace between the words)
	TSharedRef<IBreakIterator> LineBreakIterator = FBreakIterator::CreateLineBreakIterator();
	auto CountWords = [&LineBreakIterator](const FString& InTextToCount) -> int32
	{
		int32 NumWords = 0;
		LineBreakIterator->SetString(InTextToCount);

		int32 PreviousBreak = 0;
		int32 CurrentBreak;

		while ((CurrentBreak = LineBreakIterator->MoveToNext()) != INDEX_NONE)
		{
			if (CurrentBreak > PreviousBreak)
			{
				++NumWords;
			}
			PreviousBreak = CurrentBreak;
		}

		LineBreakIterator->ClearString();
		return NumWords;
	};

	return CountWords(Text.ToString());
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
FString UYapBroker::GetNewNodeID_DefaultImpl(const FYapAudioIDFormat& IDFormat, const TSet<FString>& ExistingNodeIDs) const
{
	FString Pattern = IDFormat.NodeIDFormat;

	TArray<TCHAR> IllegalChars = UYapProjectSettings::GetIllegalAudioIDCharacters().GetCharArray();

	// Note (not a to do) I could do this outside of this function before the loop. It might be a bit faster.
	TArray<char> AlphaNumerics {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	TArray<TCHAR> Alphas {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

	for (int i = 0; i < IllegalChars.Num(); ++i)
	{
		AlphaNumerics.Remove(IllegalChars[i]);
		Alphas.Remove(IllegalChars[i]);
	}

	TSet<TCHAR> ValidChars{'!', '#', '$', '%', '&', '(', ')', '+', ',', '-', '.', ';', '=', '@', '[', ']', '^', '_', '`', '{', '}', '~',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	' '};

	// The default implementation will simply replace special chars with random chars, this isn't rocket science
	for (int i = 0; i < Pattern.Len(); ++i)
	{
		const TCHAR& Char = Pattern[i];

		switch (Char)
		{
			case '*':
			{
				Pattern[i] = Alphas[FMath::RandHelper(Alphas.Num())];
				break;
			}
			case '?':
			{
				Pattern[i] = AlphaNumerics[FMath::RandHelper(AlphaNumerics.Num())];
			}
			default:
			{
				// Replace invalid file characters with an underscore
				if (!ValidChars.Contains(Char))
				{
					Pattern[i] = '_';
				}
			}
		}
	}

	return Pattern;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
bool UYapBroker::DevelopMissingIDs(TArray<int32>& ExistingFragmentIDInts, const FYapAudioIDFormat& IDFormat) const
{
	int32 LastValidID = INDEX_NONE;

	TArray<int32>& Copy = ExistingFragmentIDInts;
	
	for (int32 i = 0; i < Copy.Num(); ++i)
	{
		if (Copy[i] != INDEX_NONE)
		{
			LastValidID = Copy[i];
		}
		else
		{
			for (int32 j = i + 1; j <= Copy.Num(); ++j)
			{
				// Went past the end of the array
				if (j == Copy.Num())
				{
					if (FillInIDSpan(Copy, i, j - 1, LastValidID, INDEX_NONE, IDFormat))
					{
						i = j - 1;
						break;
					}
					
					return false;
				}

				// Found a valid ID later in the array
				else if (Copy[j] != INDEX_NONE)
				{
					if (FillInIDSpan(Copy, i, j - 1, LastValidID, Copy[j], IDFormat))
					{
						i = j - 1;
						break;
					}

					return false;
				}
			}
		}
	}

	
	ExistingFragmentIDInts = Copy;
	return true;
}

bool UYapBroker::FillInIDSpan(TArray<int32>& ExistingFragmentIDInts, int32 Start, int32 End, int32 LastValidID, int32 NextValidID, const FYapAudioIDFormat& IDFormat) const
{
	int32 Increment = IDFormat.DefaultIncrement;

	// If we have a future ID then we'll need to check if we need to reduce the increment
	if (NextValidID != INDEX_NONE)
	{
		int32 Gap = NextValidID - (LastValidID == INDEX_NONE ? 0 : LastValidID);
		int32 Count = End - Start + 1;

		bool bFirstReduction = true;
		
		while (Count * Increment >= Gap)
		{
			if (bFirstReduction)
			{
				Increment = Gap / (Count + 1);
				bFirstReduction = false;
			}
			else
			{
				Increment -= 1;
			}

			if (Increment <= 0)
			{
				return false;
			}
		}
	}
	
	int32 CurrentIDValue;

	if (LastValidID == INDEX_NONE)
	{
		// First entry in the array, we start with a value like 010 instead of 000 so that you can insert earlier values later
		CurrentIDValue = Increment;
	}
	else
	{
		CurrentIDValue = LastValidID + Increment;
	}
	
	for (int i = Start; i <= End; ++i)
	{
		ExistingFragmentIDInts[i] = CurrentIDValue;
		CurrentIDValue += Increment;
	}

	return true;
}
#endif

#undef LOCTEXT_NAMESPACE