// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapProgressionSettingsWidget.h"

#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapInputTracker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

SLATE_IMPLEMENT_WIDGET(SYapProgressionSettingsWidget)
void SYapProgressionSettingsWidget::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer&)
{
	// TODO wtf does anything go in here?
}

void SYapProgressionSettingsWidget::Construct(const FArguments& InArgs)
{
    SkippableSettingRaw = InArgs._SkippableSettingRaw;
    SkippableEvaluatedAttr = InArgs._SkippableEvaluatedAttr;
    AutoAdvanceSettingRaw = InArgs._AutoAdvanceSettingRaw;
    AutoAdvanceEvaluatedAttr = InArgs._AutoAdvanceEvaluatedAttr;
    
	auto MakeButton = [] (TOptional<bool>* SettingPtr, TAttribute<bool> EvaluatedAttr, FName OffIcon, FName OnIcon, FText OffText, FText OnText) -> TSharedPtr<SButton>
	{
		auto OnClicked = [SettingPtr, EvaluatedAttr] () -> FReply
		{
			TOptional<bool>& Setting = *SettingPtr;

			bool bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();

			if (bCtrlPressed)
			{
				Setting.Reset();
				return FReply::Handled();
			}
			
			if (!Setting.IsSet())
			{
				Setting = !EvaluatedAttr.Get();
			}
			else
			{
				Setting = !Setting.GetValue();
			}

			return FReply::Handled();
		};

		auto ToolTipText = [SettingPtr, EvaluatedAttr, OffText, OnText] () -> FText
		{
			TOptional<bool>& Setting = *SettingPtr;

			bool bEvaluatedValue = EvaluatedAttr.Get();
			
			FText InfoText = Setting.IsSet() ? LOCTEXT("ProgressionSettingButtonInfo_CtrlToReset", "Ctrl-click resets") : LOCTEXT("ProgressionSettingButtonInfo_Unset", "Unset");

			const FText& SettingText = bEvaluatedValue ? OnText : OffText;

			return FText::Format(LOCTEXT("ProgressionSettingButtonToolTip", "{0} ({1})"), SettingText, InfoText);
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

	ChildSlot
	[
		SNew(SBox)
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
		]
	];
}

#undef LOCTEXT_NAMESPACE