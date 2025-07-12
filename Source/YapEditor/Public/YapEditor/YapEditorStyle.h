// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ==============================================

struct FYapBrushes
{
	FName None;
	
	// Existing UnrealEd Icons
	FName Icon_FilledCircle;
	FName Icon_Chevron_Right;
	FName Icon_Caret_Right;
	FName Icon_PlusSign;
	FName Icon_ProjectSettings_TabIcon;
	
	// New Icons
	FName Icon_AudioTime;
	FName Icon_Baby;
	FName Icon_CornerDropdown_Right;
	FName Icon_Checkmark;
	FName Icon_CrossX;
	FName Icon_Delete;
	FName Icon_DownArrow;
	FName Icon_Edit;
	FName Icon_IDTag;

	FName Icon_LocalLimit;

	FName Icon_MoodTag_Missing;
	FName Icon_MoodTag_None;
	FName Icon_Speaker;
	FName Icon_Tag;
	FName Icon_TextTime;
	FName Icon_Timer;
	FName Icon_UpArrow;
	FName Icon_Skippable;
	FName Icon_NotSkippable;
	FName Icon_AutoAdvance;
	FName Icon_ManualAdvance;
	FName Icon_Reset_Small;
	FName Icon_Notes;
	FName Icon_FragmentData;
	
	FName Icon_Reconciled;
	FName Icon_Unreconciled;
	
	FName Icon_Switch_On;
	FName Icon_Switch_Off;

	FName Icon_PlaybackTimeHandle;
	
	FName Icon_Random;

	FName Bar_NegativePadding;
	FName Bar_PositivePadding;
	
	FName Border_DeburredSquare;
	FName Border_RoundedSquare;
	FName Border_Thick_RoundedSquare;
	FName Border_SharpSquare;
	
	FName Box_SolidLightGray;
	FName Box_SolidLightGray_Deburred;
	FName Box_SolidLightGray_Rounded;
	
	FName Box_SolidWhite;
	FName Box_SolidWhite_Deburred;
	FName Box_SolidWhite_Rounded;
	
	FName Box_SolidRed;
	FName Box_SolidRed_Deburred;
	FName Box_SolidRed_Rounded;

	FName Box_SolidNoir;
	FName Box_SolidNoir_Deburred;
	FName Box_SolidNoir_Rounded;
	
	FName Box_SolidBlack;
	FName Box_SolidBlack_Deburred;
	FName Box_SolidBlack_Rounded;
	
	FName Panel_Deburred;
	FName Panel_Rounded;
	FName Panel_Sharp;
	
	FName Outline_White_Deburred;

	FName Pin_OptionalOutput;

	FName Test; // TODO remove this eventually. This is just a placeholder for testing syntax in the .cpp file.
};

struct FYapStyles
{
	// Existing Unreal Editor styles for style consistency
	FName ButtonStyle_NoBorder;
	FName ButtonStyle_HoverHintOnly;
	FName ButtonStyle_SimpleButton;

	// New button styles
	FName ButtonStyle_ActivationLimit;
	FName ButtonStyle_AudioPreview;
	FName ButtonStyle_ConditionWidget;
	FName ButtonStyle_FragmentControls;
	FName ButtonStyle_HeaderButton;
	FName ButtonStyle_SimpleYapButton;
	FName ButtonStyle_DialogueSettings;
	FName ButtonStyle_SequencingSelector;
	FName ButtonStyle_SpeakerPopup;
	FName ButtonStyle_TimeSetting;
	FName ButtonStyle_TimeSettingOpener;
	FName ButtonStyle_TagButton;

	FName CheckBoxStyle_Skippable;
	FName CheckBoxStyle_TypeSettingsOverride;
	
	FName ComboButtonStyle_YapGameplayTagTypedPicker;
	
	FName EditableTextBoxStyle_Dialogue;
	FName EditableTextBoxStyle_TitleText;
	
	FName ProgressBarStyle_FragmentTimePadding;
	
	FName SliderStyle_FragmentTimePadding;
	
	FName ScrollBarStyle_DialogueBox;

	FName ScrollBoxStyle_Test;
	
	FName TextBlockStyle_DialogueText;
	FName TextBlockStyle_NodeHeader;
	FName TextBlockStyle_GroupLabel;
	FName TextBlockStyle_NodeSequencing;
	FName TextBlockStyle_TitleText;
};

struct FYapFonts
{
	FSlateFontInfo Font_DialogueText;
	FSlateFontInfo Font_TitleText;
	FSlateFontInfo Font_NodeHeader;	
	FSlateFontInfo Font_GroupLabel;	
	FSlateFontInfo Font_SectionHeader;	
	FSlateFontInfo Font_NodeSequencing;	
	FSlateFontInfo Font_CharacterAssetThumbnail;

	FSlateFontInfo Font_WarningText;

	FSlateFontInfo Font_OpenSans_Regular;
	FSlateFontInfo Font_NotoSans_Regular;
	FSlateFontInfo Font_NotoSans_SemiBold;

	FSlateFontInfo Font_BeVietnam_Light;
	FSlateFontInfo Font_BeVietnam_Regular;
};

extern FYapFonts YapFonts;
extern FYapBrushes YapBrushes;
extern FYapStyles YapStyles;

class FYapEditorStyle
{
public:
	static TArray<TStrongObjectPtr<UTexture2D>> Textures;
	
	static ISlateStyle& Get();

	static const FSlateBrush* GetImageBrush(FName BrushName)
	{
		return Get().GetBrush(BrushName);
	}
	
	FYapEditorStyle();
	virtual ~FYapEditorStyle();

	static FName GetStyleSetName();

	static void Initialize();

	static void ReloadTextures();
	
protected:
	static TSharedRef< class FSlateStyleSet > Create();
	
	static void Initialize_Internal();
	
	void OnPatchComplete();
	FDelegateHandle OnPatchCompleteHandle;

	static TSharedPtr<FSlateStyleSet> StyleInstance;

	static const ISlateStyle* GetParentStyle();
	
private:
	// DO NOT USE THIS FOR ANYTHING. It's a dumb macro placeholder.
	FSlateImageBrush* TEMP;
};

#undef LOCTEXT_NAMESPACE