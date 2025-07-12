// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Framework/MultiBox/MultiBox.h"

/**
 * Heading MultiBlock widget
 */
class SYapHeadingBlock : public SCompoundWidget
{

public:
    SLATE_BEGIN_ARGS( SYapHeadingBlock ){}
        SLATE_ARGUMENT(FText, HeadingText)
    SLATE_END_ARGS()

    /** Text for this heading */
    FText HeadingText;

    /** Construct this widget */
    YAPEDITOR_API void Construct( const FArguments& InArgs );
};
