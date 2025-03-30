// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/Enums/YapTimeMode.h"

struct FYapTypeGroupSettings;
enum class EYapDialogueProgressionFlags : uint8;
class UYapCharacter;
class SYapConditionsScrollBox;
class UYapCondition;
class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
class SMultiLineEditableTextBox;
class UYapBroker;

struct FYapBit;
struct FFlowPin;
struct FYapBitReplacement;
struct FYapFragment;
struct FGameplayTag;

enum class EYapTimeMode : uint8;
enum class EYapMissingAudioErrorLevel : uint8;
enum class EYapErrorLevel : uint8;
enum class EYapMaturitySetting : uint8;

#define LOCTEXT_NAMESPACE "YapEditor"

enum class EYapFragmentControlsDirection : uint8
{
	Up,
	Down,
};

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
	// ==========================================
	// CONSTRUCTION
	// ==========================================
private:
	SLATE_DECLARE_WIDGET(SFlowGraphNode_YapFragmentWidget, SCompoundWidget)

	SLATE_BEGIN_ARGS(SFlowGraphNode_YapFragmentWidget)
	{}
		
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex); // non-virtual override
		
	// ==========================================
	// STATE
	// ==========================================
protected:
	
	// Owner of this fragment
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	// Index of this fragment inside the dialogue node
	uint8 FragmentIndex = 0;

	// Color lookup table for buttons and indicators
	static TMap<EYapTimeMode, FLinearColor> TimeModeButtonColors;
	
	// Container for speech ending output pin
	TSharedPtr<SBox>	EndPinBox = nullptr;
	
	// Container for speech starting output pin
	TSharedPtr<SBox>	StartPinBox = nullptr;

	// Container for chosen prompt output pin
	TSharedPtr<SBox>	PromptOutPinBox = nullptr;

	// Used to change the click-behavior of some buttons
	bool bCtrlPressed = false;

	// Used to hold temporary overlay widgets such as the fragment up-delete-down controls  
	TSharedPtr<SOverlay> FragmentWidgetOverlay = nullptr;

	// Holds the fragment up-delete-down controls
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;

	// Holds the dialogue text, title text, and time indicators - hover state can be used to change the color of some elements when hovered
	TSharedPtr<SOverlay> FragmentTextOverlay = nullptr;

	TSharedPtr<SWidget> ChildSafeCheckBox = nullptr;

	bool bChildSafeCheckBoxHovered = false;

	static FSlateFontInfo DialogueTextFont;

public:
	// ================================================================================================
	// WIDGETS
	// ================================================================================================
protected:
	
	int32					GetFragmentActivationCount() const;
	int32					GetFragmentActivationLimit() const;
	EVisibility				Visibility_FragmentControlsWidget() const;
	EVisibility				Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const;
	FReply					OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection);
	FReply					OnClicked_FragmentDelete();
	TSharedRef<SWidget>		CreateFragmentControlsWidget();
	bool					Enabled_AudioPreviewButton(const TSoftObjectPtr<UObject>* Object) const;
	FReply					OnClicked_AudioPreviewWidget(const TSoftObjectPtr<UObject>* Object);
	TSharedRef<SWidget>		CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> VisibilityAtt);

	// ------------------------------------------

	TSharedRef<SWidget> 	CreateFragmentHighlightWidget();
	EVisibility				Visibility_FragmentHighlight() const;
	FSlateColor				BorderBackgroundColor_FragmentHighlight() const;
	
	// ------------------------------------------

	void					OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg);

	TSharedRef<SWidget> 	CreateUpperFragmentBar();
	EVisibility				Visibility_FragmentTagWidget() const;
		
	ECheckBoxState			IsChecked_ChildSafeSettings() const;
	void					OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState);
	FSlateColor				ColorAndOpacity_ChildSafeSettingsCheckBox() const;
	
	bool 					OnAreAssetsAcceptableForDrop_ChildSafeButton(TArrayView<FAssetData> AssetDatas) const;
	void 					OnAssetsDropped_ChildSafeButton(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	// ------------------------------------------
	
	TSharedRef<SWidget>		CreateFragmentWidget();

	// ------------------------------------------

	TOptional<float> 		Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const;
	TOptional<float> 		Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const;
	TOptional<float> 		Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const;

	TSharedRef<SWidget>		MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting);

	EVisibility 			Visibility_AudioSettingsButton() const;
	EVisibility 			Visibility_DialogueErrorState() const;
	FSlateColor 			ColorAndOpacity_AudioID() const;
	
	// ------------------------------------------
	
	TSharedRef<SWidget>		CreateDialogueDisplayWidget();
	
	FText					Text_TextDisplayWidget(const FText* MatureText, const FText* SafeText) const;
	
	EVisibility				Visibility_DialogueBackground() const;
	FSlateColor				BorderBackgroundColor_Dialogue() const;

	TSharedRef<SWidget>		CreateCentreTextDisplayWidget();
	TSharedRef<SWidget>		PopupContentGetter_ExpandedEditor();


	TSharedRef<SWidget> 	BuildDialogueEditors_ExpandedEditor(float Width);
	
	TSharedRef<SWidget> 	BuildDialogueEditor_SingleSide(const FText& Title, const FText& DialogueTextHint, const FText& TitleTextHint, float Width, FMargin Padding, FYapBit& Bit);
	
	TSharedRef<SWidget> 	BuildCommentEditor(TAttribute<FString> String, FString* StringProperty, FText HintText);
	
	TSharedRef<SWidget> 	BuildTimeSettings_ExpandedEditor(float Width);
	
	TSharedRef<SWidget> 	BuildTimeSettings_SingleSide(float Width, FMargin Padding, EYapMaturitySetting MaturitySetting);
		
	TSharedRef<SWidget> 	BuildPaddingSettings_ExpandedEditor(float Width);
	
	// ------------------------------------------
	TOptional<float>		Percent_FragmentTime() const;
	FLinearColor 			ColorAndOpacity_FragmentTimeIndicator() const;

	// ------------------------------------------
	
	TSharedRef<SOverlay>	CreateSpeakerWidget();

	void					OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);

	TSharedRef<SWidget>		CreateSpeakerImageWidget(int32 PortraitSize, int32 BorderSize);
	FSlateColor				BorderBackgroundColor_CharacterImage() const;
	const FSlateBrush*		Image_SpeakerImage() const;
	FText					ToolTipText_SpeakerWidget() const;
	FText					Text_SpeakerWidget() const;

	float					GetSpeakerWidgetSize(int32 PortraitSize, int32 BorderSize) const;

	// ------------------------------------------

	TSharedRef<SWidget>		CreateDirectedAtWidget();

	void					OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	
	FSlateColor				BorderBackgroundColor_DirectedAtImage() const;
	FReply					OnClicked_DirectedAtWidget();
	TSharedRef<SWidget>		PopupContentGetter_DirectedAtWidget();
	const FSlateBrush*		Image_DirectedAtWidget()const;
	void					OnSetNewDirectedAtAsset(const FAssetData& AssetData);
	
	// ------------------------------------------
	
	TSharedRef<SWidget>		PopupContentGetter_SpeakerWidget(const UYapCharacter* Character);
	void					OnSetNewSpeakerAsset(const FAssetData& AssetData);

	// ------------------------------------------

	bool					OnAreAssetsAcceptableForDrop_TextWidget(TArrayView<FAssetData> AssetDatas) const;
	void					OnAssetsDropped_TextWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	
	// ------------------------------------------

	// ------------------------------------------
	
	TSharedRef<SWidget>		CreateMoodTagSelectorWidget();

	FGameplayTag			GetCurrentMoodTag() const;
	
	FText					ToolTipText_MoodTagSelector() const;
	FSlateColor				ForegroundColor_MoodTagSelectorWidget() const;
	const FSlateBrush*		Image_MoodTagSelector() const;
	
	// ------------------------------------------
	TSharedRef<SWidget>		CreateMoodTagMenuEntryWidget(FGameplayTag InIconName, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply					OnClicked_MoodTagMenuEntry(FGameplayTag NewValue);


	// ------------------------------------------
	TSharedRef<SWidget>		CreateTitleTextDisplayWidget();

	EVisibility				Visibility_TitleTextWidgets() const;
	EVisibility				Visibility_TitleTextErrorState() const;

	// ------------------------------------------
	TSharedRef<SWidget>		CreateFragmentTagWidget();
	
	FGameplayTag			Value_FragmentTag() const;
	void					OnTagChanged_FragmentTag(FGameplayTag GameplayTag);

	// ------------------------------------------

	FReply					OnClicked_SetTimeModeButton(EYapTimeMode TimeMode);

	void					OnValueUpdated_ManualTime(float NewValue, EYapMaturitySetting MaturitySetting);
	void					OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType, EYapMaturitySetting MaturitySetting);
	FSlateColor				ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint, EYapMaturitySetting MaturitySetting) const;
	FSlateColor				ButtonColorAndOpacity_PaddingButton() const;
	FSlateColor				ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const;

	// ------------------------------------------
	
	TSharedRef<SWidget> 	CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset);

	bool					OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const;
	EVisibility				Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;

	FSlateColor				ColorAndOpacity_AudioSettingsButton() const;
	EYapErrorLevel			GetFragmentAudioErrorLevel() const;

	FSlateColor				ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;
	EYapErrorLevel			GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const;
	
	// ================================================================================================
	// HELPERS
	// ================================================================================================
protected:

	// ------------
	// TODO oh my god can I reduce this cruft at all
	const UFlowNode_YapDialogue* GetDialogueNode() const;

	UFlowNode_YapDialogue* GetDialogueNodeMutable();

	const FYapFragment& GetFragment() const;
	
	FYapFragment& GetFragmentMutable();

	FYapFragment& GetFragmentMutable() const;
	
	// ------------
	EYapMaturitySetting GetDisplayMaturitySetting() const;
	
	bool NeedsChildSafeData() const;

	bool HasAnyChildSafeData() const;
	
	bool HasCompleteChildSafeData() const;

	bool FragmentIsRunning() const;
	
	// ------------
	bool IsDroppedAsset_YapCharacter(TArrayView<FAssetData> AssetDatas) const;

	// ------------
	FSlateColor	GetColorAndOpacityForFragmentText(FLinearColor BaseColor) const;

	const FYapTypeGroupSettings& GetTypeGroup() const;
	
	// ================================================================================================
	// OVERRIDES
	// ================================================================================================
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FSlateColor ColorAndOpacity_FragmentDataIcon() const;
	TSharedRef<SWidget>	CreateRightFragmentPane();

	
	TSharedPtr<SBox> GetPinContainer(const FFlowPin& Pin);
	
	EVisibility			Visibility_EnableOnStartPinButton() const;
	EVisibility			Visibility_EnableOnEndPinButton() const;
	
	FReply				OnClicked_EnableOnStartPinButton();
	FReply				OnClicked_EnableOnEndPinButton();
};

#undef LOCTEXT_NAMESPACE
