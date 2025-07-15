// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

class SYapProgressionSettingsWidget : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET_API( SYapProgressionSettingsWidget, SCompoundWidget, YAPEDITOR_API)

public:
    SLATE_USER_ARGS(SYapProgressionSettingsWidget)
    {}
        SLATE_ARGUMENT(TOptional<bool>*, SkippableSettingRaw)
        SLATE_ARGUMENT(TAttribute<bool>, SkippableEvaluatedAttr)
        SLATE_ARGUMENT(TOptional<bool>*, AutoAdvanceSettingRaw)
        SLATE_ARGUMENT(TAttribute<bool>, AutoAdvanceEvaluatedAttr)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    TOptional<bool>* SkippableSettingRaw = nullptr;
    TAttribute<bool> SkippableEvaluatedAttr;
    TOptional<bool>* AutoAdvanceSettingRaw = nullptr;
    TAttribute<bool> AutoAdvanceEvaluatedAttr;
};