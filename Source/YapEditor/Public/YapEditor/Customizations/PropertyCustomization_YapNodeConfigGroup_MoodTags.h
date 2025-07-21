// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

struct FYapNodeConfigGroup_MoodTags;
/**
 * 
 */
class FPropertyCustomization_YapNodeConfigGroup_MoodTags : public IPropertyTypeCustomization
{
public:
    
    static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FPropertyCustomization_YapNodeConfigGroup_MoodTags>(); }
  
    const FString DefaultMoodTagRoot = "Yap.Dialogue.Mood";
    
private:
    const TArray<FString> DefaultMoodTags
    {
        "Angry",
        "Calm",
        "Confused",
        "Disgusted",
        "Happy",
        "Injured",
        "Laughing",
        "Panicked",
        "Sad",
        "Scared",
        "Smirking",
        "Stressed",
        "Surprised",
        "Thinking",
        "Tired"
    };

    void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    
    bool IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const;
    
    FReply OnClicked_ResetDefaultMoodTags(TSharedPtr<IPropertyHandle> MoodTagsRootProperty) const;

    FReply OnClicked_DeleteAllMoodTags(TSharedPtr<IPropertyHandle> MoodTagsRootProperty) const;

    FReply OnClicked_OpenMoodTagsManager(TSharedPtr<IPropertyHandle> MoodTagsRootProperty);

    FReply OnClicked_RefreshMoodTagIcons();
	
    EVisibility Visibility_BottomPanel(TSharedPtr<IPropertyHandle> MoodTagsDisabledProperty) const;

    EVisibility Visibility_MoodTagsRootHint(TSharedPtr<IPropertyHandle> MoodTagsRootProperty) const;

    FText ToolTipText_DefaultMoodTags() const;
};