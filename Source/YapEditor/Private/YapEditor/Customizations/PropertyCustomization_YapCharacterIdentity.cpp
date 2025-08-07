// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/PropertyCustomization_YapCharacterIdentity.h"

#include "DetailWidgetRow.h"
#include "Yap/YapCharacterComponent.h"
#include "Yap/YapLog.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/SlateWidgets/SYapCharacterIDSelector.h"
#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FPropertyCustomization_YapCharacterIdentity::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	static const FName TagPropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Tag);
	static const FName NamePropertyName = GET_MEMBER_NAME_CHECKED(FYapCharacterIdentity, Identity_Name);

	TagProperty = StructPropertyHandle->GetChildHandle(TagPropertyName);
	NameProperty = StructPropertyHandle->GetChildHandle(NamePropertyName);
	
	HeaderRow.NameContent()
	[
		SNew(STextBlock)
		.Text(this, &FPropertyCustomization_YapCharacterIdentity::Text_NameContent)
		.Font(StructCustomizationUtils.GetRegularFont())
	];

	HeaderRow.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SYapCharacterIDSelector)
			.IDName(this, &FPropertyCustomization_YapCharacterIdentity::GetIDName)
			.IDTag(this, &FPropertyCustomization_YapCharacterIdentity::GetIDTag)
			.OnNameChanged(this, &FPropertyCustomization_YapCharacterIdentity::SetIDName)
			.OnTagChanged(this, &FPropertyCustomization_YapCharacterIdentity::SetIDTag)
		]
	];
}

void FPropertyCustomization_YapCharacterIdentity::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}

FText FPropertyCustomization_YapCharacterIdentity::Text_NameContent() const
{
	TArray<FText> Text;
	Text.Add(LOCTEXT("CharacterIdentity_PropertyName", "Identity"));
	Text.Add(LOCTEXT("CharacterIdentity_Unset", "(Unset)"));

	void* TagRaw;
	TagProperty->GetValueData(TagRaw);

	FGameplayTag* TagPtr = reinterpret_cast<FGameplayTag*>(TagRaw);

	if (TagPtr->IsValid())
	{
		Text[1] = LOCTEXT("CharacterIdentity_UsingTag", "(Project Character)");
	}
	else
	{
		FName NameValue;
		NameProperty->GetValue(NameValue);

		if (NameValue != NAME_None)
		{
			Text[1] = LOCTEXT("CharacterIdentity_UsingName", "(Custom ID)");
		}
	}

	return FText::Join(INVTEXT(" "), Text);
}

FName FPropertyCustomization_YapCharacterIdentity::GetIDName() const
{
	FName Val;

	NameProperty->GetValue(Val);

	return Val;
}

FGameplayTag FPropertyCustomization_YapCharacterIdentity::GetIDTag() const
{
	void* Address;
	TagProperty->GetValueData(Address);

	FGameplayTag* Tag = reinterpret_cast<FGameplayTag*>(Address);

	return *Tag;
}

void FPropertyCustomization_YapCharacterIdentity::SetIDName(FName Name) const
{
	FYapScopedTransaction Transaction(FName("TODO"), INVTEXT("Modify Character ID"), nullptr);

	void* TagRaw;
	TagProperty->GetValueData(TagRaw);

	FGameplayTag* TagPtr = reinterpret_cast<FGameplayTag*>(TagRaw);
	*TagPtr = FGameplayTag::EmptyTag;
	
	NameProperty->SetValue(Name);
}

void FPropertyCustomization_YapCharacterIdentity::SetIDTag(FGameplayTag Tag) const
{
	FYapScopedTransaction Transaction(FName("TODO"), INVTEXT("Modify Character ID"), nullptr);

//	FPropertyAccess::Result NameResult = NameProperty->SetValue(NAME_None);

	void* NameRaw;
	NameProperty->GetValueData(NameRaw);

	FName* NamePtr = reinterpret_cast<FName*>(NameRaw);
	*NamePtr = NAME_None; 

	TagProperty->SetValueFromFormattedString(Tag.ToString());
}

#undef LOCTEXT_NAMESPACE
