// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/Helpers/ProgressionSettingWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TSharedRef<SWidget> PopupContentGetter_ProgressionSettings(TOptional<bool>* bSkippable, TOptional<bool>* bAutoAdvance)
{
	TSharedRef<SWidget> Box = SNew(SVerticalBox)
	+ MakeFragmentProgressionSettingRow(bSkippable, LOCTEXT("SkippableCheckBox_Label", "Skippable"))
	+ MakeFragmentProgressionSettingRow(bAutoAdvance, LOCTEXT("AutoAdvanceCheckBox_Label", "Auto Advance"));

	return Box;
}

// ------------------------------------------------------------------------------------------------

SVerticalBox::FSlot::FSlotArguments MakeFragmentProgressionSettingRow(TOptional<bool>* Setting, FText Label)
{
	SVerticalBox::FSlot::FSlotArguments Slot(SVerticalBox::Slot());
	
	Slot.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda( [Setting] ()
			{
				if (!Setting->IsSet())
				{
					return ECheckBoxState::Undetermined;
				}

				return Setting->GetValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda( [Setting] (ECheckBoxState InState)
			{
				*Setting = (InState == ECheckBoxState::Checked); 
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(STextBlock)
			.Text(Label)
			.SimpleTextMode(true)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(SBox)
			.Visibility_Lambda( [Setting] () { return Setting->IsSet() ? EVisibility::Visible : EVisibility::Hidden; }) 
			.WidthOverride(16)
			.HeightOverride(16)
			[
				SNew(SButton)
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
				.OnClicked_Lambda( [Setting] () { Setting->Reset(); return FReply::Handled(); } )
				.ContentPadding(2)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Reset_Small))
				]
			]
		]
	];

	return Slot;
}

SOverlay::FOverlaySlot::FSlotArguments MakePopupImage(TOptional<bool>* SettingRaw, TAttribute<bool> EvaluatedAttr, FName OffIcon, FName OnIcon)
{
	SOverlay::FOverlaySlot::FSlotArguments Slot(SOverlay::Slot());

	Slot
	[
		SNew(SImage)
		.Image_Lambda( [=] ()
		{			
			bool bEvaluatedValue = EvaluatedAttr.Get();
			
			return FYapEditorStyle::GetImageBrush( bEvaluatedValue ? OnIcon : OffIcon);
		})
		.ColorAndOpacity_Lambda( [=] ()
		{
			if (!SettingRaw->IsSet())
			{
				return YapColor::Button_Unset();
			}
			
			bool bEvaluatedValue = EvaluatedAttr.Get();

			return bEvaluatedValue ? YapColor::LightGreen : YapColor::Orange;  
		})
	];
	
	return Slot;
}

TSharedRef<SWidget> MakeProgressionPopupButton(TOptional<bool>* SkippableSettingRaw, TAttribute<bool> SkippableEvaluatedAttr, TOptional<bool>* AutoAdvanceSettingRaw, TAttribute<bool> AutoAdvanceEvaluatedAttr)
{
	return SNew(SYapButtonPopup)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
		.PopupPlacement(MenuPlacement_RightLeftCenter)
		.ButtonForegroundColor(YapColor::DarkGray_SemiGlass)
		.ButtonContentPadding(0)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.PopupContentGetter(FPopupContentGetter::CreateLambda( [SkippableSettingRaw, AutoAdvanceSettingRaw] () { return PopupContentGetter_ProgressionSettings(SkippableSettingRaw, AutoAdvanceSettingRaw); }))
		.ButtonContent()
		[
			SNew(SBox)
			.WidthOverride(16)
			.HeightOverride(16)
			[
				SNew(SOverlay)
				+ MakePopupImage(SkippableSettingRaw, SkippableEvaluatedAttr, YapBrushes.Icon_NotSkippable, YapBrushes.Icon_Skippable)
				+ MakePopupImage(AutoAdvanceSettingRaw, AutoAdvanceEvaluatedAttr, YapBrushes.Icon_ManualAdvance, YapBrushes.Icon_AutoAdvance)
			]
		];
}

#undef LOCTEXT_NAMESPACE
