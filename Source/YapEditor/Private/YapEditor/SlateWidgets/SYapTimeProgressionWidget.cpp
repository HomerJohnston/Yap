// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapEditor/SlateWidgets/SYapTimeProgressionWidget.h"

#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"

SLATE_IMPLEMENT_WIDGET(SYapTimeProgressionWidget)

void SYapTimeProgressionWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Wtf", MaxDisplayTimeAtt, EInvalidateWidgetReason::Paint);
}

SYapTimeProgressionWidget::SYapTimeProgressionWidget()
	: MaxDisplayTimeAtt(*this, 0.0f)
{
}

void SYapTimeProgressionWidget::Construct(const FArguments& InArgs)
{
	BarColorAtt = InArgs._BarColor;
	PaddingIsSetAtt = InArgs._PaddingIsSet;
	
	SpeechTimeAtt = InArgs._SpeechTime;
	PaddingTimeAtt = InArgs._PaddingTime;
	//MaxDisplayTimeAtt = InArgs._MaxDisplayTime;
	MaxDisplayTimeAtt.Assign(*this, InArgs._MaxDisplayTime);

	PlaybackTimeAtt = InArgs._PlaybackTime;
}

void Draw(float Left, float Right, float MaxTime, float DownScaling, float GeoWidth, FSlateWindowElementList& OutDrawElements, int32& RetLayerId, const FGeometry& AllottedGeometry, FName BrushName, FLinearColor Color)
{
	Left = Left / MaxTime;
	Left = DownScaling * Left;

	Right = Right / MaxTime;
	Right = DownScaling * Right;

	float LeftPos = Left * GeoWidth;
	float Width = (Right - Left) * GeoWidth;
		
	FSlateDrawElement::MakeBox
	(
		OutDrawElements,
		RetLayerId++,
		AllottedGeometry.ToPaintGeometry(FVector2D(Width, 3), FSlateLayoutTransform(FVector2D(LeftPos, 0))),
		FYapEditorStyle::GetImageBrush(BrushName),
		ESlateDrawEffect::None,
		Color
	);
}

int32 SYapTimeProgressionWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 RetLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (!this->IsValid())
	{
		return RetLayerId;
	}

	FLinearColor BarColor = BarColorAtt.Get();
	FLinearColor PaddingColor = PaddingIsSetAtt.Get() ? YapColor::White : BarColor;

	float SpeechDuration = SpeechTimeAtt.Get().Get(0.0f);
	float PaddingDuration = PaddingTimeAtt.Get();
	float MaxTime = MaxDisplayTimeAtt.Get();

	float NegativePaddingShift = FMath::Min(0.0f, PaddingDuration);
	float AffectedSpeechDuration = SpeechDuration + NegativePaddingShift;

	float NegativePaddedSpeechEndTime = AffectedSpeechDuration;
	float ActualSpeechEndTime = SpeechDuration;
	float PaddingEndTime = SpeechDuration + PaddingDuration;
	
	float LargestTimeValue = FMath::Max(ActualSpeechEndTime, PaddingEndTime);

	float DownScaling = 1.0f;
	
	if (LargestTimeValue > MaxTime)
	{
		DownScaling = MaxTime / LargestTimeValue;
	}
	
	TArray<FVector2D> BarTimes
	{
		FVector2D(0.0f, NegativePaddedSpeechEndTime), // Normal speech time bar
		FVector2D(NegativePaddedSpeechEndTime, ActualSpeechEndTime), // If padding time is negative, this will be the negative padded time before speech ends
		FVector2D(ActualSpeechEndTime, PaddingEndTime),	// If padding time is positive, this will be the positive padding time
	};

	float GeoWidth = AllottedGeometry.GetLocalSize().X;

	if (BarTimes[0].SizeSquared() > 0.0f)
	{
		float Left = BarTimes[0].X;
		float Right = BarTimes[0].Y;

		Draw(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Box_SolidWhite, BarColor);
	}

	if (BarTimes[1].SizeSquared() > 0.0f)
	{
		float Left = BarTimes[1].X;
		float Right = BarTimes[1].Y;

		Draw(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Bar_NegativePadding, PaddingColor);
	}
	
	if (BarTimes[2].SizeSquared() > 0.0f)
	{
		float Left = BarTimes[2].X;
		float Right = BarTimes[2].Y;

		Draw(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Bar_PositivePadding, PaddingColor);
	}
	
	if (PlaybackTimeAtt.Get().IsSet())
	{
		float PlaybackTimeValue = PlaybackTimeAtt.Get().GetValue();

		float Normalized = DownScaling * PlaybackTimeValue / MaxTime;
		
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(FVector2D(3.0f, 5.0f), FSlateLayoutTransform(FVector2D((GeoWidth - 3.0f) * Normalized, -1.0f))),
			FYapEditorStyle::GetImageBrush(YapBrushes.Icon_PlaybackTimeHandle
		));
	}
	
	return RetLayerId;
}

bool SYapTimeProgressionWidget::IsValid() const
{
	return /*MaxDisplayTimeAtt.IsBound() &&*/ SpeechTimeAtt.IsBound() && SpeechTimeAtt.Get().IsSet() && PaddingTimeAtt.IsBound();
}
