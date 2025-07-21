// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapPortraitList.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"

#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/SlateWidgets/SYapHyperlink.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FPropertyCustomization_YapPortraitList::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    
}

void FPropertyCustomization_YapPortraitList::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    TSharedPtr<IPropertyHandle> PortraitsMapProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapPortraitList, Map));

//    StructBuilder.AddProperty(PortraitsMapProperty.ToSharedRef());
    
    PortraitsMapProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FPropertyCustomization_YapPortraitList::OnPropertyValueChanged_PortraitsMap));
    
    uint32 Elements;
    PortraitsMapProperty->GetNumChildren(Elements);

    for (uint32 i = 0; i < Elements; ++i)
    {
        TSharedPtr<IPropertyHandle> PortraitProperty = PortraitsMapProperty->GetChildHandle(i);
        
        if (PortraitProperty.IsValid())
        {
            TSharedPtr<IPropertyHandle> PortraitNameProperty = PortraitProperty->GetKeyHandle();

            FName Name;
            PortraitNameProperty->GetValue(Name);

            const FGameplayTag& Tag = FGameplayTag::RequestGameplayTag(Name, false);

            FName LeafName = Yap::Tags::GetLeafOfTag(Tag);
            
            StructBuilder.AddCustomRow(INVTEXT("TODO"))
            .NameContent()
            [
                SNew(SBorder)
                .BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_RoundedSquare))
                .BorderBackgroundColor(YapColor::DarkGray)
                [
                    SNew(STextBlock)
                    .Text(FText::FromName(LeafName))
                    .Font(StructCustomizationUtils.GetRegularFont())   
                ]
            ]

            .ValueContent()
            [
                PortraitProperty->CreatePropertyValueWidget()
            ];
        }
    }

    
    /*
    FDetailWidgetRow X = StructBuilder.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
    [
        SNew(SBox)
        //.Visibility(this, &FDetailCustomization_YapCharacter::Visibility_MoodTagsOutOfDateWarning)
        //.IsEnabled_Lambda( [this] () { return GetHasObsoleteData() ? false : true; } )
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Font(YapFonts.Font_WarningText)
                .SimpleTextMode(true)
                .Text(LOCTEXT("CharacterPortraits_MoodTagsNeedRefresh", "Portraits list is out of date, "))
                .ColorAndOpacity(YapColor::OrangeRed)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SYapHyperlink)
                .Text(LOCTEXT("CharacterPortraits_PerformMoodTagsRefresh", "click to refresh"))
                //.OnNavigate(this, &FDetailCustomization_YapCharacter::OnClicked_RefreshMoodTagsButton)
                .ToolTipText(LOCTEXT("RefreshMoodTags_ToolTIp", "Will process the portraits list, removing entries which are no longer present in project settings, and adding missing entries."))
            ]
        ]
    ];
    */
}

void FPropertyCustomization_YapPortraitList::OnPropertyValueChanged_PortraitsMap()
{
    UE_LOG(LogTemp, Display, TEXT("ASDASFASF"));
}

#undef LOCTEXT_NAMESPACE