// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapCharacterSelectWidget.h"

#include "GameplayTagContainer.h"
#include "SYapHeadingBlock.h"
#include "Filters/SFilterSearchBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Yap/YapCharacterDefinition.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapFragment.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TArray<FName> SYapCharacterSelectWidget::RecentlySelectedCharacterTags;

void SYapCharacterSelectWidget::OnTextChanged_CharacterSelectorFilter(const FText& NewFilterText)
{
	UpdateCharacterSelector(NewFilterText);
}

void SYapCharacterSelectWidget::OnTextCommitted_CharacterSelectorFilter(const FText& Text, ETextCommit::Type Arg)
{
	if (FilteredCharacterTag.IsValid())
	{
		SelectCharacter(FilteredCharacterTag);
	}
}

void SYapCharacterSelectWidget::Construct(const FArguments& InArgs)
{
	DialogueNode = InArgs._DialogueNode;
	FragmentIndex = InArgs._FragmentIndex;
	SetCharacterFunction = InArgs._SetCharacterFunction;
	CurrentCharacterTag = InArgs._CurrentCharacterTag;

	CharacterList = SNew(SScrollBox)
	//.ScrollBarAlwaysVisible(true)
	.ScrollBarPadding(FMargin(4, 4, 4, 4))
	.Orientation(Orient_Vertical)
	.AnimateWheelScrolling(true)
	.WheelScrollMultiplier(2.0f);

	FilterSearchBox = SNew(SFilterSearchBox)
	.OnTextChanged(this, &SYapCharacterSelectWidget::OnTextChanged_CharacterSelectorFilter)
	.OnTextCommitted(this, &SYapCharacterSelectWidget::OnTextCommitted_CharacterSelectorFilter);
	
	UpdateCharacterSelector(FText::GetEmpty());

	auto OnClicked_Clear = [this] () -> FReply
	{
		SelectCharacter(FGameplayTag::EmptyTag);
		
		return FReply::Handled();
	};

	TSharedPtr<SVerticalBox> VerticalBox;
	
	TSharedRef<SBorder> Widget = SNew(SBorder)
	.Padding(16, 16, 16, 16)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SAssignNew(VerticalBox, SVerticalBox)
		]
	];

	if (CurrentCharacterTag.IsValid())
	{
		VerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SYapHeadingBlock)
			.HeadingText(INVTEXT("Current Character"))
		];
		
		VerticalBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(12, 0, 12, 0))
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>( "Menu.Button" ))
			.OnClicked_Lambda(OnClicked_Clear)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0, 0, 8, 0)
				.AutoWidth()
				[
					SNew(SBox)
					[
						SNew(SImage)
						.Image(FAppStyle::Get().GetBrush("GenericCommands.Delete"))
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CharacterSelector_ClearButtonLabel", "Clear"))
				]
			]
		];
	}
	
	VerticalBox->AddSlot()
	.AutoHeight()
	.Padding(0, 8, 0, 0)
	[
		SNew(SYapHeadingBlock)
		.HeadingText(INVTEXT("Browse Characters"))
	];
	
	VerticalBox->AddSlot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		FilterSearchBox.ToSharedRef()
	];
	
	VerticalBox->AddSlot()
	[
		SNew(SBorder)
		.Padding(4)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		.BorderBackgroundColor(YapColor::DarkGray)
		[
			SNew(SBox)
			.MinDesiredWidth(300)
			.MaxDesiredWidth(400)
			.MaxDesiredHeight(500)
			[
				CharacterList.ToSharedRef()
			]
		]
	];
	
	ChildSlot
	[
		Widget
	];
}

FYapFragment& SYapCharacterSelectWidget::GetFragment()
{
	return DialogueNode.Get()->GetFragmentMutableByIndex(FragmentIndex);
}

void SYapCharacterSelectWidget::UpdateRecentlySelectedCharacterTags(const FGameplayTag& CharacterTag)
{
	if (!CharacterTag.IsValid())
	{
		return;
	}
	
	RecentlySelectedCharacterTags.Remove(CharacterTag.GetTagName());

	RecentlySelectedCharacterTags.Insert(CharacterTag.GetTagName(), 0);

	while (RecentlySelectedCharacterTags.Num() > 3)
	{
		RecentlySelectedCharacterTags.Pop();
	}
}

void SYapCharacterSelectWidget::UpdateCharacterSelector(const FText& FilterText)
{
	UYapProjectSettings::UpdateReversedCharacterMap();

	const float PortraitSize = 32;
	const float BorderSize = 4;
	
	FYapFragment& Fragment = GetFragment();

	FGameplayTag MoodTag = Fragment.GetMoodTag();

	CharacterList->ClearChildren();

	int32 NumCharactersAdded = 0;
	FGameplayTag LastCharacterTag;

	auto CreateCharacterListEntry = [this, MoodTag, PortraitSize, BorderSize, FilterText, &NumCharactersAdded, &LastCharacterTag] (const FYapCharacterDefinition& CharacterDefinition)
	{
		const UObject* Character = CharacterDefinition.CharacterAsset.LoadSynchronous();
		
		FText Name = FText::GetEmpty();
		
		if (IsValid(Character))
		{
			const UObject* Target = Character;
			
			if (const UBlueprint* Blueprint = Cast<UBlueprint>(Character))
			{
				Target = Blueprint->GeneratedClass.GetDefaultObject();
			}

			if (const IYapCharacterInterface* Interface = Cast<IYapCharacterInterface>(Target))
			{
				Name = Interface->GetName(Target);
			}
			else
			{
				Name = IYapCharacterInterface::GetName(Target);
			}
		}
		
		if (!FilterText.IsEmpty())
		{
			if (!Name.ToString().Contains(FilterText.ToString()))
			{
				return;
			}
		}
		
		auto CharacterImage = [this, MoodTag, Character] () -> const FSlateBrush*
		{
			TSharedPtr<FSlateImageBrush> PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(Character, MoodTag);

			if (PortraitBrush && PortraitBrush->GetResourceObject())
			{
				return PortraitBrush.Get();
			}
			else
			{
				return FYapEditorStyle::GetImageBrush(YapBrushes.None);
			}
		};

		auto CharacterColor = [Character] () -> const FLinearColor
		{
			FLinearColor Color = YapColor::Black;

			if (IsValid(Character))
			{
				const UObject* Target = Character;
			
				if (const UBlueprint* Blueprint = Cast<UBlueprint>(Character))
				{
					Target = Blueprint->GeneratedClass.GetDefaultObject();
				}

				if (const IYapCharacterInterface* Interface = Cast<IYapCharacterInterface>(Target))
				{
					Color = Interface->GetColor(Target);
				}
				else
				{
					Color = IYapCharacterInterface::GetColor(Target);
				}
			}
	
			Color.A *= UYapDeveloperSettings::GetPortraitBorderAlpha();

			return Color;
		};

		auto OnClicked_CharacterSelect = [this] (FGameplayTag NewCharacterTag) -> FReply
		{
			FYapScopedTransaction Transaction(NAME_None, LOCTEXT("SelectCharactr", "Select Character"), GetDialogueNode());

			if (SetCharacterFunction.IsBound())
			{
				SelectCharacter(NewCharacterTag);
			}
			else
			{
				UE_LOG(LogYapEditor, Error, TEXT("Missing SetCharacterFunction!"));
			}
			
			
			return FReply::Handled();
		};
		
		CharacterList->AddSlot()
		.Padding(0, 0, 0, 2)
		[
			SNew(SButton)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_CharacterSelect)
			.ButtonColorAndOpacity(YapColor::DarkGray)
			.ContentPadding(4)
			.OnClicked_Lambda(OnClicked_CharacterSelect, CharacterDefinition.CharacterTag)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(0.5 * BorderSize)
					[
						SNew(SColorBlock)
						.Color(YapColor::Noir)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
						.BorderBackgroundColor_Lambda(CharacterColor)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(BorderSize)
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
						.Image_Lambda(CharacterImage)
					]
				]
				+ SHorizontalBox::Slot()
				.Padding(8, 0, 8, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 8)
					[
						SNew(STextBlock)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_CharacterName)
						.Text(Name)
					]
					+ SVerticalBox::Slot()
					[
						SNew(STextBlock)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_CharacterTag)
						.Text(FText::FromString(CharacterDefinition.CharacterTag.ToString()))	
					]
				]
			]
		];
		
		++NumCharactersAdded;
		LastCharacterTag = CharacterDefinition.CharacterTag;
	};

	if (RecentlySelectedCharacterTags.Num() > 0)
	{
		if (FilterText.IsEmpty())
		{
			CharacterList->AddSlot()
			[
				SNew(SYapHeadingBlock)
				.HeadingText(LOCTEXT("CharacterSelector_RecentlySelectedHeading", "Recently Selected"))
			];
		}
		
		for (const FYapCharacterDefinition& CharacterDefinition : UYapProjectSettings::GetCharacterDefinitions())
		{
			if (RecentlySelectedCharacterTags.Contains(CharacterDefinition.CharacterTag.GetTagName()))
			{
				CreateCharacterListEntry(CharacterDefinition);
			}
		}
	}
	
	if (FilterText.IsEmpty())
	{
		CharacterList->AddSlot()
		[
			SNew(SYapHeadingBlock)
			.HeadingText(LOCTEXT("CharacterSelector_ListStartHeading", "All Characters"))
		];
	}

	for (const FYapCharacterDefinition& CharacterDefinition : UYapProjectSettings::GetCharacterDefinitions())
	{
		if (!RecentlySelectedCharacterTags.Contains(CharacterDefinition.CharacterTag.GetTagName()))
		{
			CreateCharacterListEntry(CharacterDefinition);
		}
	}

	if (NumCharactersAdded == 1)
	{
		FilteredCharacterTag = LastCharacterTag;
		
		CharacterList->AddSlot()
		.HAlign(HAlign_Center)
		.Padding(0, 8, 0, 8)
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_CharacterTag)
			.Text(LOCTEXT("CharacterSelector_PressEnterToSelect", "Press Enter to Select"))
			.ColorAndOpacity(YapColor::LightGreen)
		];
	}
	else
	{
		FilteredCharacterTag = FGameplayTag::EmptyTag;
	}
}

UFlowNode_YapDialogue* SYapCharacterSelectWidget::GetDialogueNode() const
{
	return DialogueNode.Get();
}

void SYapCharacterSelectWidget::SelectCharacter(const FGameplayTag& NewCharacterTag)
{
	FYapScopedTransaction Transaction(NAME_None, LOCTEXT("CharacterSelector_SetCharacterTransaction", "Set Character"), DialogueNode.Get());

	SetCharacterFunction.Execute(NewCharacterTag);

	FSlateApplication::Get().ClearAllUserFocus();
		
	UpdateRecentlySelectedCharacterTags(NewCharacterTag);
}

FReply SYapCharacterSelectWidget::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	FSlateApplication::Get().SetKeyboardFocus(FilterSearchBox);
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
