// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapCharacterIdentity.h"

#include "DetailWidgetRow.h"
#include "Yap/YapCharacterComponent.h"
#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FPropertyCustomization_YapCharacterIdentity::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	static const FName TagPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Tag);
	static const FName NamePropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Name);

	TSharedPtr<IPropertyHandle> TagProperty = StructPropertyHandle->GetChildHandle(TagPropertyName);
	TSharedPtr<IPropertyHandle> NameProperty = StructPropertyHandle->GetChildHandle(NamePropertyName);
	
	HeaderRow.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CharacterIdentity_PropertyName", "Identity"))
		.Font(StructCustomizationUtils.GetRegularFont())
	];

	HeaderRow.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			NameProperty->CreatePropertyValueWidget()
		]
		+ SVerticalBox::Slot()
		[
			TagProperty->CreatePropertyValueWidgetWithCustomization(nullptr)
//			SNew(SYapGameplayTagTypedPicker)
		]
	];
}

void FPropertyCustomization_YapCharacterIdentity::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}

#undef LOCTEXT_NAMESPACE