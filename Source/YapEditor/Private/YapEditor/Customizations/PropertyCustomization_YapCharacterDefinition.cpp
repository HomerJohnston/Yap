// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapCharacterDefinition.h"

#include "DetailWidgetRow.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapCharacterDefinition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Interfaces/IYapCharacterInterface.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/SlateWidgets/SYapPropertyMenuAssetPicker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

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
                .ToolTipText(this, &FPropertyCustomization_YapCharacterDefinition::ToolTipText_TagStatus, CharacterDefinition)
                .Color(this, &FPropertyCustomization_YapCharacterDefinition::Color_TagStatus, CharacterDefinition)    
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
                    .ToolTipText(this, &FPropertyCustomization_YapCharacterDefinition::ToolTipText_AssetStatus, CharacterDefinition)
                    .Color(this, &FPropertyCustomization_YapCharacterDefinition::AssetStatusColor, CharacterDefinition)
                ]
            ]
            + SOverlay::Slot()
            .Padding(0, 0, 0, 0)
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Fill)
            [
                // TODO: Difficult. I want the picker to be able to list ONLY blueprints and assets that implement the Yap Character Interface.
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
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(8, 4, 0, 4)
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .HeightOverride(32)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SButton)
                    .Text(INVTEXT("Open"))
                    .ToolTipText(LOCTEXT("OpenCharacterAsset_ToolTip", "Open this asset."))
                    .VAlign(VAlign_Center)
                    .OnClicked(this, &FPropertyCustomization_YapCharacterDefinition::OnOpenCharacterAsset, AssetPropertyHandle)
                    .ButtonColorAndOpacity(this, &FPropertyCustomization_YapCharacterDefinition::ButtonColorAndOpacity_OpenCharacterAsset, AssetPropertyHandle)   
                ]
                + SOverlay::Slot()
                .Padding(-4)
                [
                    SNew(SBorder)
                    .Visibility(this, &FPropertyCustomization_YapCharacterDefinition::Visibility_CharacterErrorsButtonBorder, AssetPropertyHandle)
                    .BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
                    .BorderBackgroundColor(YapColor::Orange)
                ]
            ]
        ]
    ];
    
    AssetPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset, AssetPropertyHandle));
}

void FPropertyCustomization_YapCharacterDefinition::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    
}

FText FPropertyCustomization_YapCharacterDefinition::ToolTipText_AssetStatus(FYapCharacterDefinition* CharacterDefinition) const
{
    if ((CharacterDefinition->ErrorState & EYapCharacterDefinitionErrorState::AssetConflict) != EYapCharacterDefinitionErrorState::OK)
    {
        return LOCTEXT("CharacterAssetConflict_ToolTip", "Error: Asset conflicts with another character.");
    }

    if (CharacterDefinition->CharacterAsset.IsNull())
    {
        return LOCTEXT("CharacterTagUnset_ToolTip", "Warning: Asset is unset.");
    }

    if (!IYapCharacterInterface::IsAsset_YapCharacter(CharacterDefinition->CharacterAsset))
    {
        return LOCTEXT("CharacterTagNotUnderRoot_ToolTip", "Error: Asset does not implement Yap Character Interface.");
    }

    return FText::GetEmpty();
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::AssetStatusColor(FYapCharacterDefinition* CharacterDefinition) const
{
    if ((CharacterDefinition->ErrorState & EYapCharacterDefinitionErrorState::AssetConflict) != EYapCharacterDefinitionErrorState::OK)
    {
        return ErrorColor();
    }

    if (CharacterDefinition->CharacterAsset.IsNull())
    {
        return WarningColor();
    }

    if (!IYapCharacterInterface::IsAsset_YapCharacter(CharacterDefinition->CharacterAsset))
    {
        return InvalidCharacterColor();
    }

    return OKColor();
}

FText FPropertyCustomization_YapCharacterDefinition::ToolTipText_TagStatus(FYapCharacterDefinition* CharacterDefinition) const
{
    if ((CharacterDefinition->ErrorState & EYapCharacterDefinitionErrorState::TagConflict) != EYapCharacterDefinitionErrorState::OK)
    {
        return LOCTEXT("CharacterTagConflict_ToolTip", "Error: Tag conflicts with another character.");
    }

    const FGameplayTag& Tag = CharacterDefinition->CharacterTag;
    
    if (!Tag.IsValid())
    {
        return LOCTEXT("CharacterTagUnset_ToolTip", "Warning: Tag is unset.");
    }

    if (!Tag.MatchesTag(UYapProjectSettings::GetCharacterRootTag()))
    {
        return LOCTEXT("CharacterTagNotUnderRoot_ToolTip", "Note: Tag is not a child of the above character root tag.");
    }

    return FText::GetEmpty();
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::Color_TagStatus(FYapCharacterDefinition* CharacterDefinition) const
{
    if ((CharacterDefinition->ErrorState & EYapCharacterDefinitionErrorState::TagConflict) != EYapCharacterDefinitionErrorState::OK)
    {
        return ErrorColor();
    }

    const FGameplayTag& Tag = CharacterDefinition->CharacterTag;
    
    if (!Tag.IsValid())
    {
        return WarningColor();
    }

    if (!Tag.MatchesTag(UYapProjectSettings::GetCharacterRootTag()))
    {
        return TagNotInParentColor();
    }

    return OKColor();
}

void FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset(TSharedPtr<IPropertyHandle> AssetPropertyHandle) const
{
    UObject* Object;
    
    if (AssetPropertyHandle->GetValue(Object) == FPropertyAccess::Success)
    {
        if (Object)
        {
            if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
            {
                if (Blueprint->GeneratedClass && Blueprint->GeneratedClass->ImplementsInterface(UYapCharacterInterface::StaticClass()))
                {
                    
                }
                else
                {
                    Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("CharacterAsset_ErrorTitle", "Error"), LOCTEXT("CharacterAsset_ErrorDescription", "The selected asset is not a valid Yap character! Please select a valid character asset."));
                }
            }
            else
            {
                if (!Object->Implements<UYapCharacterInterface>())
                {
                    Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("CharacterAsset_ErrorTitle", "Error"), LOCTEXT("CharacterAsset_ErrorDescription", "The selected asset is not a valid Yap character! Please select a valid character asset."));
                }
            }
        }
        else
        {
            Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("CharacterAsset_ErrorTitle", "Error"), LOCTEXT("CharacterAsset_ErrorDescription", "The selected asset is not a valid Yap character! Please select a valid character asset."));
        }
    }
    else
    {
        Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("CharacterAsset_ErrorTitle", "Error"), LOCTEXT("CharacterAsset_ErrorDescription", "The selected asset is not a valid Yap character! Please select a valid character asset."));
    }
}

FReply FPropertyCustomization_YapCharacterDefinition::OnOpenCharacterAsset(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
    UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

    UObject* Asset = nullptr;

    PropertyHandle->GetValue(Asset);
    
    if (IsValid(Subsystem))
    {
        if (IsValid(Asset))
        {
            Subsystem->OpenEditorForAsset(Asset);
        }
    }
    
    return FReply::Handled();
}

bool FPropertyCustomization_YapCharacterDefinition::GetCharacterHasErrors(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
    UObject* Asset = nullptr;

    PropertyHandle->GetValue(Asset);

    UYapCharacterAsset* CharacterAsset = Cast<UYapCharacterAsset>(Asset);

    if (CharacterAsset)
    {
        return CharacterAsset->GetHasWarnings();
    }

    return false;
}

EVisibility FPropertyCustomization_YapCharacterDefinition::Visibility_CharacterErrorsButtonBorder(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
    if (GetCharacterHasErrors(PropertyHandle))
    {
        return EVisibility::HitTestInvisible;
    }

    return EVisibility::Collapsed;
}

FSlateColor FPropertyCustomization_YapCharacterDefinition::ButtonColorAndOpacity_OpenCharacterAsset(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
    if (GetCharacterHasErrors(PropertyHandle))
    {
        return YapColor::Orange;
    }

    return YapColor::White;
}


FLinearColor FPropertyCustomization_YapCharacterDefinition::InvalidCharacterColor()
{
    return YapColor::Error;
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

#undef LOCTEXT_NAMESPACE