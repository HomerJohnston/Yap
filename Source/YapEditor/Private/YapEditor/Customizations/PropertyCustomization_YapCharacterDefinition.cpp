// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapCharacterDefinition.h"

#include "DetailWidgetRow.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterDefinition.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorColor.h"

#define LOCTEXT_NAMESPACE "YapEditor"

#undef LOCTEXT_NAMESPACE
void FPropertyCustomization_YapCharacterDefinition::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    static const FName TagPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterDefinition, CharacterTag);
    static const FName AssetPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterDefinition, CharacterAsset);

    TSharedPtr<IPropertyHandle> TagPropertyHandle = StructPropertyHandle->GetChildHandle(TagPropertyName);
    TSharedPtr<IPropertyHandle> AssetPropertyHandle = StructPropertyHandle->GetChildHandle(AssetPropertyName);

    TSharedPtr<IPropertyHandle> Test = StructPropertyHandle;
    
    void* CharacterDefinitionAddress;
    
    if (StructPropertyHandle->GetValueData(CharacterDefinitionAddress) != FPropertyAccess::Success)
    {
        return;
    }

    FYapCharacterDefinition* CharacterDefinition = static_cast<FYapCharacterDefinition*>(CharacterDefinitionAddress);
    
    HeaderRow.NameContent()
    .HAlign(HAlign_Fill)
    .VAlign(VAlign_Fill)
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        .Padding(-24, 0, 0, 0)
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
            .WidthOverride(8)
            [
                SNew(SColorBlock)
                .Color(this, &FPropertyCustomization_YapCharacterDefinition::TagStatusColor, CharacterDefinition)    
            ]
        ]
        + SOverlay::Slot()
        .Padding(0, 0, 4, 0)
        .VAlign(VAlign_Center)
        [
            TagPropertyHandle->CreatePropertyValueWidgetWithCustomization(nullptr)
        ]
    ];
    
    HeaderRow.ValueContent()
    .HAlign(HAlign_Fill)
    .VAlign(VAlign_Fill)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .Padding(-12, 0, -38, 0)
            .HAlign(HAlign_Left)
            [
                SNew(SBox)
                .WidthOverride(8)
                [
                    SNew(SColorBlock)
                    .Color(this, &FPropertyCustomization_YapCharacterDefinition::AssetStatusColor, CharacterDefinition)
                ]
            ]
            + SOverlay::Slot()
            .Padding(0, 0, 0, 0)
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Fill)
            [
                AssetPropertyHandle->CreatePropertyValueWidgetWithCustomization(nullptr)
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(8, 4, 0, 4)
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .HeightOverride(32)
            [
                SNew(SButton)
                .Text(INVTEXT("Dialogue"))
                .IsEnabled(false)
                .ToolTipText(INVTEXT("Placeholder button."))
                .VAlign(VAlign_Center)
            ]
        ]
    ];
}

void FPropertyCustomization_YapCharacterDefinition::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::AssetStatusColor(FYapCharacterDefinition* CharacterDefinition) const
{
    if (CharacterDefinition->ErrorState == EYapCharacterDefinitionErrorState::AssetConflict)
    {
        return ErrorColor();
    }

    if (CharacterDefinition->CharacterAsset.IsNull())
    {
        return WarningColor();
    }

    return OKColor();
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::TagStatusColor(FYapCharacterDefinition* CharacterDefinition) const
{
    if (CharacterDefinition->ErrorState == EYapCharacterDefinitionErrorState::TagConflict)
    {
        return ErrorColor();
    }

    const FGameplayTag& Tag = CharacterDefinition->CharacterTag;
    
    if (!Tag.IsValid())
    {
        return WarningColor();
    }

    if (!Tag.MatchesTag(UYapProjectSettings::GetCharacterTagParent()))
    {
        return TagNotInParentColor();
    }

    return OKColor();
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::ErrorColor()
{
    return YapColor::Red;
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::WarningColor()
{
    return YapColor::Orange;
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::OKColor()
{
    return YapColor::Noir_Glass;
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::TagNotInParentColor()
{
    return YapColor::Blue_Glass;
}
