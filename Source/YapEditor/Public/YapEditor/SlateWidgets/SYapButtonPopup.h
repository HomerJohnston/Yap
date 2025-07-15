// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

#include "YapEditor/YapEditorColor.h"

DECLARE_DELEGATE(FOnOpened)
DECLARE_DELEGATE_RetVal(TSharedRef<SWidget>, FPopupContentGetter)
DECLARE_DELEGATE_OneParam(FPostPopupDelegate, bool&)

class SYapButtonPopup : public SMenuAnchor
{
public:
	SLATE_BEGIN_ARGS(SYapButtonPopup)
	// TODO make a totally blank SYapButtonPopup safe to spawn - set up safe defaults & code
		: _ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("HoverHintOnly"))
		, _ButtonBackgroundColor(YapColor::Gray)
		, _ButtonForegroundColor(YapColor::White)
		, _ButtonContentPadding(FMargin(0))
		, _HAlign(HAlign_Center)
		, _VAlign(VAlign_Center)
	{}
		/** You must return ***UNHANDLED*** to let the button popup appear; if you return Handled, it assumes the click was already used up */
		SLATE_EVENT( FOnClicked, OnClicked ) 

		SLATE_EVENT( FOnIsOpenChanged, OnPopupOpenChanged )

		/** Pass in a delegate that builds the popup, e.g. (FPopupContentGetter::Create...); this lets us avoid building this whole popup widget until we actually need it */
		SLATE_ARGUMENT( FPopupContentGetter, PopupContentGetter ) // TODO is there a way for me to take in a delegate arg the same way that a SLATE_EVENT allows?

		SLATE_ARGUMENT( EMenuPlacement, PopupPlacement )

		SLATE_STYLE_ARGUMENT( FButtonStyle, ButtonStyle )

		SLATE_ATTRIBUTE( FLinearColor, ButtonBackgroundColor )		

		SLATE_ATTRIBUTE( FLinearColor, ButtonForegroundColor )		

		SLATE_ATTRIBUTE( FMargin, ButtonContentPadding )

		SLATE_ARGUMENT( EHorizontalAlignment, HAlign )
		
		SLATE_ARGUMENT( EVerticalAlignment, VAlign )
		
		SLATE_NAMED_SLOT( FArguments, ButtonContent )

		SLATE_ARGUMENT( TSharedPtr<SWidget>, FocusWidget )

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnClicked_Button();

	/** Function to run after the popup is constructed. Return value indicates whether or not to leave default focus of popup, return true if the delegate intends to cause a custom widget focus. */
	bool RunPostPopupDelegate();

	FOnOpened OnOpened;
	
	TWeakPtr<SWidget> ContentWidgetPtr;

	TAttribute<FLinearColor> ButtonForegroundColor;

	TAttribute<FLinearColor> ButtonBackgroundColor;

	SHorizontalBox::FSlot* ButtonContentSlot;

	TSharedPtr<SButton> Button;

	FOnClicked OnClicked;

	TSharedPtr<SWidget> FocusWidget;
	
	FPopupContentGetter PopupContentGetter;
	
	virtual void SetMenuContent(TSharedRef<SWidget> InMenuContent) override;

	FSlateColor ForegroundColor_Button() const;

	FSlateColor BackgroundColor_Button() const;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};

#undef LOCTEXT_NAMESPACE
