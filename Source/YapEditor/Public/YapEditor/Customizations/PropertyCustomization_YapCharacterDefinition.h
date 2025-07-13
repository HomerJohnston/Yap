// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

struct FYapCharacterDefinition;
/**
 * 
 */
class FPropertyCustomization_YapCharacterDefinition : public IPropertyTypeCustomization
{
    // CONSTRUCTION -------------------------------------------------------------------------------

public:
    
    static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapCharacterDefinition>(); }
    
    // STATE --------------------------------------------------------------------------------------
    
protected:
    
    // OVERRIDES ----------------------------------------------------------------------------------

protected:
    
    void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    // METHODS ------------------------------------------------------------------------------------

    FText ToolTipText_AssetStatus(FYapCharacterDefinition* CharacterDefinition) const;
    
    FLinearColor AssetStatusColor(FYapCharacterDefinition* CharacterDefinition) const;
    
    FText ToolTipText_TagStatus(FYapCharacterDefinition* CharacterDefinition) const;
    
    FLinearColor Color_TagStatus(FYapCharacterDefinition* CharacterDefinition) const;
    
    void OnSetNewCharacterAsset(TSharedPtr<IPropertyHandle> AssetPropertyHandle) const;

protected:

    static FLinearColor InvalidCharacterColor();
    
    static FLinearColor ErrorColor();

    static FLinearColor WarningColor();

    static FLinearColor OKColor();

    static FLinearColor TagNotInParentColor();

};