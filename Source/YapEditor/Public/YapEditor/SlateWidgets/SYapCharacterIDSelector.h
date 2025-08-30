// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once
#include "GameplayTagContainer.h"
#include "SGameplayTagChip.h"

class SEditableTextBox;
class SComboButton;
class SMenuAnchor;
class SGameplayTagPicker;

class SYapCharacterIDSelector : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SYapCharacterIDSelector, SCompoundWidget)
	
public:

	DECLARE_DELEGATE_OneParam(FOnTagChanged, const FGameplayTag)
	
	DECLARE_DELEGATE_OneParam(FOnNameChanged, const FName)

	SLATE_BEGIN_ARGS(SYapCharacterIDSelector)
	{}
	SLATE_ARGUMENT(UObject*, Asset)

	SLATE_ATTRIBUTE(FName, IDName)

	SLATE_ATTRIBUTE(FGameplayTag, IDTag)
		
		SLATE_ARGUMENT(FString, Filter)

		SLATE_ARGUMENT(FString, TagIniFileName)

		SLATE_ARGUMENT(bool, ReadOnly)

		SLATE_ARGUMENT(bool, EnableNavigation)

		SLATE_EVENT(SGameplayTagChip::FOnNavigate, OnNavigate)

		SLATE_EVENT(SGameplayTagChip::FOnMenu, OnMenu)

		SLATE_EVENT(FOnTagChanged, OnTagChanged)

		SLATE_EVENT(FOnNameChanged, OnNameChanged)
		
	SLATE_END_ARGS()

	YAPEDITOR_API SYapCharacterIDSelector();

private:
	TSlateAttribute<FName> IDName;
	
	TSlateAttribute<FGameplayTag> IDTag;
	
	FString TagFilter;

	FString TagIniFileName;
	
	bool bReadOnly;

	bool bEnableNavigation;

	FOnTagChanged OnTagChanged;

	FOnNameChanged OnNameChanged;
	
	TSharedPtr<SMenuAnchor> MenuAnchor;

	TSharedPtr<SComboButton> ComboButton;

	TSharedPtr<SGameplayTagPicker> TagPicker;

	TSharedPtr<SEditableTextBox> TextEditor;
	
public:
	void Construct(const FArguments& InArgs);

protected:
	bool ShowClearButton() const;

	FText Text_TagValue() const;

	void OnTextCommitted(const FText& NewTag, ETextCommit::Type CommitType);

	void OnMenuOpenChanged(bool bOpen);

	TSharedRef<SWidget> OnGetMenuContent();

	FSlateColor ColorAndOpacity_TagText() const;
	
	FSlateColor ColorAndOpacity_TagIcon() const;
	
	bool IsTagValueValid() const;
	
	void OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers);

	void ChangeTag(const FString& NewTagString);
	
	bool VerifyNewTagString(const FString& NewTagString) const;

	bool AttributesBound() const;
};