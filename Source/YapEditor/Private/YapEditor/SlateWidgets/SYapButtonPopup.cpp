// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapButtonPopup.h"

#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void SYapButtonPopup::Construct(const FArguments& InArgs)
{
	ButtonForegroundColor = InArgs._ButtonForegroundColor;

	ButtonBackgroundColor = InArgs._ButtonBackgroundColor;
	
	PopupContentGetter = InArgs._PopupContentGetter;

	OnClicked = InArgs._OnClicked;

	FocusWidget = InArgs._FocusWidget;
	
	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.Placement(InArgs._PopupPlacement)
	.Method(EPopupMethod::CreateNewWindow)
	.IsCollapsedByParent(true)
	.OnMenuOpenChanged(InArgs._OnPopupOpenChanged)
	//.ShouldDeferPaintingAfterWindowContent(false)
	[
		SAssignNew(Button, SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle((InArgs._ButtonStyle) ? InArgs._ButtonStyle : &FYapEditorStyle::Get().GetWidgetStyle<FButtonStyle>(YapStyles.ButtonStyle_TimeSettingOpener))
		.OnClicked(this, &SYapButtonPopup::OnClicked_Button)
		.ForegroundColor(this, &SYapButtonPopup::ForegroundColor_Button)
		.ButtonColorAndOpacity(this, &SYapButtonPopup::BackgroundColor_Button)
		.ContentPadding(InArgs._ButtonContentPadding)
		[
			InArgs._ButtonContent.Widget
		]
	]);
}

FReply SYapButtonPopup::OnClicked_Button()
{
	FReply ButtonReply = FReply::Handled();

	if (OnClicked.IsBound())
	{
		FReply OnClickedReply = OnClicked.Execute();

		if (OnClickedReply.IsEventHandled())
		{
			return ButtonReply;
		}
	}

	if (!PopupContentGetter.IsBound())
	{
		return ButtonReply;
	}

	TSharedRef<SWidget> PopupContent = PopupContentGetter.Execute();
	
	SetMenuContent(PopupContent);

	bool bShouldOpen = ShouldOpenDueToClick();
	bool bIs = IsOpen();
	
	SetIsOpen(bShouldOpen, true);
	
	if (IsOpen())
	{
		(void)OnOpened.ExecuteIfBound();

		FSlateApplication::Get().SetKeyboardFocus(PopupContent);
	}

	/*
	if (!RunPostPopupDelegate())
	{
		ButtonReply.SetUserFocus(MenuContent.ToSharedRef(), EFocusCause::SetDirectly);
	}
	*/
	
	return ButtonReply;
}

void SYapButtonPopup::SetMenuContent(TSharedRef<SWidget> InMenuContent)
{
	WrappedContent = MenuContent =
		SNew(SBox)
		.Padding(8)
		[
			InMenuContent
		];
}

FSlateColor SYapButtonPopup::ForegroundColor_Button() const
{
	FLinearColor Col = ButtonForegroundColor.Get();

	if (!Button->IsHovered())
	{
		Col *= YapColor::LightGray;
	}
	else if (Button->IsPressed())
	{
		Col *= YapColor::LightGray;
	}

	if (IsOpen())
	{
		Col *= 2.0;
	}
	
	return Col;
}

FSlateColor SYapButtonPopup::BackgroundColor_Button() const
{
	FLinearColor Col = ButtonBackgroundColor.Get();

	if (!Button->IsHovered())
	{
		Col *= YapColor::DimWhite;
	}
	else if (Button->IsPressed())
	{
		Col *= YapColor::DimWhite;
	}

	if (IsOpen())
	{
		Col *= 2.0;
	}
	
	return Col;
}

void SYapButtonPopup::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SMenuAnchor::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

#undef LOCTEXT_NAMESPACE
