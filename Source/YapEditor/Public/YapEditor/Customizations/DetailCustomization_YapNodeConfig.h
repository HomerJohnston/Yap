// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IDetailCustomization.h"

class UYapProjectSettings;
class SGameplayTagPicker;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapNodeConfig : public IDetailCustomization
{
    //FSlateColor ForegroundColor_Button(FLinearColor Col, TSharedPtr<SButton> Button) const { return YapColor::Error; };


    FSlateFontInfo DetailFont;
	
public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FDetailCustomization_YapNodeConfig());
    }

protected:
    FText GetMoodTags() const;

    const FSlateBrush* TODOBorderImage() const;

    void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

    void ProcessCategory(IDetailCategoryBuilder& Category) const;
private:
};

#undef LOCTEXT_NAMESPACE
