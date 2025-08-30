// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapProgressionSettingsWidget.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Yap/Enums/YapAutoAdvanceFlags.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/YapTransactions.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------

SLATE_IMPLEMENT_WIDGET(SYapProgressionSettingsWidget)
void SYapProgressionSettingsWidget::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer&)
{
	// TODO wtf does anything go in here?
}

// ------------------------------------------------------------------------------------------------

void SYapProgressionSettingsWidget::Construct(const FArguments& InArgs)
{
	DialogueNode = InArgs._DialogueNode;
	FragmentIndex = InArgs._FragmentIndex;
    InterruptibleSettingRaw = InArgs._SkippableSettingRaw;
    AutoAdvanceSettingRaw = InArgs._AutoAdvanceSettingRaw;
    
	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(20)
		.HeightOverride(20)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(8)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
					.ContentPadding(0)
					.OnClicked(this, &ThisClass::OnClicked_InterruptibleToggle)
					.ToolTipText(this, &ThisClass::ToolTipText_InterruptibleToggle)
					[
						SNew(SImage)
						.Image(this, &ThisClass::Image_InterruptibleToggle)
						.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_InterruptibleToggle)
					]
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(8)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
					.ContentPadding(0)
					.OnClicked(this, &ThisClass::OnClicked_AutoAdvanceToggle)
					.ToolTipText(this, &ThisClass::ToolTipText_AutoAdvanceToggle)
					[
						SNew(SImage)
						.Image(this, &ThisClass::Image_AutoAdvanceToggle)
						.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AutoAdvanceToggle)
					]
				]
			]
		]
	];
}

// ------------------------------------------------------------------------------------------------

FReply SYapProgressionSettingsWidget::OnClicked_AutoAdvanceToggle() const
{
	if (IsCtrlPressed())
	{
		FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), nullptr);

		AutoAdvanceSettingRaw->Reset();
		
		return FReply::Handled();
	}

	static const TArray<uint8> AutoAdvanceToggles
	{
		(uint8)(EYapAutoAdvanceFlags::None),
		(uint8)(EYapAutoAdvanceFlags::FreeSpeech),
		(uint8)(EYapAutoAdvanceFlags::Conversation),
		(uint8)(EYapAutoAdvanceFlags::FreeSpeech | EYapAutoAdvanceFlags::Conversation),
	};

	{
		uint8 Current;

		if (AutoAdvanceSettingRaw->IsSet())
		{
			Current = (uint8)(AutoAdvanceSettingRaw->GetValue());
		}
		else
		{
			if (DialogueNode.IsValid())
			{
				Current = (uint8)(DialogueNode->GetNodeConfig().DialoguePlayback.AutoAdvanceFlags);
			}
			else
			{
				Current = 255;
			}
		}

		uint8 Index = AutoAdvanceToggles.Find(Current);

		if (Index == INDEX_NONE)
		{
			Index = 0;
		}
		else
		{
			++Index;
		}

		if (Index >= AutoAdvanceToggles.Num())
		{
			Index = 0;
		}

		FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), nullptr);
		
		*AutoAdvanceSettingRaw = (EYapAutoAdvanceFlags)(AutoAdvanceToggles[Index]);
	}
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------

FReply SYapProgressionSettingsWidget::OnClicked_InterruptibleToggle() const
{
	if (IsCtrlPressed())
	{
		FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), nullptr);

		InterruptibleSettingRaw->Reset();
		
		return FReply::Handled();
	}

	static const TArray<uint8> InterruptibleToggles
	{
		(uint8)(EYapInterruptibleFlags::None),
		(uint8)(EYapInterruptibleFlags::FreeSpeech),
		(uint8)(EYapInterruptibleFlags::Conversation),
		(uint8)(EYapInterruptibleFlags::FreeSpeech | EYapInterruptibleFlags::Conversation),
	};

	{
		uint8 Current = GetInterruptibleValue();

		uint8 Index = InterruptibleToggles.Find(Current);

		if (Index == INDEX_NONE)
		{
			Index = 0;
		}
		else
		{
			++Index;
		}

		if (Index >= InterruptibleToggles.Num())
		{
			Index = 0;
		}

		FYapScopedTransaction Transaction(NAME_None, INVTEXT("TODO"), nullptr);
		
		*InterruptibleSettingRaw = (EYapInterruptibleFlags)(InterruptibleToggles[Index]);
	}
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------

FText SYapProgressionSettingsWidget::ToolTipText_AutoAdvanceToggle() const
{
	uint8 Current = GetAutoAdvanceValue();

	FText BaseText = INVTEXT("Error");
	
	switch (Current)
	{
		case (uint8)(EYapAutoAdvanceFlags::None):
		{ BaseText = LOCTEXT("ToolTipText_AutoAdvanceState_None", "Requires manual advancing"); break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech):
		{ BaseText = LOCTEXT("ToolTipText_AutoAdvanceState_FreeSpeech", "Auto-advances free speech"); break; }

		case (uint8)(EYapAutoAdvanceFlags::Conversation):
		{ BaseText = LOCTEXT("ToolTipText_AutoAdvanceState_Conversation", "Auto-advances in conversations"); break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech | EYapAutoAdvanceFlags::Conversation):
		{ BaseText = LOCTEXT("ToolTipText_AutoAdvanceState_All", "Auto-advances"); break; }
		
		default: {}	
	}

	FText SuffixText = IsAutoAdvanceOverridden() ? LOCTEXT("ToolTipText_ProgressionSetting_ResetHint", "(ctrl-click resets)") : LOCTEXT("ToolTipText_ProgressionSetting_DefaultHint", "(default)");

	TArray<FText> Texts = { BaseText, SuffixText };
	
	return FText::Join(FText::FromString(TEXT(" ")), Texts);
}

// ------------------------------------------------------------------------------------------------

FText SYapProgressionSettingsWidget::ToolTipText_InterruptibleToggle() const
{
	uint8 Current = GetInterruptibleValue();

	FText BaseText = INVTEXT("Error");
	
	switch (Current)
	{
		case (uint8)(EYapInterruptibleFlags::None):
		{ BaseText = LOCTEXT("ToolTipText_InterruptibleState_None", "Playback is not interruptible"); break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech):
		{ BaseText = LOCTEXT("ToolTipText_InterruptibleState_FreeSpeech", "Playback interruptible for free speech"); break; }

		case (uint8)(EYapInterruptibleFlags::Conversation):
		{ BaseText = LOCTEXT("ToolTipText_InterruptibleState_Conversation", "Playback interruptible in conversations"); break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech | EYapInterruptibleFlags::Conversation):
		{ BaseText = LOCTEXT("ToolTipText_InterruptibleState_All", "Playback is interruptible"); break; }
		
		default: {}
	}
	
	FText SuffixText = IsInterruptibleOverridden() ? LOCTEXT("ToolTipText_ProgressionSetting_ResetHint", "(ctrl-click resets)") : LOCTEXT("ToolTipText_ProgressionSetting_DefaultHint", "(default)");

	TArray<FText> Texts = { BaseText, SuffixText };
	
	return FText::Join(FText::FromString(TEXT(" ")), Texts);
}

// ------------------------------------------------------------------------------------------------

const FSlateBrush* SYapProgressionSettingsWidget::Image_AutoAdvanceToggle() const
{
	uint8 Current = GetAutoAdvanceValue();

	FName BrushName = NAME_None;

	switch (Current)
	{
		case (uint8)(EYapAutoAdvanceFlags::None):
		{ BrushName = YapBrushes.Icon_AutoAdvance_None; break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech):
		{ BrushName = YapBrushes.Icon_AutoAdvance_FreeSpeech; break; }

		case (uint8)(EYapAutoAdvanceFlags::Conversation):
		{ BrushName = YapBrushes.Icon_AutoAdvance_Conversation; break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech | EYapAutoAdvanceFlags::Conversation):
		{ BrushName = YapBrushes.Icon_AutoAdvance_All; break; }
		
		default: {}	
	}

	return FYapEditorStyle::GetImageBrush(BrushName);
}

// ------------------------------------------------------------------------------------------------

const FSlateBrush* SYapProgressionSettingsWidget::Image_InterruptibleToggle() const
{
	uint8 Current = GetInterruptibleValue();

	FName BrushName = NAME_None;

	switch (Current)
	{
		case (uint8)(EYapInterruptibleFlags::None):
		{ BrushName = YapBrushes.Icon_Interruptible_None; break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech):
		{ BrushName = YapBrushes.Icon_Interruptible_FreeSpeech; break; }

		case (uint8)(EYapInterruptibleFlags::Conversation):
		{ BrushName = YapBrushes.Icon_Interruptible_Conversation; break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech | EYapInterruptibleFlags::Conversation):
		{ BrushName = YapBrushes.Icon_Interruptible_All; break; }
		
		default: {}
	}
	
	return FYapEditorStyle::GetImageBrush(BrushName);
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapProgressionSettingsWidget::ColorAndOpacity_AutoAdvanceToggle() const
{
	if (!IsAutoAdvanceOverridden())
	{
		return YapColor::DarkGray;
	}
	
	FLinearColor Color = YapColor::White;

	uint8 Current = GetAutoAdvanceValue();

	switch (Current)
	{
		case (uint8)(EYapAutoAdvanceFlags::None):
		{ Color *= YapColor::Orange; break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech):
		{ Color *= YapColor::LightYellow; break; }

		case (uint8)(EYapAutoAdvanceFlags::Conversation):
		{ Color *= YapColor::LightYellow; break; }

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech | EYapAutoAdvanceFlags::Conversation):
		{ Color *= YapColor::LightGreen; break; }
		
		default: {}
	}
	
	return Color;
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapProgressionSettingsWidget::ColorAndOpacity_InterruptibleToggle() const
{
	if (!IsInterruptibleOverridden())
	{
		return YapColor::DarkGray;
	}
	
	FLinearColor Color = YapColor::White;
	
	uint8 Current = GetInterruptibleValue();

	switch (Current)
	{
		case (uint8)(EYapInterruptibleFlags::None):
		{ Color *= YapColor::Orange; break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech):
		{ Color *= YapColor::LightYellow; break; }

		case (uint8)(EYapInterruptibleFlags::Conversation):
		{ Color *= YapColor::LightYellow; break; }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech | EYapInterruptibleFlags::Conversation):
		{ Color *= YapColor::LightGreen; break; }
		
		default: {}
	}
	
	return Color;
}

// ------------------------------------------------------------------------------------------------

uint8 SYapProgressionSettingsWidget::GetAutoAdvanceValue() const
{
	uint8 Current;

	if (AutoAdvanceSettingRaw->IsSet())
	{
		Current = (uint8)(AutoAdvanceSettingRaw->GetValue());
	}
	else
	{
		if (DialogueNode.IsValid())
		{
			if (FragmentIndex != INDEX_NONE && DialogueNode->GetAutoAdvanceFlags().IsSet())
			{
				Current = (uint8)(DialogueNode->GetAutoAdvanceFlags().GetValue());
			}
			else
			{
				Current = (uint8)(DialogueNode->GetNodeConfig().DialoguePlayback.AutoAdvanceFlags);
			}	
		}
		else
		{
			Current = 255;
		}
	}

	return Current;
}

// ------------------------------------------------------------------------------------------------

uint8 SYapProgressionSettingsWidget::GetInterruptibleValue() const
{
	uint8 Current;

	if (InterruptibleSettingRaw->IsSet())
	{
		Current = (uint8)(InterruptibleSettingRaw->GetValue());
	}
	else
	{
		if (DialogueNode.IsValid())
		{
			if (FragmentIndex != INDEX_NONE && DialogueNode->GetInterruptibleFlags().IsSet())
			{
				Current = (uint8)(DialogueNode->GetInterruptibleFlags().GetValue());
			}
			else
			{
				Current = (uint8)(DialogueNode->GetNodeConfig().DialoguePlayback.SpeechInterruptibleFlags);
			}	
		}
		else
		{
			Current = 255;
		}
	}

	return Current;
}

// ------------------------------------------------------------------------------------------------

bool SYapProgressionSettingsWidget::IsInterruptibleOverridden() const
{
	return InterruptibleSettingRaw->IsSet();
}

// ------------------------------------------------------------------------------------------------

bool SYapProgressionSettingsWidget::IsAutoAdvanceOverridden() const
{
	return AutoAdvanceSettingRaw->IsSet();
}

// ------------------------------------------------------------------------------------------------

bool SYapProgressionSettingsWidget::IsCtrlPressed() const
{
	return GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
}

#undef LOCTEXT_NAMESPACE
