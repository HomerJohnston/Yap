// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapEditor/SlateWidgets/SYapTimeProgressionWidget.h"

#include "Yap/YapFragment.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"

SLATE_IMPLEMENT_WIDGET(SYapTimeProgressionWidget)

#define LOCTEXT_NAMESPACE "YapEditor"

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
	DialogueNode = InArgs._DialogueNode;
	FragmentIndex = InArgs._FragmentIndex;
	BarColorAtt = InArgs._BarColor;
	PaddingIsSetAtt = InArgs._PaddingIsSet;
	
	SpeechTimeAtt = InArgs._SpeechTime;
	PaddingTimeAtt = InArgs._PaddingTime;
	//MaxDisplayTimeAtt = InArgs._MaxDisplayTime;
	MaxDisplayTimeAtt.Assign(*this, InArgs._MaxDisplayTime);

	PlaybackTimeAtt = InArgs._PlaybackTime;

	ChildSlot
	.Padding(0, -64, 0, 6)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Top)
	[
		SAssignNew(TimeDisplayBox, SOverlay)
	];
}

FText SYapTimeProgressionWidget::GetTimeText() const
{
	float SpeechDuration = SpeechTimeAtt.Get().Get(0.0f);
	float PaddingDuration = PaddingTimeAtt.Get();

	float PaddingEndTime = SpeechDuration + PaddingDuration;

	static FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MaximumFractionalDigits = 2;
	FormattingOptions.MinimumFractionalDigits = 2;

	static FText Plus(LOCTEXT("TimeProgressionPositivePaddingSign", "+"));
	static FText Minus(LOCTEXT("TimeProgressionNegativePaddingSign", "-"));
	
	return FText::Format(LOCTEXT("TimeProgressionText", "Speech: {0} s\n{1} {2} s"), FText::AsNumber(SpeechDuration, &FormattingOptions), PaddingDuration > 0 ? Plus : Minus, FText::AsNumber(FMath::Abs(PaddingDuration), &FormattingOptions));
}

void DrawBar(float Left, float Right, float MaxTime, float DownScaling, float GeoWidth, FSlateWindowElementList& OutDrawElements, int32& RetLayerId, const FGeometry& AllottedGeometry, FName BrushName, FLinearColor Color)
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
		AllottedGeometry.ToPaintGeometry(FVector2D(Width, 5), FSlateLayoutTransform(FVector2D(LeftPos, 1))),
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

		DrawBar(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Bar_NormalPadding, BarColor);
	}

	if (BarTimes[1].SizeSquared() > 0.0f)
	{
		float Left = BarTimes[1].X;
		float Right = BarTimes[1].Y;

		DrawBar(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Bar_NegativePadding, PaddingColor);
	}
	
	if (BarTimes[2].SizeSquared() > 0.0f)
	{
		float Left = BarTimes[2].X;
		float Right = BarTimes[2].Y;

		DrawBar(Left, Right, MaxTime, DownScaling, GeoWidth, OutDrawElements, RetLayerId, AllottedGeometry, YapBrushes.Bar_PositivePadding, PaddingColor);
	}
	
	if (PlaybackTimeAtt.Get().IsSet())
	{
		float TimeValue = PlaybackTimeAtt.Get().GetValue();

		float Normalized = DownScaling * TimeValue / MaxTime;
		
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			RetLayerId++,
			AllottedGeometry.ToPaintGeometry(FVector2D(3.0f, 5.0f), FSlateLayoutTransform(FVector2D((GeoWidth - 3.0f) * Normalized, -1.0f))),
			FYapEditorStyle::GetImageBrush(YapBrushes.Icon_PlaybackTimeHandle)
		);
	}

	if (AllowPaddingEditing())
	{
		if (!GEditor->IsPlaySessionInProgress() && SpeechTimeAtt.IsSet())
		{
			float Normalized = DownScaling * (PaddingEndTime / MaxTime);

			FLinearColor HandleColor = (PaddingIsSetAtt.Get() ? YapColor::LightGray : BarColor);

			FVector2D Size = FVector2D(7.0f, 7.0f);
			FVector2D Trans = FVector2D((GeoWidth - 5.0f) * Normalized, 0.0f);
		
			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(Trans)),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FilledCircle),
				ESlateDrawEffect::None,
				HandleColor
			);
		}
	}
	
	return RetLayerId;
}

bool SYapTimeProgressionWidget::IsValid() const
{
	return SpeechTimeAtt.IsBound() && SpeechTimeAtt.Get().IsSet() && PaddingTimeAtt.IsBound();
}

FReply SYapTimeProgressionWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!AllowPaddingEditing())
	{
		return FReply::Unhandled();
	}
	
	if ((MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) && DialogueNode.IsValid() && GetCursorOnHandle())// && !IsLocked())
	{
		bIsAdjusting = true;
		
		(void)OnMouseCaptureBegin.ExecuteIfBound();

		FYapTransactions::BeginModify(INVTEXT("Test"), DialogueNode.Get());

		SetCursor(EMouseCursor::None);

		ShowTimeDisplayBox();

		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	return FReply::Unhandled();
}

FReply SYapTimeProgressionWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if ((MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) && HasMouseCaptureByUser(MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex()))
	{
		bIsAdjusting = false;
		
		SetCursor(NullOpt);

		FModifierKeysState Temp;
		
		FPointerEvent FakePointerEvent(
			FSlateApplication::Get().GetUserIndexForMouse(),
			GetHandlePos() + FVector2d(1, -1),
			FSlateApplication::Get().GetCursorPos(),
			FVector2D(0, 10),
			{ EKeys::Invalid },
			Temp);
		
		FSlateApplication::Get().ProcessMouseMoveEvent(FakePointerEvent);
		
		FYapTransactions::EndModify();

		HideTimeDisplayBox();
		
		FSlateApplication::Get().SetCursorPos(GetHandlePos() + FVector2d(1, -1));
		
		return FReply::Handled().ReleaseMouseCapture();	
	}

	return FReply::Unhandled();
}

void SYapTimeProgressionWidget::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SCompoundWidget::OnFocusLost(InFocusEvent);
}

FReply SYapTimeProgressionWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCaptureByUser(MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex()) && DialogueNode.IsValid()) // && !IsLocked())
	{
		float Delta = 0.005f * MouseEvent.GetCursorDelta().X;

		FYapFragment& Fragment = DialogueNode.Get()->GetFragmentMutableByIndex(FragmentIndex);

		float PaddingValue = Fragment.GetPaddingValue(DialogueNode.Get()->GetWorld(), DialogueNode.Get()->GetNodeConfig());
		
		Fragment.SetPaddingToNextFragment(PaddingValue + Delta);
		
		return FReply::Handled();
	}
	return SCompoundWidget::OnMouseMove(MyGeometry, MouseEvent);
}

float SYapTimeProgressionWidget::PositionToValue(const FGeometry& MyGeometry, const UE::Slate::FDeprecateVector2DParameter& AbsolutePosition)
{	
	const FVector2f LocalPosition = MyGeometry.AbsoluteToLocal(AbsolutePosition);

	float RelativeValue;
	float Denominator;
	// Only need X as we rotate the thumb image when rendering vertically
	const float Indentation = 2;//GetThumbImage()->ImageSize.X * (IndentHandleSlateAttribute.Get() ? 2.f : 1.f);
	const float HalfIndentation = 0.5f * Indentation;

	Denominator = MyGeometry.Size.X - Indentation;
	RelativeValue = (Denominator != 0.f) ? (LocalPosition.X - HalfIndentation) / Denominator : 0.f;

	RelativeValue = FMath::Clamp(RelativeValue, 0.0f, 1.0f) * (MaxValue - MinValue) + MinValue;
	
	return RelativeValue;
}

TOptional<EMouseCursor::Type> SYapTimeProgressionWidget::GetCursor() const
{
	if (HasMouseCapture())
	{
		return EMouseCursor::None;
	}

	if (!IsHovered())
	{
		return NullOpt;
	}
	
	if (GetCursorOnHandle() && AllowPaddingEditing())
	{
		return EMouseCursor::ResizeLeftRight;
	}
	
	return SCompoundWidget::GetCursor();
}

bool SYapTimeProgressionWidget::GetCursorOnHandle() const
{
	if (!IsHovered())
	{
		return false;
	}
	
	float SpeechDuration = SpeechTimeAtt.Get().Get(0.0f);
	float PaddingDuration = PaddingTimeAtt.Get();
	float MaxTime = MaxDisplayTimeAtt.Get();

	float ActualSpeechEndTime = SpeechDuration;
	float PaddingEndTime = SpeechDuration + PaddingDuration;
	
	float LargestTimeValue = FMath::Max(ActualSpeechEndTime, PaddingEndTime);

	float DownScaling = 1.0f;
	
	if (LargestTimeValue > MaxTime)
	{
		DownScaling = MaxTime / LargestTimeValue;
	}

	if (!GEditor->IsPlaySessionInProgress() && SpeechTimeAtt.IsSet())
	{
		float Normalized = DownScaling * (PaddingEndTime / MaxTime);

		FVector2f Pos = GetTickSpaceGeometry().AbsolutePosition;
		FVector2f Size = GetTickSpaceGeometry().Size;

		TSharedPtr<SYapTimeProgressionWidget> AsShared = SharedThis(const_cast<SYapTimeProgressionWidget*>(this));

		TSharedPtr<SWindow> Window = FSlateApplication::Get().FindBestParentWindowForDialogs(AsShared);//  FSlateApplication::Get().GetActiveTopLevelWindow();

		if (Window.IsValid())
		{
			float xPos = Pos.X + (Size.X * Normalized);
			float yPos = Pos.Y + Size.Y;

			if (FVector2D::Distance(FSlateApplication::Get().GetCursorPos(), FVector2D(xPos, yPos)) < 6)
			{
				return true;
			}
		}
	}

	return false;
}

FVector2D SYapTimeProgressionWidget::GetHandlePos() const
{
	float SpeechDuration = SpeechTimeAtt.Get().Get(0.0f);
	float PaddingDuration = PaddingTimeAtt.Get();
	float MaxTime = MaxDisplayTimeAtt.Get();

	float ActualSpeechEndTime = SpeechDuration;
	float PaddingEndTime = SpeechDuration + PaddingDuration;
	
	float LargestTimeValue = FMath::Max(ActualSpeechEndTime, PaddingEndTime);

	float DownScaling = 1.0f;
	
	if (LargestTimeValue > MaxTime)
	{
		DownScaling = MaxTime / LargestTimeValue;
	}

	if (!GEditor->IsPlaySessionInProgress() && SpeechTimeAtt.IsSet())
	{
		float Normalized = DownScaling * (PaddingEndTime / MaxTime);

		FVector2f Pos = GetTickSpaceGeometry().AbsolutePosition;
		FVector2f Size = GetTickSpaceGeometry().Size;

		TSharedPtr<SYapTimeProgressionWidget> AsShared = SharedThis(const_cast<SYapTimeProgressionWidget*>(this));

		TSharedPtr<SWindow> Window = FSlateApplication::Get().FindBestParentWindowForDialogs(AsShared);

		if (Window.IsValid())
		{
			float xPos = Pos.X + (Size.X * Normalized);
			float yPos = Pos.Y + Size.Y;

			return FVector2D(xPos, yPos);
		}
	}

	return FSlateApplication::Get().GetCursorPos();
}

void SYapTimeProgressionWidget::ShowTimeDisplayBox()
{
	TimeDisplayBox->AddSlot()
	[
		SNew(SBorder)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Rounded))
		.BorderBackgroundColor(YapColor::DarkGray)
		.Padding(12)
		[
			SNew(SBox)
			[
				SNew(STextBlock)
				.Text(this, &SYapTimeProgressionWidget::GetTimeText)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(YapColor::DimWhite)
			]
		]
	];
}

void SYapTimeProgressionWidget::HideTimeDisplayBox()
{
	TimeDisplayBox->ClearChildren();
}

bool SYapTimeProgressionWidget::AllowPaddingEditing() const
{
	return DialogueNode->GetNodeType() != EYapDialogueNodeType::TalkAndAdvance;
}

#undef LOCTEXT_NAMESPACE
