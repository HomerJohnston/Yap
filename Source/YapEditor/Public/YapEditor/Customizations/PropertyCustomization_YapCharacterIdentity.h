// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IPropertyTypeCustomization.h"

struct FGameplayTag;
struct FYapCharacterIdentity;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FPropertyCustomization_YapCharacterIdentity : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapCharacterIdentity>(); }

	void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	TSharedPtr<IPropertyHandle> TagProperty;
	TSharedPtr<IPropertyHandle> NameProperty;
	
	FText Text_NameContent() const;
	
	FName GetIDName() const;

	FGameplayTag GetIDTag() const;

	void SetIDName(FName Name) const;

	void SetIDTag(FGameplayTag Tag) const;
};

#undef LOCTEXT_NAMESPACE