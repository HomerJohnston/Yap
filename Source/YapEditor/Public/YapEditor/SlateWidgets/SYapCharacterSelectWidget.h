// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once
#include "GameplayTagContainer.h"

struct FGameplayTag;
struct FYapFragment;
class UFlowNode_YapDialogue;

class SYapCharacterSelectWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SYapCharacterSelectWidget) :
        _DialogueNode(nullptr),
        _FragmentIndex(INDEX_NONE)
    {}
        SLATE_ATTRIBUTE(UFlowNode_YapDialogue*, DialogueNode)
        SLATE_ARGUMENT(int32, FragmentIndex)
        SLATE_ARGUMENT(FGameplayTag, CurrentCharacterTag)
        SLATE_ARGUMENT(TDelegate<void(const FGameplayTag&)>, SetCharacterFunction)
        
    SLATE_END_ARGS()

private:
    TAttribute<UFlowNode_YapDialogue*> DialogueNode = nullptr;
    int32 FragmentIndex = INDEX_NONE;
    FGameplayTag CurrentCharacterTag = FGameplayTag::EmptyTag;
    TDelegate<void(const FGameplayTag&)> SetCharacterFunction;

    TSharedPtr<SWidget> FilterSearchBox;
    
    TSharedPtr<SScrollBox> CharacterList;
        
    static TArray<FName> RecentlySelectedCharacterTags;

    FGameplayTag FilteredCharacterTag = FGameplayTag::EmptyTag;
    
public:
    void OnTextChanged_CharacterSelectorFilter(const FText& NewFilterText);

    void OnTextCommitted_CharacterSelectorFilter(const FText& Text, ETextCommit::Type Arg);
    
    void Construct(const FArguments& InArgs);
    
    void UpdateCharacterSelector(const FText& FilterText);

    UFlowNode_YapDialogue* GetDialogueNode() const;

    FYapFragment& GetFragment();

    void UpdateRecentlySelectedCharacterTags(const FGameplayTag& CharacterTag);

    void SelectCharacter(const FGameplayTag& NewCharacterTag);
    
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent);
    
    virtual bool SupportsKeyboardFocus() const override { return true; }
};
