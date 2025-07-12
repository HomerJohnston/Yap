// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapHeadingBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSeparator.h"



/**
 * Construct this widget
 *
 * @param	InArgs	The declaration data for this widget
 */
void SYapHeadingBlock::Construct( const FArguments& InArgs )
{
    HeadingText = InArgs._HeadingText;
    
    ChildSlot
    .Padding(8, 16, 16, 8)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .AutoWidth()
        [
            SNew(STextBlock)
            .Text(HeadingText.ToUpper())
            .TextStyle(FAppStyle::Get(), "Menu.Heading")
        ]

        + SHorizontalBox::Slot()
        .Padding(FMargin(14.f, 0.f, 0.f, 0.f))
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Fill)
        [
            SNew(SSeparator)
            .Orientation(Orient_Horizontal)
            .Thickness(1.0f)
            .SeparatorImage(FAppStyle::GetBrush("Menu.Separator"))
        ]
    ];
}
