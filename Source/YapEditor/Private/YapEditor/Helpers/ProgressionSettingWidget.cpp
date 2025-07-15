// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/Helpers/ProgressionSettingWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

/*
TSharedRef<SWidget> PopupContentGetter_ProgressionSettings(TOptional<bool>* bSkippable, TOptional<bool>* bAutoAdvance)
{
	TSharedRef<SWidget> Box = SNew(SVerticalBox)
	+ MakeFragmentProgressionSettingRow(bSkippable, LOCTEXT("SkippableCheckBox_Label", "Skippable"))
	+ MakeFragmentProgressionSettingRow(bAutoAdvance, LOCTEXT("AutoAdvanceCheckBox_Label", "Auto Advance"));

	return Box;
}
*/

// ------------------------------------------------------------------------------------------------

/*
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
*/

/*
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
*/

/*
TSharedRef<SWidget> MakeProgressionPopupButton(TOptional<bool>* SkippableSettingRaw, TAttribute<bool> SkippableEvaluatedAttr, TOptional<bool>* AutoAdvanceSettingRaw, TAttribute<bool> AutoAdvanceEvaluatedAttr)
{
	auto MakeButton = [] (TOptional<bool>* SettingPtr, TAttribute<bool> EvaluatedAttr, FName OffIcon, FName OnIcon, FText OffText, FText OnText) -> TSharedPtr<SButton>
	{
		auto OnClicked = [SettingPtr] () -> FReply
		{
			TOptional<bool>& Setting = *SettingPtr;
			
			if (!Setting.IsSet())
			{
				Setting = true;
			}
			else
			{
				Setting = !Setting.GetValue();
			}

			return FReply::Handled();
		};

		auto ToolTipText = [SettingPtr, OffText, OnText] () -> FText
		{
			if (!SettingPtr->IsSet())
			{
				return LOCTEXT("ProgressionSetting_Tooltip_Unset", "Unset");
			}

			return SettingPtr->GetValue() ? OnText : OffText;
		};
		
		return SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
		.ContentPadding(0)
		.OnClicked_Lambda(OnClicked)
		.ToolTipText_Lambda(ToolTipText)
		[
			SNew(SImage)
			.Image_Lambda( [EvaluatedAttr, OffIcon, OnIcon] ()
			{			
				bool bEvaluatedValue = EvaluatedAttr.Get();
			
				return FYapEditorStyle::GetImageBrush( bEvaluatedValue ? OnIcon : OffIcon);
			})
			.ColorAndOpacity_Lambda( [=] ()
			{
				if (!SettingPtr->IsSet())
				{
					return YapColor::Button_Unset();
				}
			
				bool bEvaluatedValue = EvaluatedAttr.Get();

				return bEvaluatedValue ? YapColor::LightGreen : YapColor::Orange;  
			})
		];
	};

	FText NotSkippable = LOCTEXT("SkippableButtonToolTip_NotSkippable", "Not Skippable");
	FText Skippable = LOCTEXT("SkippableButtonToolTip_Skippable", "Skippable");

	FText ManualAdvance = LOCTEXT("AutoAdvanceButtonToolTip_ManualAdvance", "Manual Advance");
	FText AutoAdvance = LOCTEXT("AutoAdvanceButtonToolTip_AutoAdvance", "Auto Advance");
	
	return SNew(SBox)
	.WidthOverride(16)
	.HeightOverride(16)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SBox)
			.HeightOverride(8)
			[
				MakeButton(SkippableSettingRaw, SkippableEvaluatedAttr, YapBrushes.Icon_NotSkippable, YapBrushes.Icon_Skippable, NotSkippable, Skippable).ToSharedRef()
			]
		]
		+ SVerticalBox::Slot()
		[
			SNew(SBox)
			.HeightOverride(8)
			[
				MakeButton(AutoAdvanceSettingRaw, AutoAdvanceEvaluatedAttr, YapBrushes.Icon_ManualAdvance, YapBrushes.Icon_AutoAdvance, ManualAdvance, AutoAdvance).ToSharedRef()
			]
		]
	];
}
*/

#undef LOCTEXT_NAMESPACE
