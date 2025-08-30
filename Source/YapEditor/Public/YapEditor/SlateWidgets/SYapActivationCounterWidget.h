// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "SYapTextPropertyEditableTextBox.h"
#include "Widgets/SCompoundWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

class SEditableText;
/**
 * 
 */
class SYapActivationCounterWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS( SYapActivationCounterWidget )
		: _FontHeight(8) {}

	SLATE_ATTRIBUTE(int32, ActivationCount)
	SLATE_ATTRIBUTE(int32, ActivationLimit)
	SLATE_ARGUMENT(int32, FontHeight)
	
	SLATE_END_ARGS()

	TAttribute<int32> ActivationCount;
	TAttribute<int32> ActivationLimit;
	int32 FontHeight;
	
	FText NumeratorText() const;
	FText DenominatorText() const;

	FSlateColor NumeratorColor() const;
	FSlateColor DenominatorColor() const;

	TSharedPtr<SEditableText> Denominator;

	virtual void Construct(const FArguments& InArgs, FOnTextCommitted OnTextCommitted);

	EVisibility Visibility_UpperElements() const;
};

#undef LOCTEXT_NAMESPACE