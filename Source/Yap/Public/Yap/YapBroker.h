// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "FlowAsset.h"
#include "YapLog.h"
#include "Nodes/FlowNode_YapDialogue.h"

#include "YapBroker.generated.h"

enum class EYapMaturitySetting : uint8;
class UYapCharacterAsset;
struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================

/**
 * Required class for brokering Yap to your game.
 *   1) Create a child class of this in either C++ or Blueprint, and implement any needed functions.
 *   2) Go into Yap's Project Settings and set the broker setting to your class.
 *   3) Do NOT call Super or Parent function implementations when overriding any functions in this class.
 */
UCLASS(Blueprintable)
class YAP_API UYapBroker : public UObject
{
	GENERATED_BODY()
	
	// ============================================================================================
	// C++ OVERRIDABLE FUNCTIONS (DO NOT OVERRIDE THESE IF YOU ARE IMPLEMENTING YOUR BROKER WITH C++!)
	// ============================================================================================
	
public:

	// - - - - - GAME FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** Use this to do any desired initialization, such as creating a Dialogue UI instance if you aren't creating one already elsewhere. Do NOT call Super. */
	virtual void Initialize();
	
	/** Use this to read your game's user settings (e.g. "Enable Mature Content") and determine if mature language is permitted. Do NOT call Super. */
	virtual EYapMaturitySetting GetMaturitySetting() const;

	/** Use this to modify speaking time by a scalar multiplier. Does NOT affect anything when fragments are using audio duration. Do NOT call Super. */
	virtual float GetPlaybackSpeed() const;
	
	// - - - - - EDITOR FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** Provides a word count estimate of a given piece of FText. A default implementation of this function exists. Do NOT call Super. */
	virtual int32 CalculateWordCount(const FText& Text) const;

	/** Use this to fully override how your game calculates text time. The default implementation of this function will use your node config setting UYapNodeConfig.DialoguePlayback.TimeSettings.TextWordsPerMinute multiplied by GetPlaybackSpeed from your UYapBroker. Do NOT call Super. */
	virtual float CalculateTextTime(int32 WordCount, int32 CharCount, const UYapNodeConfig&  NodeConfig) const;
	
	/** Override this if you use 3rd party audio (WWise, FMOD). Cast to your audio type and return the duration length in seconds. Do NOT call Super. */
	virtual float GetAudioAssetDuration(const UObject* AudioAsset) const;

#if WITH_EDITOR
public:
	/** Override this if you use 3rd party audio (Wwise, FMOD, etc.). Cast to your audio type and initiate playback in editor. Do NOT call Super. */
	virtual bool PreviewAudioAsset(const UObject* AudioAsset) const;

	/** For a single node, generate a new ID */
	virtual FString GetNewNodeID(const FYapAudioIDFormat& IDFormat, const TSet<FString>& ExistingNodeIDs) const;

	/** For a whole node, generate fragment index IDs for all fragments. Return success/fail as true/false. */
	bool GetFragmentIDs(const FYapAudioIDFormat& IDFormat, TArray<int32>& ExistingFragmentIDs) const;
#endif
	
	// ============================================================================================
	// BLUEPRINT OVERRIDABLE FUNCTIONS (DO NOT USE THESE IF YOU ARE IMPLEMENTING YOUR BROKER WITH C++!)
	// ============================================================================================

protected:
	
	// - - - - - GAME FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Initialize", meta = (ToolTip = "Use this to do any desired initialization, such as creating a Dialogue UI instance if you aren't creating one already elsewhere.") )
	void K2_Initialize();
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Maturity Setting", meta = (ToolTip = "Use this to read your game's user settings (e.g. 'Enable Mature Content' or 'Child Safe' settings) and determine if mature language is permitted."))
	EYapMaturitySetting K2_GetMaturitySetting() const;
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Playback Speed", meta = (ToolTip = "Use this to modify speaking time by a scalar multiplier, return 1.0 for no change. Does NOT affect anything when fragments are using audio duration!"))
	float K2_GetPlaybackSpeed() const;
	
	// - - - - - EDITOR FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Calculate Word Count", meta = (ToolTip = "Use this to override how Yap calculates word counts of text."))
	int32 K2_CalculateWordCount(const FText& Text) const;
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Calculate Word Count", meta = (ToolTip = "Use this to read your game's settings (e.g. text playback speed) and determine the duration a dialogue should run for.\nThe default implementation of this function will use your project setting TextWordsPerMinute multiplied by GetPlaybackSpeed."))
	float K2_CalculateTextTime(int32 WordCount, int32 CharCount, const UYapNodeConfig* NodeConfig) const;
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Dialogue Audio Duration", meta = (ToolTip = "Override this if you use 3rd party audio (Wwise, FMOD, etc.). Cast to your audio type and return the duration length in seconds."))
	float K2_GetAudioAssetDuration(const UObject* AudioAsset) const;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Play Dialogue Audio In Editor", meta = (ToolTip = "Override this if you use 3rd party audio (Wwise, FMOD, etc.). Cast to your audio type and initiate playback in editor."))
	bool K2_PreviewAudioAsset(const UObject* AudioAsset) const;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get New Node ID", meta = (ToolTip = "Override this if you want to change the default interpretation of the Audio ID Format."))
	FString K2_GetNewNodeID(const FYapAudioIDFormat& IDFormat, const TSet<FString>& ExistingNodeIDs) const;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Fragment ID", meta = (ToolTip = "Override this if you want to change the default interpretation of the Audio ID Format. Return success status."))
	bool K2_GetFragmentIDs(const FYapAudioIDFormat& IDFormat, UPARAM(ref) TArray<int32>& ExistingFragmentIDs) const;
	
	// ============================================================================================
	// INTERNAL YAP FUNCTIONS
	// ============================================================================================
		
#if WITH_EDITOR
public:
	bool ImplementsGetWorld() const override { return true; }
#endif
	
	// ------------------------------------------------------------------------------------------------
	// STATE DATA
	// ------------------------------------------------------------------------------------------------
private:
	// These will be initialized during BeginPlay and keep track of whether there is a K2 function implementation to call, if the C++ implementation is not overridden. If the C++ implementation
	// is overridden, it will supersede. This is "backwards" compared to normal BNE's but if you're overriding this class in C++ you won't want to override further in BP.
	static TOptional<bool> bImplemented_Initialize;
	static TOptional<bool> bImplemented_GetMaturitySetting;
	static TOptional<bool> bImplemented_GetPlaybackSpeed;
	static TOptional<bool> bImplemented_GetAudioAssetDuration;
#if WITH_EDITOR
	static TOptional<bool> bImplemented_PreviewAudioAsset;
	static TOptional<bool> bImplemented_GetNewNodeID;
	static TOptional<bool> bImplemented_GetFragmentIDs;
#endif
	
	// Some of these functions may be ran on tick by the editor or during play. We want to log errors, but not spam the log on tick, only on the first occurrence. I also want to make sure the errors log each time PIE runs, not just once.
	static bool bWarned_Initialize;
	static bool bWarned_GetMaturitySetting;
	static bool bWarned_GetPlaybackSpeed;
	static bool bWarned_GetAudioAssetDuration;
#if WITH_EDITOR
	static bool bWarned_PreviewAudioAsset;
#endif
	
public:
	void BeginPlay();

	static UYapBroker& Get(const UObject* WorldContext);

#if WITH_EDITOR
	static UYapBroker& GetInEditor();
#endif
	
#if WITH_EDITOR
public:
	FString GenerateDialogueAudioID(const UFlowNode_YapDialogue* InNode) const;

	void UpdateNodeFragmentIDs(UFlowNode_YapDialogue* InNode) const;
private:
	virtual FString GenerateRandomDialogueAudioID() const;
#endif
	
#if WITH_EDITOR
public:
	bool PreviewAudioAsset_Internal(const UObject* AudioAsset) const;
	
	bool ImplementsPreviewAudioAsset_Internal() const;

private:
	FString GetNewNodeID_DefaultImpl(const FYapAudioIDFormat& IDFormat, const TSet<FString>& ExistingNodeIDs) const;

	bool GetFragmentIDs_Internal(const FYapAudioIDFormat& IDFormat, TArray<int32>& ExistingFragmentIDs) const;

	bool DevelopMissingIDs(TArray<int32>& ExistingFragmentIDInts, const FYapAudioIDFormat& IDFormat) const;

	bool FillInIDSpan(TArray<int32>& ExistingFragmentIDInts, int32 Start, int32 End, int32 LastValidID, int32 NextValidID, const FYapAudioIDFormat& IDFormat) const;
	
#endif

private:
	int32 CalculateWordCount_DefaultImpl(const FText& Text) const;
	
	// Thank you to Blue Man for this thing
	template<typename TFunction, typename... TArgs>
	struct TResolveFunctionReturn
	{
		using Type = std::invoke_result_t<TFunction, UYapBroker, TArgs...>;
	};

	// The purpose of this macro is to call a generic function and simultaneously log warnings for unimplemented functions. It's kind of badly designed
	template<auto TFunction, typename ...TArgs>
	auto CallK2Function(FString FunctionName, TOptional<bool>& bImplemented, bool& bWarned, bool bLogWarnings, TArgs&&... Args) -> typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type
	{
		using TReturn = typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type;
		
		check(bImplemented.IsSet());

		if (bImplemented.GetValue())
		{
			return (this->*TFunction)(std::forward<TArgs>(Args)...);
		}

		if (!bWarned)
		{
			if (bLogWarnings)
			{
				UE_LOG(LogYap, Error, TEXT("Unimplemented broker function: %s, using default implementation (you can suppress these warnings in project settings)"), *FunctionName);
			}

			bWarned = true;
		}
		
		return TReturn{};
	}

	// const variant, exactly the same. Just call the non-const variant.
	template<auto TFunction, typename ...TArgs>
	auto CallK2Function(FString FunctionName, TOptional<bool>& bImplemented, bool& bWarned, bool bLogWarnings, TArgs&&... Args) const -> typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type
	{
		return (const_cast<UYapBroker*>(this))->CallK2Function<TFunction, TArgs...>(FunctionName, bImplemented, bWarned, bLogWarnings, Args...);
	}
};

#undef LOCTEXT_NAMESPACE
