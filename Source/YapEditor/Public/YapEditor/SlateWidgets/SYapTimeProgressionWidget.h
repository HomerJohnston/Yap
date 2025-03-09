// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Widgets/SCompoundWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

/**
 * 
 */
class SYapTimeProgressionWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SYapTimeProgressionWidget)
		{}

		SLATE_ATTRIBUTE(FLinearColor, BarColor)
		SLATE_ATTRIBUTE(TOptional<float>, SpeechTime)
		SLATE_ATTRIBUTE(float, PaddingTime)
		SLATE_ATTRIBUTE(float, MaxDisplayTime)

		SLATE_ATTRIBUTE(TOptional<float>, PlaybackTime)
	
	SLATE_END_ARGS()

	TAttribute<FLinearColor> BarColorAtt;
	TAttribute<TOptional<float>> SpeechTimeAtt;
	TAttribute<float> PaddingTimeAtt;
	TAttribute<float> MaxDisplayTimeAtt;
	
	TAttribute<TOptional<float>> PlaybackTimeAtt;

	virtual void Construct(const FArguments& InArgs);

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	bool IsValid() const;
};


#undef LOCTEXT_NAMESPACE
