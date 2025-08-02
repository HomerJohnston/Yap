// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapProgressionSettingsWidget.h"

#include "Yap/Enums/YapAutoAdvanceFlags.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/YapTransactions.h"

#define LOCTEXT_NAMESPACE "YapEditor"

SLATE_IMPLEMENT_WIDGET(SYapProgressionSettingsWidget)
void SYapProgressionSettingsWidget::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer&)
{
	// TODO wtf does anything go in here?
}


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
					.OnClicked(this, &SYapProgressionSettingsWidget::OnClicked_AutoAdvanceToggle)
					.ToolTipText(this, &SYapProgressionSettingsWidget::ToolTipText_AutoAdvanceToggle)
					[
						SNew(SImage)
						.Image(this, &SYapProgressionSettingsWidget::Image_AutoAdvanceToggle)
						.ColorAndOpacity(this, &SYapProgressionSettingsWidget::ColorAndOpacity_AutoAdvanceToggle)
					]
				]
			]
		]
	];
}

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
			Current = (uint8)(DialogueNode->GetNodeConfig().DialoguePlayback.AutoAdvanceFlags);
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

FText SYapProgressionSettingsWidget::ToolTipText_AutoAdvanceToggle() const
{
	uint8 Current = GetAutoAdvanceValue();

	switch (Current)
	{
		case (uint8)(EYapAutoAdvanceFlags::None):
		{ return LOCTEXT("ToolTipText_AutoAdvanceState_None", "Requires manual advance");}

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech):
		{ return LOCTEXT("ToolTipText_AutoAdvanceState_FreeSpeech", "Auto advances free speech only");}

		case (uint8)(EYapAutoAdvanceFlags::Conversation):
		{ return LOCTEXT("ToolTipText_AutoAdvanceState_Conversation", "Auto advances conversations only");}

		case (uint8)(EYapAutoAdvanceFlags::FreeSpeech | EYapAutoAdvanceFlags::Conversation):
		{ return LOCTEXT("ToolTipText_AutoAdvanceState_All", "Auto advances");}
		
		default: {}	
	}

	return INVTEXT("Error");
}

FText SYapProgressionSettingsWidget::ToolTipText_InterruptibleToggle() const
{
	uint8 Current = GetInterruptibleValue();

	switch (Current)
	{
		case (uint8)(EYapInterruptibleFlags::None):
		{ return LOCTEXT("ToolTipText_InterruptibleState_None", "Playback is not interruptible"); }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech):
		{ return LOCTEXT("ToolTipText_InterruptibleState_FreeSpeech", "Playback interruptible in free speech only"); }

		case (uint8)(EYapInterruptibleFlags::Conversation):
		{ return LOCTEXT("ToolTipText_InterruptibleState_Conversation", "Playback interruptible in conversations only"); }

		case (uint8)(EYapInterruptibleFlags::FreeSpeech | EYapInterruptibleFlags::Conversation):
		{ return LOCTEXT("ToolTipText_InterruptibleState_All", "Playback is interruptible"); }
		
		default: {}
	}
	
	return INVTEXT("Error");
}

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

uint8 SYapProgressionSettingsWidget::GetAutoAdvanceValue() const
{
	uint8 Current;

	if (AutoAdvanceSettingRaw->IsSet())
	{
		Current = (uint8)(AutoAdvanceSettingRaw->GetValue());
	}
	else
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

	return Current;
}

uint8 SYapProgressionSettingsWidget::GetInterruptibleValue() const
{
	uint8 Current;

	if (InterruptibleSettingRaw->IsSet())
	{
		Current = (uint8)(InterruptibleSettingRaw->GetValue());
	}
	else
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

	return Current;
}

bool SYapProgressionSettingsWidget::IsInterruptibleOverridden() const
{
	return InterruptibleSettingRaw->IsSet();
}

bool SYapProgressionSettingsWidget::IsAutoAdvanceOverridden() const
{
	return AutoAdvanceSettingRaw->IsSet();
}

bool SYapProgressionSettingsWidget::IsCtrlPressed() const
{
	return GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
}

#undef LOCTEXT_NAMESPACE
