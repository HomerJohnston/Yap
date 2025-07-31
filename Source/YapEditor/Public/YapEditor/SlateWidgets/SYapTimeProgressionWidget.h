// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Widgets/SCompoundWidget.h"

class UFlowNode_YapDialogue;
struct FYapFragment;

#define LOCTEXT_NAMESPACE "YapEditor"

/**
 * 
 */
class SYapTimeProgressionWidget : public SCompoundWidget//, public TSharedFromThis<SYapTimeProgressionWidget>
{
	
	SLATE_DECLARE_WIDGET_API(SYapTimeProgressionWidget, SCompoundWidget, YAPEDITOR_API)
	
public:

	SLATE_USER_ARGS(SYapTimeProgressionWidget)
		: _MaxDisplayTime(0.0)
	{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNode)
		SLATE_ARGUMENT(int32, FragmentIndex)
	
		SLATE_ATTRIBUTE(FLinearColor, BarColor)
		SLATE_ATTRIBUTE(bool, PaddingIsSet)
		SLATE_ATTRIBUTE(TOptional<float>, SpeechTime)
		SLATE_ATTRIBUTE(float, PaddingTime)
		SLATE_ATTRIBUTE(float, MaxDisplayTime)

		SLATE_ATTRIBUTE(TOptional<float>, PlaybackTime)
	
		/** Invoked when the mouse is pressed and a capture begins. */
		SLATE_EVENT(FSimpleDelegate, OnMouseCaptureBegin)

		/** Invoked when the mouse is released and a capture ends. */
		SLATE_EVENT(FSimpleDelegate, OnMouseCaptureEnd)
	
	SLATE_END_ARGS()

public:
	SYapTimeProgressionWidget();

	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueNode;
	int32 FragmentIndex = INDEX_NONE;
	
	TAttribute<FLinearColor> BarColorAtt;
	TAttribute<bool> PaddingIsSetAtt;
	TAttribute<TOptional<float>> SpeechTimeAtt;
	TAttribute<float> PaddingTimeAtt;
	TSlateAttribute<float> MaxDisplayTimeAtt;
	
	float MinValue;
	float MaxValue;
	
	TAttribute<TOptional<float>> PlaybackTimeAtt;

	bool bIsAdjusting = false;
	bool bTimeDisplayBoxActive = false;
	
	TSharedPtr<SOverlay> TimeDisplayBox;
	
	virtual void Construct(const FArguments& InArgs);

	FText GetTimeText() const;

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void DrawBar(float Left, float Right, float MaxTime, float DownScaling, float GeoWidth, FSlateWindowElementList& OutDrawElements, int32& RetLayerId, const FGeometry& AllottedGeometry, FName BrushName, FLinearColor Color) const;

	bool IsValid() const;

	/** Get the MinValue attribute */
	float GetMinValue() const { return MinValue; }

	/** Get the MaxValue attribute */
	float GetMaxValue() const { return MaxValue; }
	
	// Holds a delegate that is executed when the mouse is pressed and a capture begins.
	FSimpleDelegate OnMouseCaptureBegin;

	// Holds a delegate that is executed when the mouse is let up and a capture ends.
	FSimpleDelegate OnMouseCaptureEnd;

	// Holds a delegate that is executed when the slider's value changed.
	FOnFloatValueChanged OnValueChanged;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void OnFocusLost(const FFocusEvent& InFocusEvent) override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	float PositionToValue(const FGeometry& MyGeometry, const FSlateImageBrush* HandleImage, const UE::Slate::FDeprecateVector2DParameter& AbsolutePosition);

	TOptional<EMouseCursor::Type> GetCursor() const override;

	bool IsCursorOnHandle() const;
	
	FVector2D GetHandlePos() const;

	void ShowTimeDisplayBox();

	void HideTimeDisplayBox();

	bool AllowPaddingEditing() const;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	float GetPaddingHandleSize() const;
	
	FVector2D GetPlaybackHandleSize() const;
	
	const FSlateBrush* GetPaddingHandleImage() const;
	
	const FSlateBrush* GetPlaybackHandleImage() const;
};



#undef LOCTEXT_NAMESPACE
