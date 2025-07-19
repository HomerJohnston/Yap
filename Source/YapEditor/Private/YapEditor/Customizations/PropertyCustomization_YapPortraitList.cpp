// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapPortraitList.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/Globals/YapTagHelpers.h"

void FPropertyCustomization_YapPortraitList::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    
}

void FPropertyCustomization_YapPortraitList::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    TSharedPtr<IPropertyHandle> PortraitsMapProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FYapPortraitList, Map));

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
}
