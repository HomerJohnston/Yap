// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

struct FYapCharacterStaticDefinition;
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
    TWeakPtr<IPropertyHandle> CharacterDefinitionProperty_Weak;

    FYapCharacterStaticDefinition* GetCharacterDefinition() const;
    TSharedPtr<IPropertyHandle> GetTagPropertyHandle() const;
    TSharedPtr<IPropertyHandle> GetAssetPropertyHandle() const;
    
    FGameplayTag GetCharacterTag() const;

    // OVERRIDES ----------------------------------------------------------------------------------

protected:
    void OnSetNewCharacterAsset(const FAssetData& AssetData);

    void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    // METHODS ------------------------------------------------------------------------------------

    FText ToolTipText_AssetStatus() const;
    
    FLinearColor AssetStatusColor() const;
    
    FText ToolTipText_TagStatus() const;
    
    FLinearColor Color_TagStatus() const;
    
    FString ObjectPath_CharacterAsset() const;

    static bool OnShouldFilterAsset_CharacterAsset(const FAssetData& AssetData); 
    
    FReply OnOpenCharacterAsset() const;

    bool GetCharacterHasErrors() const;

    EVisibility Visibility_CharacterErrorsButtonBorder() const;

    FSlateColor ButtonColorAndOpacity_OpenCharacterAsset() const;

protected:

    static FLinearColor InvalidCharacterColor();
    
    static FLinearColor ErrorColor();

    static FLinearColor WarningColor();

    static FLinearColor OKColor();

    static FLinearColor TagNotInParentColor();
};