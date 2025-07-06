// Copyright Epic Games, Inc. All Rights Reserved.

#include "YapEditor/SlateWidgets/SYapTextPropertyEditableTextBox.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Styling/AppStyle.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "Styling/StyleColors.h"
#include "Widgets/Layout/SLinkedBox.h"
#include "SSimpleComboButton.h"
#include "YapEditor/YapTransactions.h"

#define LOCTEXT_NAMESPACE "SYapTextPropertyEditableTextBox"

FText SYapTextPropertyEditableTextBox::MultipleValuesText(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"));

#if USE_STABLE_LOCALIZATION_KEYS

#endif // USE_STABLE_LOCALIZATION_KEYS

void SYapTextPropertyEditableStringTableReference::Construct(const FArguments& InArgs, const TSharedRef<IEditableTextProperty>& InEditableTextProperty)
{
	EditableTextProperty = InEditableTextProperty;

	OptionTextFilter = MakeShareable(new FOptionTextFilter(FOptionTextFilter::FItemToStringArray::CreateLambda([](const TSharedPtr<FAvailableStringTable>& InItem, OUT TArray< FString >& StringArray) {
		StringArray.Add(InItem->DisplayName.ToString());
	})));
	KeyTextFilter = MakeShareable(new FKeyTextFilter(FKeyTextFilter::FItemToStringArray::CreateLambda([](const TSharedPtr<FString>& InItem, OUT TArray< FString >& StringArray) {
		StringArray.Add(*InItem);
	})));

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	HorizontalBox->AddSlot()
		.Padding(0)
		[
			SAssignNew(StringTableOptionsCombo, SComboButton)
			.ComboButtonStyle(&InArgs._ComboStyle->ComboButtonStyle)
			.ContentPadding(FMargin(4.0, 2.0))
			.OnGetMenuContent(this, &SYapTextPropertyEditableStringTableReference::OnGetStringTableComboOptions)
			.OnComboBoxOpened(this, &SYapTextPropertyEditableStringTableReference::UpdateStringTableComboOptions)
			.CollapseMenuOnParentFocus(true)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &SYapTextPropertyEditableStringTableReference::GetStringTableComboContent)
				.ToolTipText(this, &SYapTextPropertyEditableStringTableReference::GetStringTableComboToolTip)
				.Font(InArgs._Font)
			]
		];

	HorizontalBox->AddSlot()
		.Padding(10, 0)
		[
			SAssignNew(StringTableKeysCombo, SComboButton)
			.ComboButtonStyle(&InArgs._ComboStyle->ComboButtonStyle)
			.ContentPadding(FMargin(4.0, 2.0))
			.IsEnabled(this, &SYapTextPropertyEditableStringTableReference::IsUnlinkEnabled)
			.OnGetMenuContent(this, &SYapTextPropertyEditableStringTableReference::OnGetStringTableKeyOptions)
			.OnComboBoxOpened(this, &SYapTextPropertyEditableStringTableReference::UpdateStringTableKeyOptions)
			.CollapseMenuOnParentFocus(true)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &SYapTextPropertyEditableStringTableReference::GetKeyComboContent)
				.ToolTipText(this, &SYapTextPropertyEditableStringTableReference::GetKeyComboToolTip)
				.Font(InArgs._Font)
			]
		];

	if (InArgs._AllowUnlink)
	{
		HorizontalBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.HeightOverride(22)
				.WidthOverride(22)
				[
					SNew(SButton)
					.ButtonStyle(InArgs._ButtonStyle)
					.ContentPadding(0)
					.ToolTipText(LOCTEXT("UnlinkStringTable", "Unlink"))
					.IsEnabled(this, &SYapTextPropertyEditableStringTableReference::IsUnlinkEnabled)
					.OnClicked(this, &SYapTextPropertyEditableStringTableReference::OnUnlinkClicked)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Delete"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			];
	}

	ChildSlot
	[
		HorizontalBox
	];
}

void SYapTextPropertyEditableStringTableReference::OnOptionsFilterTextChanged(const FText& InNewText)
{
	OptionTextFilter->SetRawFilterText(InNewText);
	OptionsSearchBox->SetError(OptionTextFilter->GetFilterErrorText());

	UpdateStringTableComboOptions();
}

void SYapTextPropertyEditableStringTableReference::OnKeysFilterTextChanged(const FText& InNewText)
{
	KeyTextFilter->SetRawFilterText(InNewText);
	KeysSearchBox->SetError(KeyTextFilter->GetFilterErrorText());

	UpdateStringTableKeyOptions();
}

TSharedRef<SWidget> SYapTextPropertyEditableStringTableReference::OnGetStringTableComboOptions()
{
	const FComboButtonStyle& ComboButtonStyle = FCoreStyle::Get().GetWidgetStyle< FComboButtonStyle >("ComboButton");
	return SNew(SBorder)
		.BorderImage(&ComboButtonStyle.MenuBorderBrush)
		.Padding(ComboButtonStyle.MenuBorderPadding)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(OptionsSearchBox, SSearchBox)
				.OnTextChanged(this, &SYapTextPropertyEditableStringTableReference::OnOptionsFilterTextChanged)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() { return StringTableComboOptions.IsEmpty() ? 0 : 1; })

				+SWidgetSwitcher::Slot() // Appears when there are no string tables with keys
				.Padding(12)
				[
					SNew(STextBlock).Text(LOCTEXT("EmptyStringTableList", "No string tables available"))
					.SimpleTextMode(true)
				]

				+SWidgetSwitcher::Slot() // Appears when there's a string table with at least a key
				[
					SNew(SBox)
					.Padding(4)
					.WidthOverride(280)
					.MaxDesiredHeight(600)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0, 5, 0, 0)
						[
							SAssignNew(StringTableOptionsList, SListView<TSharedPtr<FAvailableStringTable>>)
							.ListItemsSource(&StringTableComboOptions)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SYapTextPropertyEditableStringTableReference::OnGenerateStringTableComboOption)
							.OnSelectionChanged(this, &SYapTextPropertyEditableStringTableReference::OnStringTableComboChanged)
						]
					]
				]
			]
		];
}

TSharedRef<ITableRow> SYapTextPropertyEditableStringTableReference::OnGenerateStringTableComboOption(TSharedPtr<FAvailableStringTable> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(InItem->DisplayName)
			.SimpleTextMode(true)
			.ToolTipText(FText::FromName(InItem->TableId))
		];
}

TSharedRef<SWidget> SYapTextPropertyEditableStringTableReference::OnGetStringTableKeyOptions()
{
	const FComboButtonStyle& ComboButtonStyle = FCoreStyle::Get().GetWidgetStyle< FComboButtonStyle >("ComboButton");
	return SNew(SBorder)
		.BorderImage(&ComboButtonStyle.MenuBorderBrush)
		.Padding(ComboButtonStyle.MenuBorderPadding)
		[
			SNew(SBox)
			.Padding(4)
			.WidthOverride(280)
			.MaxDesiredHeight(600)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(KeysSearchBox, SSearchBox)
					.OnTextChanged(this, &SYapTextPropertyEditableStringTableReference::OnKeysFilterTextChanged)
				]
				+SVerticalBox::Slot()
				.FillHeight(1.f)
				.Padding(0, 5, 0, 0)
				[
					SAssignNew(StringTableKeysList, SListView<TSharedPtr<FString>>)
					.ListItemsSource(&KeyComboOptions)
					.SelectionMode(ESelectionMode::Single)
					.OnGenerateRow(this, &SYapTextPropertyEditableStringTableReference::OnGenerateStringTableKeyOption)
					.OnSelectionChanged(this, &SYapTextPropertyEditableStringTableReference::OnKeyComboChanged)
				]
			]
		];
}

TSharedRef<ITableRow> SYapTextPropertyEditableStringTableReference::OnGenerateStringTableKeyOption(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem))
			.SimpleTextMode(true)
			.ToolTipText(FText::FromString(*InItem))
		];
}

void SYapTextPropertyEditableStringTableReference::GetTableIdAndKey(FName& OutTableId, FString& OutKey) const
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts > 0)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		FTextInspector::GetTableIdAndKey(PropertyValue, OutTableId, OutKey);

		// Verify that all texts are using the same string table and key
		for (int32 TextIndex = 1; TextIndex < NumTexts; ++TextIndex)
		{
			FName TmpTableId;
			FString TmpKey;
			if (FTextInspector::GetTableIdAndKey(PropertyValue, TmpTableId, TmpKey) && OutTableId == TmpTableId)
			{
				if (!OutKey.Equals(TmpKey, ESearchCase::CaseSensitive))
				{
					// Not using the same key - clear the key but keep the table and keep enumerating to verify the table on the remaining texts
					OutKey.Reset();
				}
			}
			else
			{
				// Not using a string table, or using a different string table - clear both table ID and key
				OutTableId = FName();
				OutKey.Reset();
				break;
			}
		}
	}
}

void SYapTextPropertyEditableStringTableReference::SetTableIdAndKey(const FName InTableId, const FString& InKey)
{
	const FText TextToSet = FText::FromStringTable(InTableId, InKey);
	if (TextToSet.IsFromStringTable())
	{
		const int32 NumTexts = EditableTextProperty->GetNumTexts();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			EditableTextProperty->SetText(TextIndex, TextToSet);
		}
	}
}

void SYapTextPropertyEditableStringTableReference::OnStringTableComboChanged(TSharedPtr<FAvailableStringTable> NewSelection, ESelectInfo::Type SelectInfo)
{
	// If it's set from code, we did that on purpose
	if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid())
	{
		// Make sure any selected string table asset is loaded
		FName TableId = NewSelection->TableId;
		IStringTableEngineBridge::FullyLoadStringTableAsset(TableId);

		FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable(TableId);
		if (StringTable.IsValid())
		{
			// Just use the first key when changing the string table
			StringTable->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
			{
				SetTableIdAndKey(TableId, InKey);
				return false; // stop enumeration
			});

			StringTableOptionsCombo->SetIsOpen(false);

			OptionsSearchBox->SetText(FText::GetEmpty());
		}
	}
}

void SYapTextPropertyEditableStringTableReference::UpdateStringTableComboOptions()
{
	FName CurrentTableId;
	{
		FString TmpKey;
		GetTableIdAndKey(CurrentTableId, TmpKey);
	}

	TSharedPtr<FAvailableStringTable> SelectedStringTableComboEntry;
	StringTableComboOptions.Reset();

	// Process assets first (as they may currently be unloaded)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);

		TArray<FAssetData> StringTableAssets;
		AssetRegistryModule.Get().GetAssetsByClass(UStringTable::StaticClass()->GetClassPathName(), StringTableAssets);

		for (const FAssetData& StringTableAsset : StringTableAssets)
		{
			FName StringTableId = *StringTableAsset.GetObjectPathString();
			// Only allow string tables assets that have entries to be visible otherwise unexpected behavior happens for the user
			bool HasEntries = false;
			FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable(StringTableId);
			if (StringTable.IsValid())
			{
				StringTable->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
				{
					HasEntries = true;
					return false; // stop enumeration
				});
			}
			else
			{
				// Asset is currently unloaded, so just assume it has entries
				HasEntries = true;
			}

			if (!HasEntries)
			{
				continue; // continue on to the next string table asset
			}

			TSharedRef<FAvailableStringTable> AvailableStringTableEntry = MakeShared<FAvailableStringTable>();
			AvailableStringTableEntry->TableId = StringTableId;
			AvailableStringTableEntry->DisplayName = FText::FromName(StringTableAsset.AssetName);
			if (StringTableId == CurrentTableId)
			{
				SelectedStringTableComboEntry = AvailableStringTableEntry;
			}
			if (OptionTextFilter->PassesFilter(AvailableStringTableEntry))
			{
				StringTableComboOptions.Add(AvailableStringTableEntry);
			}
		}
	}

	// Process the remaining non-asset string tables now
	FStringTableRegistry::Get().EnumerateStringTables([&](const FName& InTableId, const FStringTableConstRef& InStringTable) -> bool
	{
		const bool bAlreadyAdded = StringTableComboOptions.ContainsByPredicate([InTableId](const TSharedPtr<FAvailableStringTable>& InAvailableStringTable)
		{
			return InAvailableStringTable->TableId == InTableId;
		});

		bool bHasEntries = false;
		InStringTable->EnumerateSourceStrings([&bHasEntries](const FString& InKey, const FString& InSourceString) -> bool
		{
			bHasEntries = true;
			return false; // stop enumeration
		});

		if (!bAlreadyAdded && bHasEntries)
		{
			TSharedRef<FAvailableStringTable> AvailableStringTableEntry = MakeShared<FAvailableStringTable>();
			AvailableStringTableEntry->TableId = InTableId;
			AvailableStringTableEntry->DisplayName = FText::FromName(InTableId);
			if (InTableId == CurrentTableId)
			{
				SelectedStringTableComboEntry = AvailableStringTableEntry;
			}
			if (OptionTextFilter->PassesFilter(AvailableStringTableEntry))
			{
				StringTableComboOptions.Add(AvailableStringTableEntry);
			}
		}

		return true; // continue enumeration
	});

	StringTableComboOptions.Sort([](const TSharedPtr<FAvailableStringTable>& InOne, const TSharedPtr<FAvailableStringTable>& InTwo)
	{
		return InOne->DisplayName.ToString() < InTwo->DisplayName.ToString();
	});

	StringTableOptionsList->RebuildList();

	if (SelectedStringTableComboEntry.IsValid())
	{
		StringTableOptionsList->SetItemSelection(SelectedStringTableComboEntry, true);
	}
	else
	{
		StringTableOptionsList->ClearSelection();
	}
}

FText SYapTextPropertyEditableStringTableReference::GetStringTableComboContent() const
{
	FName CurrentTableId;
	{
		FString TmpKey;
		GetTableIdAndKey(CurrentTableId, TmpKey);
	}

	return FText::FromString(FPackageName::GetLongPackageAssetName(CurrentTableId.ToString()));
}

FText SYapTextPropertyEditableStringTableReference::GetStringTableComboToolTip() const
{
	FName CurrentTableId;
	{
		FString TmpKey;
		GetTableIdAndKey(CurrentTableId, TmpKey);
	}

	return FText::FromName(CurrentTableId);
}

void SYapTextPropertyEditableStringTableReference::OnKeyComboChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	// If it's set from code, we did that on purpose
	if (SelectInfo != ESelectInfo::Direct && NewSelection.IsValid())
	{
		FName CurrentTableId;
		{
			FString TmpKey;
			GetTableIdAndKey(CurrentTableId, TmpKey);
		}

		SetTableIdAndKey(CurrentTableId, *NewSelection);

		StringTableKeysCombo->SetIsOpen(false);

		KeysSearchBox->SetText(FText::GetEmpty());
	}
}

void SYapTextPropertyEditableStringTableReference::UpdateStringTableKeyOptions()
{
	FName CurrentTableId;
	FString CurrentKey;
	GetTableIdAndKey(CurrentTableId, CurrentKey);

	TSharedPtr<FString> SelectedKeyComboEntry;
	KeyComboOptions.Reset();

	if (!CurrentTableId.IsNone())
	{
		FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable(CurrentTableId);
		if (StringTable.IsValid())
		{
			StringTable->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
			{
				TSharedRef<FString> KeyComboEntry = MakeShared<FString>(InKey);
				if (InKey.Equals(CurrentKey, ESearchCase::CaseSensitive))
				{
					SelectedKeyComboEntry = KeyComboEntry;
				}
				if (KeyTextFilter->PassesFilter(KeyComboEntry))
				{
					KeyComboOptions.Add(KeyComboEntry);
				}
				return true; // continue enumeration
			});
		}
	}

	KeyComboOptions.Sort([](const TSharedPtr<FString>& InOne, const TSharedPtr<FString>& InTwo)
	{
		return *InOne < *InTwo;
	});

	StringTableKeysList->RebuildList();

	if (SelectedKeyComboEntry.IsValid())
	{
		StringTableKeysList->SetItemSelection(SelectedKeyComboEntry, true);
	}
	else
	{
		StringTableKeysList->ClearSelection();
	}
}

FText SYapTextPropertyEditableStringTableReference::GetKeyComboContent() const
{
	FString CurrentKey;
	{
		FName TmpTableId;
		GetTableIdAndKey(TmpTableId, CurrentKey);
	}

	if (CurrentKey.IsEmpty())
	{
		return LOCTEXT("NoKeyLabel", "No Key");
	}

	return FText::FromString(MoveTemp(CurrentKey));
}

FText SYapTextPropertyEditableStringTableReference::GetKeyComboToolTip() const
{
	return GetKeyComboContent();
}

bool SYapTextPropertyEditableStringTableReference::IsUnlinkEnabled() const
{
	bool bEnabled = false;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
	{
		const FText CurrentText = EditableTextProperty->GetText(TextIndex);
		if (CurrentText.IsFromStringTable())
		{
			bEnabled = true;
			break;
		}
	}

	return bEnabled;
}

FReply SYapTextPropertyEditableStringTableReference::OnUnlinkClicked()
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
	{
		const FText CurrentText = EditableTextProperty->GetText(TextIndex);
		if (CurrentText.IsFromStringTable())
		{
			// Make a copy of the FText separate from the string table but generate a new stable namespace and key
			// This prevents problems with properties that disallow empty text (e.g. enum display name)
			FString NewNamespace;
			FString NewKey;
			EditableTextProperty->GetStableTextId(
				TextIndex,
				IEditableTextProperty::ETextPropertyEditAction::EditedKey,
				CurrentText.ToString(),
				FString(),
				FString(),
				NewNamespace,
				NewKey
			);
			
			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, CurrentText));
		}
	}

	return FReply::Handled();
}

/** Single row in the advanced text settings/localization menu. Has a similar appearance to a details row in the property editor. */
class STextPropertyEditableOptionRow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STextPropertyEditableOptionRow)
		: _IsHeader(false)
		, _ContentHAlign(HAlign_Fill)
		{}
		SLATE_ARGUMENT(bool, IsHeader)
		SLATE_ARGUMENT(EHorizontalAlignment, ContentHAlign)
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedRef<FLinkedBoxManager> InManager)
	{
		InArgs._Content.Widget->SetToolTip(GetToolTip());

		if (InArgs._IsHeader)
		{
			// Header row, text only, fills entire row
			ChildSlot
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
				.Padding(FMargin(0, 0, 0, 1))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryTop"))
					.BorderBackgroundColor(FSlateColor(FLinearColor::White))
					.Padding(FMargin(12, 8, 0, 8))
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
						.SimpleTextMode(true)
						.Font(FAppStyle::Get().GetFontStyle("PropertyWindow.BoldFont"))
						.Text(InArgs._Text)
						.ToolTip(GetToolTip())
					]
				]
			];
		}
		else
		{
			// Non-header row, has a name column followed by a value widget
			ChildSlot
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SLinkedBox, InManager)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
						.Padding(FMargin(0, 0, 0, 1))
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
							.BorderBackgroundColor(this, &STextPropertyEditableOptionRow::GetBackgroundColor)
							.Padding(FMargin(20, 3.5, 0, 3.5))
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
								.SimpleTextMode(true)
								.Text(InArgs._Text)
								.ToolTip(GetToolTip())
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
					.Padding(FMargin(0, 0, 0, 1))
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
						.BorderBackgroundColor(this, &STextPropertyEditableOptionRow::GetBackgroundColor)
						.Padding(FMargin(14, 3.5, 4, 3.5))
						.HAlign(InArgs._ContentHAlign)
						.VAlign(VAlign_Center)
						[
							InArgs._Content.Widget
						]
					]
				]
			];

			// Clear the tooltip from this widget since it's set on the name/value widgets now
			SetToolTip(nullptr);
		}
	}

private:
	FSlateColor GetBackgroundColor() const
	{
		if (IsHovered())
		{
			return FStyleColors::Header;
		}

		return FStyleColors::Panel;
	}
};

void SYapTextPropertyEditableTextBox::Construct(const FArguments& InArgs, const TSharedRef<IEditableTextProperty>& InEditableTextProperty)
{
	EditableTextProperty = InEditableTextProperty;
	Owner = InArgs._Owner;

	TSharedPtr<SHorizontalBox> HorizontalBox;

	const bool bIsPassword = EditableTextProperty->IsPassword();
	bIsMultiLine = EditableTextProperty->IsMultiLineText();
	if (bIsMultiLine)
	{
		ChildSlot
		[
			SAssignNew(HorizontalBox, SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			[
				SNew(SBox)
				.MinDesiredWidth(InArgs._MinDesiredWidth)
				.MaxDesiredHeight(InArgs._MaxDesiredHeight)
				[
					SAssignNew(MultiLineWidget, SMultiLineEditableTextBox)
					.Text(this, &SYapTextPropertyEditableTextBox::GetTextValue)
					.HintText(InArgs._HintText)
					.ToolTipText(this, &SYapTextPropertyEditableTextBox::GetToolTipText)
					.Style(InArgs._Style)
					.Font(InArgs._Font)
					.ForegroundColor(InArgs._ForegroundColor)
					.SelectAllTextWhenFocused(false)
					.ClearKeyboardFocusOnCommit(false)
					.OnTextChanged(this, &SYapTextPropertyEditableTextBox::OnTextChanged)
					.OnTextCommitted(this, &SYapTextPropertyEditableTextBox::OnTextCommitted)
					.SelectAllTextOnCommit(false)
					.IsReadOnly(this, &SYapTextPropertyEditableTextBox::IsSourceTextReadOnly)
					.IsEnabled_Lambda( [this] () { return !IsSourceTextReadOnly(); } )
					.AutoWrapText(InArgs._AutoWrapText)
					.WrapTextAt(InArgs._WrapTextAt)
					.ModiferKeyForNewLine(EModifierKey::Shift)
					.Padding(FMargin(6, 6, 6, 6))
				]
			]
		];

		PrimaryWidget = MultiLineWidget;
	}
	else
	{
		ChildSlot
		[
			SAssignNew(HorizontalBox, SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.MinDesiredWidth(InArgs._MinDesiredWidth)
				[
					SAssignNew(SingleLineWidget, SEditableTextBox)
					.Text(this, &SYapTextPropertyEditableTextBox::GetTextValue)
					.ToolTipText(this, &SYapTextPropertyEditableTextBox::GetToolTipText)
					.Style(InArgs._Style)
					.Font(InArgs._Font)
					.ForegroundColor(InArgs._ForegroundColor)
					.SelectAllTextWhenFocused(true)
					.ClearKeyboardFocusOnCommit(false)
					.OnTextChanged(this, &SYapTextPropertyEditableTextBox::OnTextChanged)
					.OnTextCommitted(this, &SYapTextPropertyEditableTextBox::OnTextCommitted)
					.SelectAllTextOnCommit(true)
					.IsReadOnly(this, &SYapTextPropertyEditableTextBox::IsSourceTextReadOnly)
					.IsPassword(bIsPassword)
				]
			]
		];

		PrimaryWidget = SingleLineWidget;
	}

	const TSharedRef<FLinkedBoxManager> LinkedBoxManager = MakeShared<FLinkedBoxManager>();
	const FSlateFontInfo PropertyNormalFont = FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont");

	HorizontalBox->AddSlot()
		.AutoWidth()
		[
			SNew(SSimpleComboButton)
			.Icon(this, &SYapTextPropertyEditableTextBox::GetAdvancedTextSettingsComboImage)
			.MenuContent()
			[
				SNew(SBox)
				.WidthOverride(340)
				.Padding(1)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.Text(LOCTEXT("TextLocalizableLabel", "Localize"))
						.ToolTipText(LOCTEXT("TextLocalizableCheckBoxToolTip", "Whether to assign this text a key and allow it to be gathered for localization.\nIf set to false, marks this text as 'culture invariant' to prevent it being gathered for localization."))
						.ContentHAlign(HAlign_Left)
						[
							SNew(SCheckBox)
							.IsEnabled(this, &SYapTextPropertyEditableTextBox::IsCultureInvariantFlagEnabled)
							.IsChecked(this, &SYapTextPropertyEditableTextBox::GetLocalizableCheckState)
							.OnCheckStateChanged(this, &SYapTextPropertyEditableTextBox::HandleLocalizableCheckStateChanged)
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.IsHeader(true)
						.Text(LOCTEXT("TextReferencedTextLabel", "Referenced Text"))
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.Text(LOCTEXT("TextStringTableLabel", "String Table"))
						.IsEnabled(this, &SYapTextPropertyEditableTextBox::IsTextLocalizable)
						[
							SNew(SYapTextPropertyEditableStringTableReference, InEditableTextProperty)
							.AllowUnlink(true)
							.Font(PropertyNormalFont)
							.IsEnabled(this, &SYapTextPropertyEditableTextBox::CanEdit)
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.IsHeader(true)
						.Text(LOCTEXT("TextInlineTextLabel", "Inline Text"))
					]

#if USE_STABLE_LOCALIZATION_KEYS
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.Text(LOCTEXT("TextPackageLabel", "Package"))
						.IsEnabled(this, &SYapTextPropertyEditableTextBox::IsTextLocalizable)
						[
							SNew(SEditableTextBox)
							.Text(this, &SYapTextPropertyEditableTextBox::GetPackageValue)
							.Font(PropertyNormalFont)
							.IsReadOnly(true)
						]
					]
#endif // USE_STABLE_LOCALIZATION_KEYS

					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.Text(LOCTEXT("TextNamespaceLabel", "Namespace"))
						.IsEnabled(this, &SYapTextPropertyEditableTextBox::IsTextLocalizable)
						[
							SAssignNew(NamespaceEditableTextBox, SEditableTextBox)
							.Text(this, &SYapTextPropertyEditableTextBox::GetNamespaceValue)
							.Font(PropertyNormalFont)
							.SelectAllTextWhenFocused(true)
							.ClearKeyboardFocusOnCommit(false)
							.OnTextChanged(this, &SYapTextPropertyEditableTextBox::OnNamespaceChanged)
							.OnTextCommitted(this, &SYapTextPropertyEditableTextBox::OnNamespaceCommitted)
							.SelectAllTextOnCommit(true)
							.IsReadOnly(this, &SYapTextPropertyEditableTextBox::IsIdentityReadOnly)
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
						.Text(LOCTEXT("TextKeyLabel", "Key"))
						.IsEnabled(this, &SYapTextPropertyEditableTextBox::IsTextLocalizable)
						[
							SAssignNew(KeyEditableTextBox, SEditableTextBox)
							.Text(this, &SYapTextPropertyEditableTextBox::GetKeyValue)
							.Font(PropertyNormalFont)
#if USE_STABLE_LOCALIZATION_KEYS
							.SelectAllTextWhenFocused(true)
							.ClearKeyboardFocusOnCommit(false)
							.OnTextChanged(this, &SYapTextPropertyEditableTextBox::OnKeyChanged)
							.OnTextCommitted(this, &SYapTextPropertyEditableTextBox::OnKeyCommitted)
							.SelectAllTextOnCommit(true)
							//.IsReadOnly(this, &SYapTextPropertyEditableTextBox::IsIdentityReadOnly)
							.IsReadOnly(true) // TODO project setting to enable editing of Yap text keys
#else	// USE_STABLE_LOCALIZATION_KEYS
							.IsReadOnly(true)
#endif	// USE_STABLE_LOCALIZATION_KEYS
						]
					]
				]
			]
		];

	SetEnabled(TAttribute<bool>(this, &SYapTextPropertyEditableTextBox::CanEdit));
}

bool SYapTextPropertyEditableTextBox::IsTextLocalizable() const
{
	// All text need !IsCultureInvariant()
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 Index = 0; Index < NumTexts; ++Index)
	{
		const FText PropertyValue = EditableTextProperty->GetText(Index);
		if (PropertyValue.IsCultureInvariant())
		{
			return false;
		}
	}
	return true;
}

void SYapTextPropertyEditableTextBox::GetDesiredWidth(float& OutMinDesiredWidth, float& OutMaxDesiredWidth)
{
	if (bIsMultiLine)
	{
		OutMinDesiredWidth = 250.0f;
	}
	else
	{
		OutMinDesiredWidth = 125.0f;
	}

	OutMaxDesiredWidth = 600.0f;
}

bool SYapTextPropertyEditableTextBox::SupportsKeyboardFocus() const
{
	return PrimaryWidget.IsValid() && PrimaryWidget->SupportsKeyboardFocus();
}

FReply SYapTextPropertyEditableTextBox::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	// Forward keyboard focus to our editable text widget
	return FReply::Handled().SetUserFocus(PrimaryWidget.ToSharedRef(), InFocusEvent.GetCause());
}

bool SYapTextPropertyEditableTextBox::CanEdit() const
{
	const bool bIsReadOnly = FTextLocalizationManager::Get().IsLocalizationLocked() || EditableTextProperty->IsReadOnly();
	return !bIsReadOnly;
}

bool SYapTextPropertyEditableTextBox::IsCultureInvariantFlagEnabled() const
{
	return !IsSourceTextReadOnly();
}

bool SYapTextPropertyEditableTextBox::IsSourceTextReadOnly() const
{
	if (!CanEdit())
	{
		return true;
	}

	// We can't edit the source string of string table references
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
	{
		const FText TextValue = EditableTextProperty->GetText(TextIndex);
		if (TextValue.IsFromStringTable())
		{
			return true;
		}
	}

	return false;
}

bool SYapTextPropertyEditableTextBox::IsIdentityReadOnly() const
{
	if (!CanEdit())
	{
		return true;
	}

	// We can't edit the identity of texts that don't gather for localization
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
	{
		const FText TextValue = EditableTextProperty->GetText(TextIndex);
		if (!TextValue.ShouldGatherForLocalization())
		{
			return true;
		}
	}

	return false;
}

FText SYapTextPropertyEditableTextBox::GetToolTipText() const
{
	FText LocalizedTextToolTip;
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText TextValue = EditableTextProperty->GetText(0);

		if (TextValue.IsFromStringTable())
		{
			FName TableId;
			FString Key;
			FTextInspector::GetTableIdAndKey(TextValue, TableId, Key);

			LocalizedTextToolTip = FText::Format(
				LOCTEXT("StringTableTextToolTipFmt", "--- String Table Reference ---\nTable ID: {0}\nKey: {1}"), 
				FText::FromName(TableId), FText::FromString(Key)
				);
		}
		else
		{
			FTextId TextId;
			const FString* SourceString = FTextInspector::GetSourceString(TextValue);

			if (SourceString && TextValue.ShouldGatherForLocalization())
			{
				TextId = FTextInspector::GetTextId(TextValue);
			}

			if (!TextId.IsEmpty())
			{
				check(SourceString);

				
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
				const FString Namespace = TextId.GetNamespace().GetChars();
				const FString Key = TextId.GetKey().GetChars();
#else
				const FString Namespace = TextId.GetNamespace().ToString();
				const FString Key = TextId.GetKey().ToString();
#endif
				
				const FString PackageNamespace = TextNamespaceUtil::ExtractPackageNamespace(Namespace);
				const FString TextNamespace = TextNamespaceUtil::StripPackageNamespace(Namespace);

				FFormatNamedArguments LocalizedTextToolTipArgs;
				LocalizedTextToolTipArgs.Add(TEXT("Package"), FText::FromString(PackageNamespace));
				LocalizedTextToolTipArgs.Add(TEXT("Namespace"), FText::FromString(TextNamespace));
				LocalizedTextToolTipArgs.Add(TEXT("Key"), FText::FromString(Key));
				LocalizedTextToolTipArgs.Add(TEXT("Source"), FText::FromString(*SourceString));
				LocalizedTextToolTipArgs.Add(TEXT("Display"), TextValue);

				if (SourceString->Equals(TextValue.ToString(), ESearchCase::CaseSensitive))
				{
					LocalizedTextToolTip = FText::Format(
						LOCTEXT("LocalizedTextNoDisplayToolTipFmt", "--- Localized Text ---\nPackage: {Package}\nNamespace: {Namespace}\nKey: {Key}\nSource: {Source}"),
						LocalizedTextToolTipArgs
						);
				}
				else
				{
					LocalizedTextToolTip = FText::Format(
						LOCTEXT("LocalizedTextWithDisplayToolTipFmt", "--- Localized Text ---\nPackage: {Package}\nNamespace: {Namespace}\nKey: {Key}\nSource: {Source}\nDisplay: {Display}"),
						LocalizedTextToolTipArgs
						);
				}
			}
		}
	}
	
	FText BaseToolTipText = EditableTextProperty->GetToolTipText();
	if (FTextLocalizationManager::Get().IsLocalizationLocked())
	{
		const FText LockdownToolTip = FTextLocalizationManager::Get().IsGameLocalizationPreviewEnabled() 
			? LOCTEXT("LockdownToolTip_Preview", "Localization is locked down due to the active game localization preview")
			: LOCTEXT("LockdownToolTip_Other", "Localization is locked down");
		BaseToolTipText = BaseToolTipText.IsEmptyOrWhitespace() ? LockdownToolTip : FText::Format(LOCTEXT("ToolTipLockdownFmt", "!!! {0} !!!\n\n{1}"), LockdownToolTip, BaseToolTipText);
	}

	if (LocalizedTextToolTip.IsEmptyOrWhitespace())
	{
		return BaseToolTipText;
	}
	if (BaseToolTipText.IsEmptyOrWhitespace())
	{
		return LocalizedTextToolTip;
	}

	return FText::Format(FText::AsCultureInvariant(TEXT("{0}\n\n{1}")), BaseToolTipText, LocalizedTextToolTip);
}

FText SYapTextPropertyEditableTextBox::GetTextValue() const
{
	FText TextValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		TextValue = EditableTextProperty->GetText(0);

		if (const FString* SourceString = FTextInspector::GetSourceString(TextValue);
			SourceString && !FTextLocalizationManager::Get().IsLocalizationLocked())
		{
			// We should always edit the source string, but if the source string matches the current 
			// display string then we can avoid making a temporary text from the source string
			if (!SourceString->IsEmpty() && !SourceString->Equals(TextValue.ToString(), ESearchCase::CaseSensitive))
			{
				TextValue = FText::AsCultureInvariant(*SourceString);
			}
		}
	}
	else if (NumTexts > 1)
	{
		TextValue = MultipleValuesText;
	}

	return TextValue;
}

void SYapTextPropertyEditableTextBox::OnTextChanged(const FText& NewText)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	FText TextErrorMsg;

	// Don't validate the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString().Equals(MultipleValuesText.ToString(), ESearchCase::CaseSensitive)))
	{
		EditableTextProperty->IsValidText(NewText, TextErrorMsg);
	}

	// Update or clear the error message
	SetTextError(TextErrorMsg);
}

void SYapTextPropertyEditableTextBox::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || !NewText.ToString().Equals(MultipleValuesText.ToString(), ESearchCase::CaseSensitive)))
	{
		FText TextErrorMsg;
		if (EditableTextProperty->IsValidText(NewText, TextErrorMsg))
		{
			// Valid text; clear any error
			SetTextError(FText::GetEmpty());
		}
		else
		{
			// Invalid text; set the error and prevent the new text from being set
			SetTextError(TextErrorMsg);
			return;
		}

		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new text is different
			if (PropertyValue.ToString().Equals(NewText.ToString(), ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Is the new text is empty, just use the empty instance
			if (NewText.IsEmpty())
			{
				EditableTextProperty->SetText(TextIndex, FText::GetEmpty());
				continue;
			}

			// Maintain culture invariance when editing the text
			if (PropertyValue.IsCultureInvariant())
			{
				EditableTextProperty->SetText(TextIndex, FText::AsCultureInvariant(NewText.ToString()));
				continue;
			}

			FString NewNamespace;
			FString NewKey;
#if USE_STABLE_LOCALIZATION_KEYS
			{
				// Get the stable namespace and key that we should use for this property
				const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
				EditableTextProperty->GetStableTextId(
					TextIndex, 
					IEditableTextProperty::ETextPropertyEditAction::EditedSource, 
					TextSource ? *TextSource : FString(), 
					FTextInspector::GetNamespace(PropertyValue).Get(FString()), 
					FTextInspector::GetKey(PropertyValue).Get(FString()), 
					NewNamespace, 
					NewKey
					);
			}
#else	// USE_STABLE_LOCALIZATION_KEYS
			{
				// We want to preserve the namespace set on this property if it's *not* the default value
				if (!EditableTextProperty->IsDefaultValue())
				{
					// Some properties report that they're not the default, but still haven't been set from a property, so we also check the property key to see if it's a valid GUID before allowing the namespace to persist
					FGuid TmpGuid;
					if (FGuid::Parse(FTextInspector::GetKey(PropertyValue).Get(FString()), TmpGuid))
					{
						NewNamespace = FTextInspector::GetNamespace(PropertyValue).Get(FString());
					}
				}

				NewKey = FGuid::NewGuid().ToString();
			}
#endif	// USE_STABLE_LOCALIZATION_KEYS

			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, NewText));
		}
	}
}

void SYapTextPropertyEditableTextBox::SetTextError(const FText& InErrorMsg)
{
	if (MultiLineWidget.IsValid())
	{
		MultiLineWidget->SetError(InErrorMsg);
	}

	if (SingleLineWidget.IsValid())
	{
		SingleLineWidget->SetError(InErrorMsg);
	}
}

FText SYapTextPropertyEditableTextBox::GetNamespaceValue() const
{
	FText NamespaceValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundNamespace = FTextInspector::GetNamespace(PropertyValue);
		if (FoundNamespace.IsSet())
		{
			NamespaceValue = FText::FromString(TextNamespaceUtil::StripPackageNamespace(FoundNamespace.GetValue()));
		}
	}
	else if (NumTexts > 1)
	{
		NamespaceValue = MultipleValuesText;
	}

	return NamespaceValue;
}

void SYapTextPropertyEditableTextBox::OnNamespaceChanged(const FText& NewText)
{
	FText ErrorMessage;
	const FText ErrorCtx = LOCTEXT("TextNamespaceErrorCtx", "Namespace");
	IsValidIdentity(NewText, &ErrorMessage, &ErrorCtx);

	NamespaceEditableTextBox->SetError(ErrorMessage);
}

void SYapTextPropertyEditableTextBox::OnNamespaceCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	if (!IsValidIdentity(NewText))
	{
		return;
	}

	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString() != MultipleValuesText.ToString()))
	{
		const FString& TextNamespace = NewText.ToString();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new namespace is different - we want to keep the keys stable where possible
			const FString CurrentTextNamespace = TextNamespaceUtil::StripPackageNamespace(FTextInspector::GetNamespace(PropertyValue).Get(FString()));
			if (CurrentTextNamespace.Equals(TextNamespace, ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Get the stable namespace and key that we should use for this property
			FString NewNamespace;
			FString NewKey;
#if USE_STABLE_LOCALIZATION_KEYS
			{
				const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
				EditableTextProperty->GetStableTextId(
					TextIndex, 
					IEditableTextProperty::ETextPropertyEditAction::EditedNamespace, 
					TextSource ? *TextSource : FString(), 
					TextNamespace, 
					FTextInspector::GetKey(PropertyValue).Get(FString()), 
					NewNamespace, 
					NewKey
					);
			}
#else	// USE_STABLE_LOCALIZATION_KEYS
			{
				NewNamespace = TextNamespace;

				// If the current key is a GUID, then we can preserve that when setting the new namespace
				NewKey = FTextInspector::GetKey(PropertyValue).Get(FString());
				{
					FGuid TmpGuid;
					if (!FGuid::Parse(NewKey, TmpGuid))
					{
						NewKey = FGuid::NewGuid().ToString();
					}
				}
			}
#endif	// USE_STABLE_LOCALIZATION_KEYS

			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, PropertyValue));
		}
	}
}

FText SYapTextPropertyEditableTextBox::GetKeyValue() const
{
	FText KeyValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundKey = FTextInspector::GetKey(PropertyValue);
		if (FoundKey.IsSet())
		{
			KeyValue = FText::FromString(FoundKey.GetValue());
		}
	}
	else if (NumTexts > 1)
	{
		KeyValue = MultipleValuesText;
	}

	return KeyValue;
}

#if USE_STABLE_LOCALIZATION_KEYS

void SYapTextPropertyEditableTextBox::OnKeyChanged(const FText& NewText)
{
	FText ErrorMessage;
	const FText ErrorCtx = LOCTEXT("TextKeyErrorCtx", "Key");
	const bool bIsValidName = IsValidIdentity(NewText, &ErrorMessage, &ErrorCtx);

	if (NewText.IsEmptyOrWhitespace())
	{
		ErrorMessage = LOCTEXT("TextKeyEmptyErrorMsg", "Key cannot be empty so a new key will be assigned");
	}
	else if (bIsValidName)
	{
		// Valid name, so check it won't cause an identity conflict (only test if we have a single text selected to avoid confusion)
		const int32 NumTexts = EditableTextProperty->GetNumTexts();
		if (NumTexts == 1)
		{
			const FText PropertyValue = EditableTextProperty->GetText(0);

			const FString TextNamespace = FTextInspector::GetNamespace(PropertyValue).Get(FString());
			const FString TextKey = NewText.ToString();

			// Get the stable namespace and key that we should use for this property
			// If it comes back with the same namespace but a different key then it means there was an identity conflict
			FString NewNamespace;
			FString NewKey;
			const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
			EditableTextProperty->GetStableTextId(
				0,
				IEditableTextProperty::ETextPropertyEditAction::EditedKey,
				TextSource ? *TextSource : FString(),
				TextNamespace,
				TextKey,
				NewNamespace,
				NewKey
				);

			if (TextNamespace.Equals(NewNamespace, ESearchCase::CaseSensitive) && !TextKey.Equals(NewKey, ESearchCase::CaseSensitive))
			{
				ErrorMessage = LOCTEXT("TextKeyConflictErrorMsg", "Identity (namespace & key) is being used by a different text within this package so a new key will be assigned");
			}
		}
	}

	KeyEditableTextBox->SetError(ErrorMessage);
}

void SYapTextPropertyEditableTextBox::OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	if (!IsValidIdentity(NewText))
	{
		return;
	}

	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString() != MultipleValuesText.ToString()))
	{
		const FString& TextKey = NewText.ToString();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new key is different - we want to keep the keys stable where possible
			const FString CurrentTextKey = FTextInspector::GetKey(PropertyValue).Get(FString());
			if (CurrentTextKey.Equals(TextKey, ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Get the stable namespace and key that we should use for this property
			FString NewNamespace;
			FString NewKey;
			const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
			EditableTextProperty->GetStableTextId(
				TextIndex, 
				IEditableTextProperty::ETextPropertyEditAction::EditedKey, 
				TextSource ? *TextSource : FString(), 
				FTextInspector::GetNamespace(PropertyValue).Get(FString()), 
				TextKey, 
				NewNamespace, 
				NewKey
				);

			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, PropertyValue));
		}
	}
}

FText SYapTextPropertyEditableTextBox::GetPackageValue() const
{
	FText PackageValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundNamespace = FTextInspector::GetNamespace(PropertyValue);
		if (FoundNamespace.IsSet())
		{
			PackageValue = FText::FromString(TextNamespaceUtil::ExtractPackageNamespace(FoundNamespace.GetValue()));
		}
	}
	else if (NumTexts > 1)
	{
		PackageValue = MultipleValuesText;
	}

	return PackageValue;
}

#endif // USE_STABLE_LOCALIZATION_KEYS

ECheckBoxState SYapTextPropertyEditableTextBox::GetLocalizableCheckState() const
{
	TOptional<ECheckBoxState> Result;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	for (int32 Index = 0; Index < NumTexts; ++Index)
	{
		const FText PropertyValue = EditableTextProperty->GetText(Index);

		const bool bIsLocalized = !PropertyValue.IsCultureInvariant();
		ECheckBoxState NewState = bIsLocalized ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		if (NewState != Result.Get(NewState))
		{
			return ECheckBoxState::Undetermined;
		}
		Result = NewState;
	}

	return Result.Get(ECheckBoxState::Unchecked);
}

void SYapTextPropertyEditableTextBox::HandleLocalizableCheckStateChanged(ECheckBoxState InCheckboxState)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	if (InCheckboxState == ECheckBoxState::Checked)
	{
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Assign a key to any currently culture invariant texts
			if (PropertyValue.IsCultureInvariant())
			{
				// Get the stable namespace and key that we should use for this property
				FString NewNamespace;
				FString NewKey;
				EditableTextProperty->GetStableTextId(
					TextIndex,
					IEditableTextProperty::ETextPropertyEditAction::EditedKey,
					PropertyValue.ToString(),
					FString(),
					FString(),
					NewNamespace,
					NewKey
					);

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
				EditableTextProperty->SetText(TextIndex, FInternationalization::Get().ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*PropertyValue.ToString(), *NewNamespace, *NewKey));
#else
				EditableTextProperty->SetText(TextIndex, FText::AsLocalizable_Advanced(*NewNamespace, *NewKey, *PropertyValue.ToString()));
#endif
			}
		}
	}
	else
	{
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Clear the identity from any non-culture invariant texts
			if (!PropertyValue.IsCultureInvariant())
			{
				EditableTextProperty->SetText(TextIndex, FText::AsCultureInvariant(PropertyValue.ToString()));
			}
		}
	}
}

FText SYapTextPropertyEditableTextBox::GetAdvancedTextSettingsComboToolTip() const
{
	if (IsTextLocalizable())
	{
		return LOCTEXT("AdvancedTextSettingsComboToolTip", "Edit advanced text settings.");
	}
	else
	{
		return LOCTEXT("TextNotLocalizedWarningToolTip", "This text is marked as 'culture invariant' and won't be gathered for localization.\nYou can change this by editing the advanced text settings.");
	}
}

const FSlateBrush* SYapTextPropertyEditableTextBox::GetAdvancedTextSettingsComboImage() const
{
	if (IsTextLocalizable())
	{
		return FAppStyle::Get().GetBrush("LocalizationDashboard.MenuIcon");
	}
	else
	{
		return FCoreStyle::Get().GetBrush("Icons.Warning");
	}
}

bool SYapTextPropertyEditableTextBox::IsValidIdentity(const FText& InIdentity, FText* OutReason, const FText* InErrorCtx) const
{
	const FString InvalidIdentityChars = FString::Printf(TEXT("%s%c%c"), INVALID_NAME_CHARACTERS, TextNamespaceUtil::PackageNamespaceStartMarker, TextNamespaceUtil::PackageNamespaceEndMarker);
	return FName::IsValidXName(InIdentity.ToString(), InvalidIdentityChars, OutReason, InErrorCtx);
}

#undef LOCTEXT_NAMESPACE
