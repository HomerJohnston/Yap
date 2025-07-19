// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IDetailCustomization.h"

class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FPropertyCustomization_YapPortraitList : public IPropertyTypeCustomization
{
public:
    
    static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapPortraitList>(); }
  
    void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
};

#undef LOCTEXT_NAMESPACE