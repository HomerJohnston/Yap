// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Yap/Enums/YapInterruptibleFlags.h"
#include "Widgets/SCompoundWidget.h"

class UFlowNode_YapDialogue;
enum class EYapAutoAdvanceFlags : uint8;

class SYapProgressionSettingsWidget : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET_API( SYapProgressionSettingsWidget, SCompoundWidget, YAPEDITOR_API)

public:
    SLATE_USER_ARGS(SYapProgressionSettingsWidget)
    {}
        SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNode)
        SLATE_ARGUMENT(int32, FragmentIndex)
        SLATE_ARGUMENT(TOptional<EYapInterruptibleFlags>*, SkippableSettingRaw)
        SLATE_ARGUMENT(TOptional<EYapAutoAdvanceFlags>*, AutoAdvanceSettingRaw)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    UFlowNode_YapDialogue* DialogueNode = nullptr;
    int32 FragmentIndex = INDEX_NONE;
    TOptional<EYapInterruptibleFlags>* InterruptibleSettingRaw = nullptr;
    TOptional<EYapAutoAdvanceFlags>* AutoAdvanceSettingRaw = nullptr;

protected:
    FReply OnClicked_AutoAdvanceToggle() const;

    FReply OnClicked_InterruptibleToggle() const;
    
    FText ToolTipText_AutoAdvanceToggle() const;

    FText ToolTipText_InterruptibleToggle() const;
    
    const FSlateBrush* Image_AutoAdvanceToggle() const;

    const FSlateBrush* Image_InterruptibleToggle() const;
    
    FSlateColor ColorAndOpacity_AutoAdvanceToggle() const;

    FSlateColor ColorAndOpacity_InterruptibleToggle() const;

    uint8 GetAutoAdvanceValue() const;
    
    uint8 GetInterruptibleValue() const;

    bool IsInterruptibleOverridden() const;

    bool IsAutoAdvanceOverridden() const;
    
    bool IsCtrlPressed() const;
};
