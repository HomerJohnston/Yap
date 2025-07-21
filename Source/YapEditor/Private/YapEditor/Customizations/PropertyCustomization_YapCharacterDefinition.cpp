// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapCharacterDefinition.h"

#include "DetailWidgetRow.h"
#include "EditorClassUtils.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapCharacterDefinition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Interfaces/IYapCharacterInterface.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/SlateWidgets/SYapPropertyMenuAssetPicker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset(const FAssetData& AssetData)
{
    UObject* Object = AssetData.GetAsset();

    if (!AssetPropertyHandle.IsValid())
    {
        return;
    }
    
    if (Object)
    {
        
        if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
        {
            UClass* ObjectClass = Blueprint->GeneratedClass;

            if (ObjectClass)
            {
                AssetPropertyHandle->SetValue(static_cast<UObject*>(ObjectClass));
            }
        }
        else
        {
            AssetPropertyHandle->SetValue(Object);
        }
    }
    else
    {
        AssetPropertyHandle->SetValue(static_cast<UObject*>(nullptr));
    }
    /*
    if (!Object->Implements<UYapCharacterInterface>())
    {
        Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("CharacterAsset_ErrorTitle", "Error"), LOCTEXT("CharacterAsset_ErrorDescription", "The selected asset is not a valid Yap character! Please select a valid character asset."));
    }
    */
}

void FPropertyCustomization_YapCharacterDefinition::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    static const FName TagPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterDefinition, CharacterTag);
    static const FName AssetPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterDefinition, CharacterAsset);

    TSharedPtr<IPropertyHandle> TagPropertyHandle = StructPropertyHandle->GetChildHandle(TagPropertyName);
    AssetPropertyHandle = StructPropertyHandle->GetChildHandle(AssetPropertyName);

    void* CharacterDefinitionAddress;
    
    if (StructPropertyHandle->GetValueData(CharacterDefinitionAddress) != FPropertyAccess::Success)
    {
        return;
    }

    CharacterDefinition = static_cast<FYapCharacterDefinition*>(CharacterDefinitionAddress);
    
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
                .ToolTipText(this, &FPropertyCustomization_YapCharacterDefinition::ToolTipText_TagStatus)
                .Color(this, &FPropertyCustomization_YapCharacterDefinition::Color_TagStatus)    
            ]
        ]
        + SOverlay::Slot()
        .Padding(0, 0, 4, 0)
        .VAlign(VAlign_Center)
        [
            TagPropertyHandle->CreatePropertyValueWidgetWithCustomization(nullptr)
        ]
    ];

    TSoftObjectPtr<UObject> AssetPropertySoftPtr;

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
                    .ToolTipText(this, &FPropertyCustomization_YapCharacterDefinition::ToolTipText_AssetStatus)
                    .Color(this, &FPropertyCustomization_YapCharacterDefinition::AssetStatusColor)
                ]
            ]
            + SOverlay::Slot()
            .Padding(0, 0, 0, 0)
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Fill)
            [
                // TODO: Difficult. I want the picker to be able to list ONLY blueprints and assets that implement the Yap Character Interface.
                SNew(SObjectPropertyEntryBox)
                .OnObjectChanged(this, &FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset)
                .ObjectPath(this, &FPropertyCustomization_YapCharacterDefinition::ObjectPath_CharacterAsset)
                .OnShouldFilterAsset(this, &FPropertyCustomization_YapCharacterDefinition::OnShouldFilterAsset_CharacterAsset)
				.ThumbnailPool(UThumbnailManager::Get().GetSharedThumbnailPool())
                .EnableContentPicker(true)
                .DisplayUseSelected(true)
                .DisplayBrowse(true)
                .DisplayThumbnail(true)
                .ThumbnailSizeOverride(48)
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
                    .Text(LOCTEXT("EditCharacterAsset_ButtonLabel", "Edit"))
                    .ToolTipText(LOCTEXT("EditCharacterAsset_ToolTip", "Open this character asset/blueprint to edit it."))
                    .VAlign(VAlign_Center)
                    .OnClicked(this, &FPropertyCustomization_YapCharacterDefinition::OnOpenCharacterAsset)
                    .ButtonColorAndOpacity(this, &FPropertyCustomization_YapCharacterDefinition::ButtonColorAndOpacity_OpenCharacterAsset)   
                ]
                + SOverlay::Slot()
                .Padding(-4)
                [
                    SNew(SBorder)
                    .Visibility(this, &FPropertyCustomization_YapCharacterDefinition::Visibility_CharacterErrorsButtonBorder)
                    .BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
                    .BorderBackgroundColor(YapColor::Orange)
                ]
            ]
        ]
    ];
    
    //AssetPropertyHandle->SetOnPropertyValueChangedWithData(TDelegate<void(const FPropertyChangedEvent&)>::CreateSP(this, &FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset_New, AssetPropertyHandle));
    //AssetPropertyHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateSP(this, &FPropertyCustomization_YapCharacterDefinition::OnSetNewCharacterAsset, AssetPropertyHandle));
}

void FPropertyCustomization_YapCharacterDefinition::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    
}

FText FPropertyCustomization_YapCharacterDefinition::ToolTipText_AssetStatus() const
{
    if (CharacterDefinition->CharacterAsset.IsNull())
    {
        return LOCTEXT("CharacterTagUnset_ToolTip", "Warning: Character is unset.");
    }
    
    if ((CharacterDefinition->ErrorState & EYapCharacterDefinitionErrorState::AssetConflict) != EYapCharacterDefinitionErrorState::OK)
    {
        return LOCTEXT("CharacterAssetConflict_ToolTip", "Error: Asset conflicts with another character.");
    }

    if (!CharacterDefinition->CharacterAsset.IsNull())
    {
        if (!IYapCharacterInterface::IsAsset_YapCharacter(CharacterDefinition->CharacterAsset))
        {
            return LOCTEXT("CharacterTagNotUnderRoot_ToolTip", "Error: Asset does not implement Yap Character Interface.");
        }
    }

    return FText::GetEmpty();
}

FLinearColor FPropertyCustomization_YapCharacterDefinition::AssetStatusColor() const
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

FText FPropertyCustomization_YapCharacterDefinition::ToolTipText_TagStatus() const
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

FLinearColor FPropertyCustomization_YapCharacterDefinition::Color_TagStatus() const
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

FString FPropertyCustomization_YapCharacterDefinition::ObjectPath_CharacterAsset() const
{
    if (CharacterDefinition->HasValidCharacterData())
    {
        if (!CharacterDefinition->CharacterAsset.IsNull())
        {
            return CharacterDefinition->CharacterAsset->GetPathName();
        }
    }
        
    return "";
}

bool FPropertyCustomization_YapCharacterDefinition::OnShouldFilterAsset_CharacterAsset(const FAssetData& AssetData) const
{
    // Check if blueprint implements interface
    FString YapCharacterInterfacePath = UYapCharacterInterface::StaticClass()->GetPathName();
    
    TArray<FString> Interfaces;

    FEditorClassUtils::GetImplementedInterfaceClassPathsFromAsset(AssetData, Interfaces);

    const FString ImplementedInterfaces = AssetData.GetTagValueRef<FString>(FBlueprintTags::ImplementedInterfaces);

    for (int32 i = 0; i < Interfaces.Num(); ++i)
    {
        Interfaces[i] = FPackageName::ExportTextPathToObjectPath(Interfaces[i]);
    }

    if (Interfaces.Contains(YapCharacterInterfacePath))
    {
        return false;
    }

    // Check if C++ implemented interface
    UClass* Class = AssetData.GetClass(EResolveClass::No);

    if (Class && Class->ImplementsInterface(UYapCharacterInterface::StaticClass()))
    {
        return false;
    }

    return true;
}

FReply FPropertyCustomization_YapCharacterDefinition::OnOpenCharacterAsset() const
{
    UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

    if (!CharacterDefinition->CharacterAsset.IsNull())
    {
        Subsystem->OpenEditorForAsset(CharacterDefinition->CharacterAsset.Get());
    }
    
    return FReply::Handled();
}

bool FPropertyCustomization_YapCharacterDefinition::GetCharacterHasErrors() const
{
    if (CharacterDefinition->CharacterAsset.IsNull())
    {
        return false;
    }

    UYapCharacterAsset* CharacterAsset = Cast<UYapCharacterAsset>(CharacterDefinition->CharacterAsset.Get());

    if (CharacterAsset)
    {
        return CharacterAsset->GetHasWarnings();
    }

    return false;
}

EVisibility FPropertyCustomization_YapCharacterDefinition::Visibility_CharacterErrorsButtonBorder() const
{
    if (GetCharacterHasErrors())
    {
        return EVisibility::HitTestInvisible;
    }

    return EVisibility::Collapsed;
}

FSlateColor FPropertyCustomization_YapCharacterDefinition::ButtonColorAndOpacity_OpenCharacterAsset() const
{
    if (GetCharacterHasErrors())
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
