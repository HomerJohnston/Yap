// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapEditor/SlateWidgets/SYapTimeProgressionWidget.h"

#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"

void SYapTimeProgressionWidget::Construct(const FArguments& InArgs)
{
	BarColor = InArgs._BarColor;
	
	SpeechTime = InArgs._SpeechTime;
	PaddingTime = InArgs._PaddingTime;
	MaxDisplayTime = InArgs._MaxDisplayTime;

	PlaybackTime = InArgs._PlaybackTime;
	
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(TAttribute<float>::CreateSP(this, &SYapTimeProgressionWidget::GetFirstBarPercentage))
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
			.ColorAndOpacity(BarColor)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(TAttribute<float>::CreateSP(this, &SYapTimeProgressionWidget::GetSecondBarPercentage))
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Bar_NegativePadding))
			.ColorAndOpacity(BarColor)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(TAttribute<float>::CreateSP(this, &SYapTimeProgressionWidget::GetThirdBarPercentage))
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Bar_PositivePadding))
			.ColorAndOpacity(BarColor)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(TAttribute<float>::CreateSP(this, &SYapTimeProgressionWidget::GetFourthBarPercentage))
		[
			SNew(SSpacer)
		]
	];
}

float SYapTimeProgressionWidget::GetFirstBarPercentage() const
{
	/*
	if (!MaxDisplayTime.IsBound() || !SpeechTime.IsBound() || !SpeechTime.Get().IsSet() || !PaddingTime.IsBound())
	{
		return 0.0f;
	}
	*/

	float SpeechTimeActual = SpeechTime.Get().GetValue();
	
	float PaddingShift = FMath::Min(0.0f, PaddingTime.Get(0.0f));
	
	float Val = SpeechTimeActual + PaddingShift;

	return Val / MaxDisplayTime.Get(1.0f);
}

float SYapTimeProgressionWidget::GetSecondBarPercentage() const
{
	if (PaddingTime.Get() >= 0.0f)
	{
		return 0.0f;
	}
	
	return FMath::Abs(PaddingTime.Get(0.0f)) / MaxDisplayTime.Get(1.0f);
}

float SYapTimeProgressionWidget::GetThirdBarPercentage() const
{
	if (PaddingTime.Get() < 0.0f)
	{
		return 0.0f;
	}
	
	return PaddingTime.Get(0.0f) / MaxDisplayTime.Get(1.0f);
}

float SYapTimeProgressionWidget::GetFourthBarPercentage() const
{
	return 1.0 - (GetFirstBarPercentage() + GetSecondBarPercentage() + GetThirdBarPercentage());
}

bool SYapTimeProgressionWidget::GetPaddingPositivity() const
{
	return true;
}

int32 SYapTimeProgressionWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 RetLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (PlaybackTime.Get().IsSet())
	{
		float PlaybackTimeValue = PlaybackTime.Get().GetValue();

		float Normalized = PlaybackTimeValue / MaxDisplayTime.Get(1.0f);
		
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			RetLayerId,
			AllottedGeometry.ToPaintGeometry(FVector2D(3, 5), FSlateLayoutTransform(FVector2D(AllottedGeometry.GetLocalSize().X * Normalized, -1))),
			FYapEditorStyle::GetImageBrush(YapBrushes.Icon_PlaybackTimeHandle
		));

		RetLayerId++;	
	}
	
	return RetLayerId;
}
