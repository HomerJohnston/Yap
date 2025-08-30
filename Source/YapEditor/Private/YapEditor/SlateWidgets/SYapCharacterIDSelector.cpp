#include "YapEditor/SlateWidgets/SYapCharacterIDSelector.h"

#include "GameplayTagsManager.h"
#include "SGameplayTagPicker.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Yap/Globals/YapEditorWarning.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapTagHelpers.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------


SYapCharacterIDSelector::SYapCharacterIDSelector()
	:IDName(*this)
	,IDTag(*this)
{
}

// ------------------------------------------------------------------------------------------------

SLATE_IMPLEMENT_WIDGET(SYapCharacterIDSelector)
void SYapCharacterIDSelector::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "IDTag", IDTag, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "IDName", IDName, EInvalidateWidgetReason::Layout);
}

// ------------------------------------------------------------------------------------------------

void SYapCharacterIDSelector::Construct(const FArguments& InArgs)
{
	IDName.Assign(*this, InArgs._IDName);
	
	IDTag.Assign(*this, InArgs._IDTag);

	TagFilter = InArgs._Filter;
	
	TagIniFileName = InArgs._TagIniFileName;

	bReadOnly = InArgs._ReadOnly;

	bEnableNavigation = InArgs._EnableNavigation;

	OnTagChanged = InArgs._OnTagChanged;

	OnNameChanged = InArgs._OnNameChanged;
	
	ChildSlot
	.Padding(0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(2, 0, 2, 0)
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(TextEditor, SEditableTextBox)
				.Visibility_Lambda( [this] () { return !IsTagValueValid() ? EVisibility::Visible : EVisibility::Collapsed; } )
				.SelectAllTextWhenFocused(true)
				.Text(this, &ThisClass::Text_TagValue)
				.ForegroundColor(this, &ThisClass::ColorAndOpacity_TagText)
				//.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagText)
				.OnTextCommitted(this, &ThisClass::OnTextCommitted)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
				.HintText(INVTEXT("\u2014"))	
			]
			+ SOverlay::Slot()
			[
				SNew(STextBlock)
				.Visibility_Lambda( [this] () { return IsTagValueValid() ? EVisibility::Visible : EVisibility::Collapsed; } )
				.Text(this, &ThisClass::Text_TagValue)
				//.ForegroundColor(this, &ThisClass::ColorAndOpacity_TagText)
				//.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagText)
				//.OnTextCommitted(this, &ThisClass::OnTextCommitted)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
				//.HintText(INVTEXT("\u2014"))				
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SAssignNew(ComboButton, SComboButton)
			.ComboButtonStyle(FYapEditorStyle::Get(), YapStyles.ComboButtonStyle_YapGameplayTagTypedPicker)
			.Cursor(EMouseCursor::Default)
			.HasDownArrow(false)
			.ContentPadding(FMargin(3, 0, 3, 0))
			.Clipping(EWidgetClipping::OnDemand)
			.OnMenuOpenChanged(this, &ThisClass::OnMenuOpenChanged)
			.OnGetMenuContent(this, &ThisClass::OnGetMenuContent)
			.ButtonContent()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Tag))
				.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagIcon)
			]
		]
	];
}

// ------------------------------------------------------------------------------------------------

bool SYapCharacterIDSelector::ShowClearButton() const
{
	return true;
}

// ------------------------------------------------------------------------------------------------

FText SYapCharacterIDSelector::Text_TagValue() const
{
	if (!AttributesBound())
	{
		return INVTEXT("Error: something unbound");
	}

	if (IDName.Get() != NAME_None)
	{
		return FText::FromName(IDName.Get());
	}

	if (IDTag.Get().IsValid())
	{
		return FText::FromName(IDTag.Get().GetTagName());
	}

	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		return LOCTEXT("YapCharacterID_Unset", "<Unset>");
	}
	else
	{
		return LOCTEXT("YapCharacterID_AutoHint", "(auto)");
	}
	/*
	FString TagAsString = IDTag.Get().ToString();

	if (!TagFilter.IsEmpty() && TagAsString.StartsWith(TagFilter))
	{
		TagAsString = TagAsString.RightChop(TagFilter.Len() + 1);
	}
	
	return FText::FromString(TagAsString);
	*/
}

// ------------------------------------------------------------------------------------------------

void SYapCharacterIDSelector::OnMenuOpenChanged(bool bOpen)
{
	if (bOpen && TagPicker.IsValid())
	{
		const FGameplayTag TagToHilight = IDTag.Get();
		TagPicker->RequestScrollToView(TagToHilight);
							
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapCharacterIDSelector::OnGetMenuContent()
{
	// If property is not set, well put the edited tag into a container and use that for picking.
	TArray<FGameplayTagContainer> TagContainers;
	
	const FGameplayTag TagToEdit = IDTag.Get();
	TagContainers.Add(FGameplayTagContainer(TagToEdit));

	TagPicker = SNew(SGameplayTagPicker)
		.Filter(TagFilter)
		.SettingsName(TagIniFileName)
		.ShowMenuItems(true)
		.MaxHeight(350.0f)
		.MultiSelect(false)
		.OnTagChanged(this, &ThisClass::OnTagSelected)
		.Padding(2)
		.TagContainers(TagContainers);

	if (TagPicker->GetWidgetToFocusOnOpen())
	{
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}

	return TagPicker.ToSharedRef();
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapCharacterIDSelector::ColorAndOpacity_TagText() const
{
	if (!AttributesBound())
	{
		return YapColor::Error;
	}
	
	if (TextEditor->HasKeyboardFocus())
	{
		return YapColor::White;
	}

	if (IDName.Get() != NAME_None)
	{
		return YapColor::LightGreen;
	}

	if (IDTag.Get().IsValid())
	{
		return YapColor::LightBlue;
	}

	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		return YapColor::OrangeRed;
	}

	return YapColor::Gray;
}

// ------------------------------------------------------------------------------------------------

void SYapCharacterIDSelector::OnTextCommitted(const FText& NewTag, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::OnEnter)
	{
		return;
	}

	FString AsString = NewTag.ToString();

	FText Error;
	FText ErrorContext;

	if (!FName::IsValidXName(AsString, INVALID_NAME_CHARACTERS, &Error, &ErrorContext))
	{
		// TODO error logging
		return;
	}

	OnNameChanged.Execute(FName(AsString));

	/**
	FName NewIDName (AsString);

	if (!TagFilter.IsEmpty() && !NewTag.IsEmpty())
	{
		NewTagName = TagFilter + "." + NewTagName; 
	}

	if (NewTagName == IDTag.Get().ToString())
	{
		return;
	}

	ChangeTag(NewTagName);\
	*/
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapCharacterIDSelector::ColorAndOpacity_TagIcon() const
{
	//return IsValueValid() ? YapColor::Gray_SemiTrans : YapColor::Gray_Glass;
	return YapColor::Gray_SemiTrans;
}

// ------------------------------------------------------------------------------------------------

bool SYapCharacterIDSelector::IsTagValueValid() const
{
	FGameplayTag TempTag = IDTag.Get();

	if (TempTag.IsValid())
	{
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

void SYapCharacterIDSelector::OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers)
{
	UE_LOG(LogTemp, Display, TEXT("OnTagSelected"));
	
	FGameplayTag NewTag = TagContainers[0].First();
	
	ChangeTag(NewTag.ToString());
}

// ------------------------------------------------------------------------------------------------

void SYapCharacterIDSelector::ChangeTag(const FString& NewTagString)
{
	FGameplayTag NewTag;
	
	{
		FYapScopedTransaction Transaction("TODO", INVTEXT("TODO"), nullptr);

		if (!NewTagString.IsEmpty())
		{
			NewTag = Yap::Tags::GetTag(NewTagString);
			(void)OnTagChanged.ExecuteIfBound(NewTag);
		}
		else
		{
			(void)OnTagChanged.ExecuteIfBound(FGameplayTag::EmptyTag);
		}
	}

	/*
	if (OldTag.IsValid() && NewTag.IsValid())
	{
		Yap::Tags::RedirectTags({{OldTag, NewTag}}, true);
	}
	else if (OldTag.IsValid() && !NewTag.IsValid())
	{
		// We will watch the old tag to see if it stops being used and can be removed
		UYapEditorSubsystem::AddTagPendingDeletion(OldTag);
	}
	*/
}

// ------------------------------------------------------------------------------------------------

bool SYapCharacterIDSelector::VerifyNewTagString(const FString& NewTagString) const
{
	FText ErrorMsg;
	
	if (!NewTagString.IsEmpty() && !UGameplayTagsManager::Get().IsValidGameplayTagString(NewTagString, &ErrorMsg))
	{
		Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("Tag_Error", "Tag Error"), ErrorMsg);
		return false;
	}
	
	return true;
}

// ------------------------------------------------------------------------------------------------

bool SYapCharacterIDSelector::AttributesBound() const
{
	if (!IDName.IsBound(*this))
	{
		return false;
	}

	if (!IDTag.IsBound(*this))
	{
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
