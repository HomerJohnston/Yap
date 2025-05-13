// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "Yap/Enums/YapTimeMode.h"
#include "Templates/SharedPointer.h"

class IYapCharacterInterface;
class SYapDialogueEditor;
struct FYapTypeGroupSettings;
enum class EYapDialogueProgressionFlags : uint8;
class UYapCharacterAsset;
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

enum class EYapTextType : uint8
{
	Speech,
	TitleText
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
		SLATE_ARGUMENT(SFlowGraphNode_YapDialogueWidget*, InOwner)
		SLATE_ARGUMENT(uint8, InFragmentIndex)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs);
		
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

	// Used to change the click-behavior of some buttons
	bool bShiftPressed = false;
	
	// Used to hold temporary overlay widgets such as the fragment up-delete-down controls  
	TSharedPtr<SOverlay> FragmentWidgetOverlay = nullptr;

	// Holds the fragment up-delete-down controls
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;

	// Holds the dialogue text, title text, and time indicators - hover state can be used to change the color of some elements when hovered
	TSharedPtr<SOverlay> FragmentTextOverlay = nullptr;

	//
	TSharedPtr<SWidget> ChildSafeCheckBox = nullptr;

	//
	TSharedPtr<SWidget> DirectedAtWidget = nullptr;

	//
	TSharedPtr<SWidget> SpeakerDropTarget = nullptr;

	//
	TSharedPtr<SWidget> AudioIDButton = nullptr;

	//
	TWeakPtr<SYapDialogueEditor> ExpandedDialogueEditor;
	
	//
	bool bChildSafeCheckBoxHovered = false;

	//
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

	EVisibility 			Visibility_AudioSettingsButton() const;
	EVisibility 			Visibility_DialogueErrorState() const;
	FSlateColor 			ColorAndOpacity_AudioIDText() const;
	EVisibility				Visibility_TimeProgressionWidget() const;
	
	// ------------------------------------------
	
	TSharedRef<SWidget>		CreateDialogueDisplayWidget();
	
	TSharedRef<SWidget>		CreateCentreTextDisplayWidget();
	
	TSharedRef<SWidget>		PopupContentGetter_ExpandedEditor();

	void					OnPostPopup_TextEditor(bool& bOverrideFocus, EYapTextType TextType, EYapMaturitySetting Maturity);
	
	// ------------------------------------------
	
	TOptional<float>		Percent_FragmentTime() const;
	FLinearColor 			ColorAndOpacity_FragmentTimeIndicator() const;
	bool					Bool_PaddingTimeIsSet() const;

	// ------------------------------------------
	
	TSharedRef<SWidget>		CreateSpeakerWidget();

	void					OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);

	TSharedRef<SWidget>		CreateSpeakerImageWidget(int32 PortraitSize, int32 BorderSize);
	FSlateColor				BorderBackgroundColor_SpeakerImage() const;
	const FSlateBrush*		Image_SpeakerImage() const;
	FText					ToolTipText_SpeakerWidget() const;
	FText					Text_SpeakerWidget() const;

	float					GetSpeakerWidgetSize(int32 PortraitSize, int32 BorderSize) const;

	TSharedRef<SWidget>		CreateSpeakerImageWidget_LowDetail(int32 PortraitSize, int32 BorderSize);

	// ------------------------------------------

	TSharedRef<SWidget>		CreateDirectedAtWidget();

	void					OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	
	FSlateColor				BorderBackgroundColor_DirectedAtImage() const;
	FReply					OnClicked_DirectedAtWidget();
	TSharedRef<SWidget>		PopupContentGetter_DirectedAtWidget();
	const FSlateBrush*		Image_DirectedAtWidget()const;
	void					OnSetNewDirectedAtAsset(const FAssetData& AssetData);
	
	// ------------------------------------------
	
	TSharedRef<SWidget>		PopupContentGetter_SpeakerWidget(const UObject* Character);
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

	FSlateColor				ColorAndOpacity_AudioIDButton() const;
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

	bool FragmentRecentlyRan() const;
	
	// ------------
	bool IsDroppedAsset_YapSpeaker(TArrayView<FAssetData> AssetDatas) const;

	bool IsAsset_YapSpeaker(const FAssetData& AssetData) const;
	
	bool ShouldFilter_YapSpeaker(const FAssetData& AssetData) const;

	// ------------
	FSlateColor	GetColorAndOpacityForFragmentText(FLinearColor BaseColor) const;

	const FYapTypeGroupSettings& GetTypeGroup() const;

public:
	bool GetIsChildSafeCheckBoxHovered() const { return bChildSafeCheckBoxHovered; };
	
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
