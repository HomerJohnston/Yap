#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

#include "GameplayTagsManager.h"
#include "SGameplayTagPicker.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Yap/YapLog.h"
#include "Yap/Globals/YapEditorWarning.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapTagHelpers.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------

SYapGameplayTagTypedPicker::SYapGameplayTagTypedPicker()
	: Tag(*this)
{
}

// ------------------------------------------------------------------------------------------------

SLATE_IMPLEMENT_WIDGET(SYapGameplayTagTypedPicker)
void SYapGameplayTagTypedPicker::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Tag", Tag, EInvalidateWidgetReason::Layout);
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;
	
	Tag.Assign(*this, InArgs._Tag);

	Filter = InArgs._Filter;
	
	TagIniFileName = InArgs._TagIniFileName;

	bReadOnly = InArgs._ReadOnly;

	bEnableNavigation = InArgs._EnableNavigation;

	OnTagChanged = InArgs._OnTagChanged;
	
	ChildSlot
	.Padding(0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(2, 0, 2, 0)
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SAssignNew(TextEditor, SEditableText)
			.SelectAllTextWhenFocused(true)
			.Text(this, &ThisClass::Text_TagValue)
			.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagText)
			.OnTextCommitted(this, &ThisClass::OnTextCommitted)
			.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
			.HintText(INVTEXT("\u2014"))
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

bool SYapGameplayTagTypedPicker::ShowClearButton() const
{
	return true;
}

// ------------------------------------------------------------------------------------------------

FText SYapGameplayTagTypedPicker::Text_TagValue() const
{
	if (!Tag.Get().IsValid())
	{
		return FText::GetEmpty();
	}
	
	FString TagAsString = Tag.Get().ToString();

	if (!Filter.IsEmpty() && TagAsString.StartsWith(Filter))
	{
		TagAsString = TagAsString.RightChop(Filter.Len() + 1);
	}
	
	return FText::FromString(TagAsString);
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnMenuOpenChanged(bool bOpen)
{
	if (bOpen && TagPicker.IsValid())
	{
		const FGameplayTag TagToHilight = Tag.Get();
		TagPicker->RequestScrollToView(TagToHilight);
							
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapGameplayTagTypedPicker::OnGetMenuContent()
{
	// If property is not set, well put the edited tag into a container and use that for picking.
	TArray<FGameplayTagContainer> TagContainers;
	
	const FGameplayTag TagToEdit = Tag.Get();
	TagContainers.Add(FGameplayTagContainer(TagToEdit));

	TagPicker = SNew(SGameplayTagPicker)
		.Filter(Filter)
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

FSlateColor SYapGameplayTagTypedPicker::ColorAndOpacity_TagText() const
{
	if (TextEditor->HasKeyboardFocus())
	{
		return YapColor::White;
	}

	/*
	if (!Tag.Get().IsValid())
	{
		return YapColor::DarkGray;
	}
*/
	
	return YapColor::Gray;
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnTextCommitted(const FText& NewTag, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::OnEnter)
	{
		return;
	}

	FString NewTagName = NewTag.ToString();
	
	if (!Filter.IsEmpty() && !NewTag.IsEmpty())
	{
		NewTagName = Filter + "." + NewTagName; 
	}

	if (NewTagName == Tag.Get().ToString())
	{
		return;
	}

	ChangeTag(NewTagName);
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapGameplayTagTypedPicker::ColorAndOpacity_TagIcon() const
{
	return IsValueValid() ? YapColor::Gray_SemiTrans : YapColor::Gray_Glass;
}

// ------------------------------------------------------------------------------------------------

bool SYapGameplayTagTypedPicker::IsValueValid() const
{
	FGameplayTag TempTag = Tag.Get();

	if (TempTag.IsValid())
	{
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers)
{
	FGameplayTag NewTag = TagContainers[0].First();
	
	ChangeTag(NewTag.ToString());
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::ChangeTag(const FString& NewTagString)
{
	if (!Asset.IsValid())
	{
		// TODO
		UE_LOG(LogYap, Warning, TEXT("SYapGameplayTagTypedPicker has no Asset assigned, this isn't supported!"));
		return;
	}

	FGameplayTag OldTag = Tag.Get();
	FGameplayTag NewTag;
	FString OldTagString = (Tag.Get().IsValid()) ? Tag.Get().ToString() : "";
	
	{
		FYapScopedTransaction Transaction("TODO", INVTEXT("TODO"), nullptr);

		if (!NewTagString.IsEmpty())
		{
			NewTag = Yap::Tags::GetOrAddTag(NewTagString);
			(void)OnTagChanged.ExecuteIfBound(NewTag);
		}
		else
		{
			(void)OnTagChanged.ExecuteIfBound(FGameplayTag::EmptyTag);
		}
	}
	
	if (OldTag.IsValid() && NewTag.IsValid())
	{
		Yap::Tags::RedirectTags({{OldTag, NewTag}}, true);
	}
	else if (OldTag.IsValid() && !NewTag.IsValid())
	{
		// We will watch the old tag to see if it stops being used and can be removed
		UYapEditorSubsystem::AddTagPendingDeletion(OldTag);
	}
}

// ------------------------------------------------------------------------------------------------

bool SYapGameplayTagTypedPicker::VerifyNewTagString(const FString& NewTagString) const
{
	FText ErrorMsg;
	
	if (!NewTagString.IsEmpty() && !UGameplayTagsManager::Get().IsValidGameplayTagString(NewTagString, &ErrorMsg))
	{
		Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("Tag_Error", "Tag Error"), ErrorMsg);
		return false;
	}
	
	return true;
}

#undef LOCTEXT_NAMESPACE
