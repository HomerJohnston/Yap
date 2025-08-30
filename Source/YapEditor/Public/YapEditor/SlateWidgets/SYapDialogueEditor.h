// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"
#include "CoreTypes.h"
#include "Widgets/SCompoundWidget.h"

class UYapNodeConfig;
class SFlowGraphNode_YapFragmentWidget;
enum class EYapTimeMode : uint8;
struct FYapFragment;
class UFlowNode_YapDialogue;
struct FYapBit;
enum class EYapMaturitySetting : uint8;
enum class EYapErrorLevel : uint8;

enum class EYapTextType : uint8
{
	Unspecified,
	Speech,
	TitleText
};

/***
 * The actual popup editor for dialogue, title text, audio, time settings, etc.
 */
class SYapDialogueEditor : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SYapDialogueEditor)
	{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNodeIn)
		SLATE_ARGUMENT(uint8, FragmentIndexIn)
		SLATE_ARGUMENT(bool, bNeedsChildSafeIn)
		SLATE_ARGUMENT(TSharedPtr<SFlowGraphNode_YapFragmentWidget>, OwnerIn)

		SLATE_ARGUMENT(EYapTextType, InitialFocusText)
		SLATE_ARGUMENT(EYapMaturitySetting, InitialFocusMaturity)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueNode = nullptr;
	
	uint8 FragmentIndex = 0;

	bool bNeedsChildSafe = false;
	
	// Color lookup table for buttons and indicators
	static TMap<EYapTimeMode, FLinearColor> TimeModeButtonColors;

	TSharedPtr<SFlowGraphNode_YapFragmentWidget> Owner;

	// -----------

	TSharedPtr<SWidget> MatureDialogueText;

	TSharedPtr<SWidget> ChildSafeDialogueText;
	
	TSharedPtr<SWidget> MatureTitleText;

	TSharedPtr<SWidget> ChildSafeTitleText;

	EYapTextType InitialFocusText;

	EYapMaturitySetting InitialFocusMaturity;

	// -----------
	TSharedRef<SWidget> 	BuildDialogueEditors_ExpandedEditor(float Width);
	
	TSharedRef<SWidget> 	BuildDialogueEditor_SingleSide(const FText& Title, const FText& DialogueTextHint, const FText& TitleTextHint, float Width, FMargin Padding, FYapBit& Bit, TSharedPtr<SWidget>& DialogueTextWidget, TSharedPtr<SWidget>& TitleTextWidget);
	
	TSharedRef<SWidget> 	BuildCommentEditor(TAttribute<FString> String, FString* StringProperty, FText HintText);
	
	TSharedRef<SWidget> 	BuildTimeSettings_ExpandedEditor(float Width);
	
	TSharedRef<SWidget> 	BuildTimeSettings_SingleSide(float Width, FMargin Padding, EYapMaturitySetting MaturitySetting);
		
	TSharedRef<SWidget> 	BuildPaddingSettings_ExpandedEditor(float Width);

	FYapFragment&			GetFragment() const;
	
	FSlateColor				ButtonColorAndOpacity_PaddingButton() const;
	
	TSharedRef<SWidget> 	CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset);
	
	TSharedRef<SWidget>		CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> VisibilityAtt);
	
	bool					OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const;
	
	EVisibility				Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;
	
	FSlateColor				ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;
	
	bool					Enabled_AudioPreviewButton(const TSoftObjectPtr<UObject>* Object) const;
	
	TSharedRef<SWidget>		MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting);
	
	FReply					OnClicked_AudioPreviewWidget(const TSoftObjectPtr<UObject>* Object);
	
	EYapErrorLevel			GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const;
	
	TOptional<float> 		Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const;

	TOptional<float> 		Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const;

	TOptional<float> 		Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const;
	
	void					OnValueUpdated_ManualTime(float NewValue, EYapMaturitySetting MaturitySetting);
	
	void					OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType, EYapMaturitySetting MaturitySetting);
	
	FReply					OnClicked_SetTimeModeButton(EYapTimeMode TimeMode);
	
	FSlateColor				ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint, EYapMaturitySetting MaturitySetting) const;
	
	FSlateColor				ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const;

	// TODO this should NOT be required, it exists because of a limitation in my GetFragment()->GetTimeMode function, fix it!
	EYapMaturitySetting	 	GetDisplayMaturitySetting() const;
	
	const UYapNodeConfig&	GetNodeConfig() const;

public:
	bool SupportsKeyboardFocus() const override { return true; }
	
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent);
	
	void SetFocus_MatureDialogue();

	void SetFocus_MatureTitleText();

	void SetFocus_ChildSafeDialogue();

	void SetFocus_ChildSafeTitleText();
	
	void PressEndKey();
	
};

#undef LOCTEXT_NAMESPACE
