// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once
#include "GameplayTagContainer.h"
#include "SGameplayTagChip.h"

class SMenuAnchor;
class SComboButton;
class SEditableText;
class SGameplayTagPicker;

enum class EYapGameplayTagTypedPickerResponse : uint8
{
	Unspecified,
	DeleteOldTag,
	RedirectOldTag,
};

class SYapGameplayTagTypedPicker : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SYapGameplayTagTypedPicker, SCompoundWidget)
	
public:

	DECLARE_DELEGATE_OneParam(FOnTagChanged, const FGameplayTag)

	SLATE_BEGIN_ARGS(SYapGameplayTagTypedPicker)
	{}
		SLATE_ARGUMENT(UObject*, Asset)
		
		SLATE_ATTRIBUTE(FGameplayTag, Tag)
		
		SLATE_ARGUMENT(FString, Filter)

		SLATE_ARGUMENT(FString, TagIniFileName)

		SLATE_ARGUMENT(bool, ReadOnly)

		SLATE_ARGUMENT(bool, EnableNavigation)

		SLATE_EVENT(SGameplayTagChip::FOnNavigate, OnNavigate)

		SLATE_EVENT(SGameplayTagChip::FOnMenu, OnMenu)

		SLATE_EVENT(FOnTagChanged, OnTagChanged)
	SLATE_END_ARGS()

	YAPEDITOR_API SYapGameplayTagTypedPicker();

private:
	TWeakObjectPtr<UObject> Asset;
	
	TSlateAttribute<FGameplayTag> Tag;
	
	FString Filter;

	FString TagIniFileName;
	
	bool bReadOnly;

	bool bEnableNavigation;

	FOnTagChanged OnTagChanged;
	
	TSharedPtr<SMenuAnchor> MenuAnchor;

	TSharedPtr<SComboButton> ComboButton;

	TSharedPtr<SGameplayTagPicker> TagPicker;

	TSharedPtr<SEditableText> TextEditor;
	
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
	
	bool IsValueValid() const;
	
	void OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers);

	void ChangeTag(const FString& NewTagString);
	
	bool VerifyNewTagString(const FString& NewTagString) const;
};
