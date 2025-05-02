// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapDialogueEditor.h"

#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapErrorLevel.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Enums/YapMaturitySetting.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "YapEditor/Helpers/YapEditableTextPropertyHandle.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"
#include "YapEditor/SlateWidgets/SYapTextPropertyEditableTextBox.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TMap<EYapTimeMode, FLinearColor> SYapDialogueEditor::TimeModeButtonColors =
	{
	{ EYapTimeMode::None, YapColor::Red },
	{ EYapTimeMode::Default, YapColor::Green },
	{ EYapTimeMode::AudioTime, YapColor::Cyan },
	{ EYapTimeMode::TextTime, YapColor::LightBlue },
	{ EYapTimeMode::ManualTime, YapColor::Orange },
};

// ------------------------------------------------------------------------------------------------

void SYapDialogueEditor::Construct(const FArguments& InArgs)
{
	DialogueNode = InArgs._DialogueNodeIn;
	FragmentIndex = InArgs._FragmentIndexIn;
	bNeedsChildSafe = InArgs._bNeedsChildSafeIn;
	Owner = InArgs._OwnerIn;
	
    float Width = bNeedsChildSafe ? 500 : 600;
    
    Width += UYapDeveloperSettings::GetExpandedEditorWidthAdjustment();
    
    ChildSlot
    [
        SNew(SSplitter)
        .Orientation(Orient_Vertical)
        .PhysicalSplitterHandleSize(2.0)
        + SSplitter::Slot()
        .Resizable(false)
        .SizeRule(SSplitter::SizeToContent)
        [
            SNew(SBox)
            .Padding(0, 0, 0, 4)
            [
                BuildDialogueEditors_ExpandedEditor(Width)
            ]
        ]
        + SSplitter::Slot()
        .Resizable(false)
        .SizeRule(SSplitter::SizeToContent)
        [
            SNew(SBox)
            .Padding(0, 4, 0, 4)
            [
                BuildTimeSettings_ExpandedEditor(Width)
            ]
        ]
        + SSplitter::Slot()
        .Resizable(false)
        .SizeRule(SSplitter::SizeToContent)
        [
            SNew(SBox)
            .Padding(0, 4, 0, 0)
            [
                BuildPaddingSettings_ExpandedEditor(Width)
            ]
        ]
    ];
}

// ------------------------------------------------------------------------------------------------

// TODO this is all doubled-up for mature and child safe. Simplify it.
TSharedRef<SWidget> SYapDialogueEditor::BuildDialogueEditors_ExpandedEditor(float Width)
{
	bool bMatureOnly = !bNeedsChildSafe;

	TSharedRef<SSplitter> Splitter = SNew(SSplitter)
	.Orientation(Orient_Horizontal)
	.PhysicalSplitterHandleSize(2.0f);
	
	{
		FYapBit& Bit = GetFragment().GetMatureBitMutable();

		FText Title = (bMatureOnly) ? LOCTEXT("DialogueDataEditor_Title", "DIALOGUE") : LOCTEXT("MatureDialogueDataEditor_Title", "MATURE DIALOGUE");
		FText DialogueTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Dialogue text...");
		FText TitleTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Title text...");
		
		FMargin Padding(0, 4, 4, 4);
		
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildDialogueEditor_SingleSide(Title, DialogueTextHint, TitleTextHint, Width, Padding, Bit, MatureDialogueText, MatureTitleText)
		];
	}
	
	if (!bMatureOnly)
	{
		FYapBit& Bit = GetFragment().GetChildSafeBitMutable();

		FText Title = LOCTEXT("ChildSafeDataEditor_Title", "CHILD-SAFE DIALOGUE");
		FText DialogueTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Dialogue text (child-safe)...");
		FText TitleTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Title text (child-safe)...");

		FMargin Padding(4, 4, 0, 4);
				
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildDialogueEditor_SingleSide(Title, DialogueTextHint, TitleTextHint, Width, Padding, Bit, ChildSafeDialogueText, ChildSafeTitleText)
		];
	};

	return Splitter;
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::BuildDialogueEditor_SingleSide(const FText& Title, const FText& DialogueTextHint, const FText& TitleTextHint, float Width, FMargin Padding, FYapBit& Bit, TSharedPtr<SWidget>& DialogueTextWidget, TSharedPtr<SWidget>& TitleTextWidget)
{
	FYapText& DialogueText = Bit.DialogueText;
	FYapText& TitleText = Bit.TitleText;
		
	TSoftObjectPtr<UObject>& AudioAsset = Bit.AudioAsset;
	FString& StageDirections = Bit.StageDirections;

	FString& DialogueLocalizationComments = Bit.DialogueLocalizationComments;
	FString& TitleTextLocalizationComments = Bit.TitleTextLocalizationComments;

	TSharedRef<IEditableTextProperty> DialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(DialogueText, Cast<UFlowGraphNode_YapDialogue>(DialogueNode->GetGraphNode())));
	TSharedRef<IEditableTextProperty> TitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(TitleText, Cast<UFlowGraphNode_YapDialogue>(DialogueNode->GetGraphNode())));

	auto DialogueCommentAttribute = TAttribute<FString>::CreateLambda( [DialogueLocalizationComments] () { return *DialogueLocalizationComments; });
	FText DialogueCommentText = LOCTEXT("POComment_HintText", "Comments for translators...");
	
	auto TitleTextCommentAttribute = TAttribute<FString>::CreateLambda( [TitleTextLocalizationComments] () { return *TitleTextLocalizationComments; });
	FText TitleTextCommentText = LOCTEXT("POComment_HintText", "Comments for translators...");
	
	auto StageDirectionsAttribute = TAttribute<FString>::CreateLambda( [StageDirections] () { return *StageDirections; });
	FText StageDirectionsText = LOCTEXT("StageDirections_HintText", "Stage directions...");

	return SNew(SBox)
	.WidthOverride(Width)
	.Padding(Padding)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(YapFonts.Font_SectionHeader)
			.Justification(ETextJustify::Center)	
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0)
		.Padding(0, 6, 0, 0)
		[
			SNew(SBox)
			.HeightOverride(66) // Fits ~4 lines of text
			.VAlign(VAlign_Fill)
			[
				SAssignNew(DialogueTextWidget, SYapTextPropertyEditableTextBox, DialogueTextProperty)
				.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
				.Owner(DialogueNode)
				.HintText(DialogueTextHint)
				.Font(YapFonts.Font_DialogueText)
				.ForegroundColor(YapColor::White)
				.Cursor(EMouseCursor::Default)
				.MaxDesiredHeight(66)
				.AutoWrapText_Lambda( [] () { return UYapDeveloperSettings::GetWrapExpandedEditorText(); } )
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2, 0, 0)
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible) // TODO project settings to disable localization metadata
			.Padding(0, 0, 28, 0)
			[
					BuildCommentEditor(DialogueCommentAttribute, &DialogueLocalizationComments, LOCTEXT("POComment_HintText", "Comments for translators..."))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 12, 0, 0)
		[
			SNew(SVerticalBox)
			.Visibility(DialogueNode->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
			+ SVerticalBox::Slot()
			[
				SAssignNew(TitleTextWidget, SYapTextPropertyEditableTextBox, TitleTextProperty)
				.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
				.Owner(DialogueNode)
				.HintText(TitleTextHint)
				.ForegroundColor(YapColor::YellowGray)
				.Cursor(EMouseCursor::Default)
				.AutoWrapText_Lambda( [] () { return UYapDeveloperSettings::GetWrapExpandedEditorText(); } )
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2, 0, 0)
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible) // TODO project settings to disable localization metadata
				.Padding(0, 0, 28, 0)
				[
					BuildCommentEditor(TitleTextCommentAttribute, &TitleTextLocalizationComments, LOCTEXT("POComment_HintText", "Comments for translators..."))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 12, 0, 0)
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible) // TODO project settings to disable audio
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 0)
				[							
					SNew(STextBlock)
					.Text(LOCTEXT("AudioAssetPicker_Title", "Audio Asset"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						[
							CreateAudioAssetWidget(AudioAsset)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						CreateAudioPreviewWidget(&AudioAsset, EVisibility::Visible)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 28, 0)
				.AutoHeight()
				[
					BuildCommentEditor(StageDirectionsAttribute, &StageDirections, LOCTEXT("StageDirections_HintText", "Stage directions..."))
				]
			]
		]
	];
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::BuildCommentEditor(TAttribute<FString> String, FString* StringProperty, FText HintText)
{
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
	.BorderBackgroundColor(YapColor::DeepGray_SemiGlass)
	.Visibility_Lambda( [this] () { return EVisibility::Visible; } ) // TODO project developer settings to show/hide this
	[
		SNew(SMultiLineEditableText)
		.HintText(HintText)
		.ClearKeyboardFocusOnCommit(false)
		.Text_Lambda( [String] () { return FText::FromString(String.Get()); } )
		.OnTextCommitted_Lambda( [this, StringProperty] (const FText& NewText, ETextCommit::Type CommitType)
		{
			check(DialogueNode.IsValid());
			
			if (CommitType != ETextCommit::OnCleared)
			{
				FYapScopedTransaction Transaction("TODO", LOCTEXT("TransactionText_ChangeComment", "Change comment"), DialogueNode.Get());
				*StringProperty = NewText.ToString();
			}
		})
	];
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::BuildTimeSettings_ExpandedEditor(float Width)
{
	bool bMatureOnly = !bNeedsChildSafe;

	TSharedRef<SSplitter> Splitter = SNew(SSplitter)
		.Orientation(Orient_Horizontal)
		.PhysicalSplitterHandleSize(2.0f);

	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildTimeSettings_SingleSide(Width, FMargin(4, 4, 28, 4), EYapMaturitySetting::Mature)
		];
	}
	
	if (!bMatureOnly)
	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildTimeSettings_SingleSide(Width, FMargin(4, 4, 28, 4), EYapMaturitySetting::ChildSafe)
		];
	};

	return Splitter;
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::BuildTimeSettings_SingleSide(float Width, FMargin Padding, EYapMaturitySetting MaturitySetting)
{
	return SNew(SBox)
	.WidthOverride(Width)
	.Padding(Padding)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::Default, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::AudioTime, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::TextTime, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 0)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::ManualTime, MaturitySetting)
		]
	];
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::BuildPaddingSettings_ExpandedEditor(float Width)
{
	bool bMatureOnly = !bNeedsChildSafe;

	TSharedRef<SSplitter> Splitter = SNew (SSplitter)
		.Orientation(Orient_Horizontal)
		.PhysicalSplitterHandleSize(2.0f);

	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			SNew(SBox)
			.Padding(4, 4, 28, 4)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PaddingTime_Header", "Padding Time"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[				
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
					.ContentPadding(FMargin(4, 3))
					.ToolTipText(LOCTEXT("UseDefault_Button", "Use Default"))
					.OnClicked_Lambda( [this] ()
					{
						if (!GetFragment().Padding.IsSet())
						{
							TOptional<float> CurrentPadding_Default = GetFragment().GetPaddingSetting();
							GetFragment().SetPaddingToNextFragment(CurrentPadding_Default.Get(0.0f));
						}
						else
						{
							GetFragment().Padding.Reset();
						}
						return FReply::Handled();
					})// TODO transactions
					.ButtonColorAndOpacity(this, &SYapDialogueEditor::ButtonColorAndOpacity_PaddingButton) // TODO coloring
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(16, 16))
						.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_ProjectSettings_TabIcon))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(60)
					[
						// -----------------------------
						// TIME DISPLAY
						// -----------------------------
						SNew(SNumericEntryBox<float>)
						.IsEnabled(true)
						.AllowSpin(true)
						.Delta(0.01f)
						.MinSliderValue(-1.0f)
						.MaxSliderValue(UYapProjectSettings::GetFragmentPaddingSliderMax())
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value_Lambda( [this] () { return GetFragment().GetPaddingValue(DialogueNode->GetTypeGroupTag()); } )
						.OnValueChanged_Lambda( [this] (float NewValue) { GetFragment().SetPaddingToNextFragment(NewValue); } )
						.OnValueCommitted_Lambda( [this] (float NewValue, ETextCommit::Type) { GetFragment().SetPaddingToNextFragment(NewValue); } ) // TODO transactions
					]
				]
			]
		];
	}

	if (!bMatureOnly)
	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			SNew(SBox)
			.WidthOverride(Width)
		];
	}

	return Splitter;
}

// ------------------------------------------------------------------------------------------------

FYapFragment& SYapDialogueEditor::GetFragment() const
{
	return DialogueNode->GetFragmentsMutable()[FragmentIndex];
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapDialogueEditor::ButtonColorAndOpacity_PaddingButton() const
{
	if (!GetFragment().Padding.IsSet())
	{
		return YapColor::Green;
	}

	return YapColor::DarkGray;
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset)
{
	EYapMaturitySetting Type = (&Asset == &GetFragment().GetChildSafeBit().AudioAsset) ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature; 
	
	UClass* DialogueAssetClass = nullptr;
	
	const TArray<TSoftClassPtr<UObject>>& DialogueAssetClassPtrs = UYapProjectSettings::GetAudioAssetClasses();
	
	bool bFoundAssetClass = false;
	for (const TSoftClassPtr<UObject>& Ptr : DialogueAssetClassPtrs)
	{
		if (!Ptr.IsNull())
		{
			UClass* LoadedClass = Ptr.LoadSynchronous();
			bFoundAssetClass = true;

			if (DialogueAssetClass == nullptr)
			{
				DialogueAssetClass = LoadedClass;
			}
		}
	}

	if (DialogueAssetClass == nullptr)
	{
		DialogueAssetClass = UObject::StaticClass(); // Note: if I use nullptr then SObjectPropertyEntryBox throws a shitfit
	}
	
	bool bSingleDialogueAssetClass = bFoundAssetClass && DialogueAssetClassPtrs.Num() == 1;

	TDelegate<void(const FAssetData&)> OnObjectChangedDelegate;

	OnObjectChangedDelegate = TDelegate<void(const FAssetData&)>::CreateLambda( [this, Type] (const FAssetData& InAssetData)
	{
		check(DialogueNode.IsValid())
		
		FYapTransactions::BeginModify(LOCTEXT("SettingAudioAsset", "Setting audio asset"), DialogueNode.Get());

		if (Type == EYapMaturitySetting::Mature)
		{
			GetFragment().GetMatureBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		else
		{
			GetFragment().GetChildSafeBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		
		FYapTransactions::EndModify();
	});
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.IsEnabled(bFoundAssetClass)
		.AllowedClass(bSingleDialogueAssetClass ? DialogueAssetClass : UObject::StaticClass()) // Use this feature if the project only has one dialogue asset class type
		.DisplayBrowse(true)
		.DisplayUseSelected(true)
		.DisplayThumbnail(true)
		.OnShouldFilterAsset(this, &SYapDialogueEditor::OnShouldFilterAsset_AudioAssetWidget)
		.EnableContentPicker(true)
		.ObjectPath_Lambda( [&Asset] () { return Asset->GetPathName(); })
		.OnObjectChanged(OnObjectChangedDelegate)
		.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."));

	TSharedRef<SWidget> Widget = SNew(SOverlay)
	//.Visibility_Lambda()
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		AudioAssetProperty
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("MarqueeSelection"))
		.Visibility(this, &SYapDialogueEditor::Visibility_AudioAssetErrorState, &Asset)
		.ColorAndOpacity(this, &SYapDialogueEditor::ColorAndOpacity_AudioAssetErrorState, &Asset)
	];
	
	return Widget;
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapDialogueEditor::CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> VisibilityAtt)
{
	return SNew(SBox)
	.WidthOverride(28)
	.HeightOverride(20)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ContentPadding(1)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SimpleButton)
		.Visibility(VisibilityAtt)
		.IsEnabled(this, &SYapDialogueEditor::Enabled_AudioPreviewButton, AudioAsset)
		.ToolTipText(LOCTEXT("PlayAudio", "Play audio"))
		.OnClicked(this, &SYapDialogueEditor::OnClicked_AudioPreviewWidget, AudioAsset)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(16, 16))
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Speaker))
			.ColorAndOpacity(FSlateColor::UseForeground())	
		]
	];
}

bool SYapDialogueEditor::OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const
{
	const TArray<TSoftClassPtr<UObject>>& Classes = UYapProjectSettings::GetAudioAssetClasses();

	// TODO async loading
	if (Classes.ContainsByPredicate( [&AssetData] (const TSoftClassPtr<UObject>& Class) { return AssetData.GetClass(EResolveClass::Yes) == Class; } ))
	{
		return true;
	}

	return false;	
}

EVisibility SYapDialogueEditor::Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	if (GetAudioAssetErrorLevel(*Asset) > EYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SYapDialogueEditor::ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(*Asset);

	switch (ErrorLevel)
	{
		case EYapErrorLevel::OK:
		{
			return YapColor::DarkGray;
		}
		case EYapErrorLevel::Warning:
		{
			return YapColor::Orange;
		}
		case EYapErrorLevel::Error:
		{
			return YapColor::Red;
		}
		case EYapErrorLevel::Unknown:
		{
			return YapColor::Error; 
		}
		default:
		{
			return YapColor::Noir;
		}
	}
}

bool SYapDialogueEditor::Enabled_AudioPreviewButton(const TSoftObjectPtr<UObject>* Object) const
{
	if (!Object)
	{
		return false;
	}

	if (!Object->IsValid())
	{
		return false;
	}

	return true;
}

TSharedRef<SWidget> SYapDialogueEditor::MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting)
{
	using FLabelText =				FText;
	using FToolTipText =			FText;
	using FSlateBrushIcon =			const FSlateBrush*;
	using FValueFunction =			TOptional<float>	(SYapDialogueEditor::*) (EYapMaturitySetting) const;
	using FValueUpdatedFunction =	void				(SYapDialogueEditor::*) (float, EYapMaturitySetting);
	using FValueCommittedFunction =	void				(SYapDialogueEditor::*) (float, ETextCommit::Type, EYapMaturitySetting);

	constexpr FSlateBrush* NoBrush = nullptr;
	constexpr FValueFunction NoValueFunc = nullptr;
	constexpr FValueUpdatedFunction NoValueUpdatedFunc = nullptr;
	constexpr FValueCommittedFunction NoValueCommittedFunc = nullptr;
	
	using TimeModeData = TTuple
	<
		FLabelText,
		FToolTipText,
		FSlateBrushIcon,
		FValueFunction,
		FValueUpdatedFunction,
		FValueCommittedFunction
	>;

	// Each entry in this represents how to display each row.
	static const TMap<EYapTimeMode, TimeModeData> Data
	{
		{
			EYapTimeMode::Default,
			{
				LOCTEXT("DialogueTimeMode_Default_Label", "Use Default Time"),
				LOCTEXT("DialogueTimeMode_Default_ToolTip", "Use default time method set in project settings"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_ProjectSettings_TabIcon),
				NoValueFunc,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			}
		},
		{
			EYapTimeMode::None, // This entry isn't used for anything
			{
				INVTEXT(""),
				INVTEXT(""),
				NoBrush,
				NoValueFunc,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::AudioTime,
			{
				LOCTEXT("DialogueTimeMode_Audio_Label", "Use Audio Time"),
				LOCTEXT("DialogueTimeMode_Audio_ToolTip", "Use a time read from the audio asset"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_AudioTime),
				&SYapDialogueEditor::Value_TimeSetting_AudioTime,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::TextTime,
			{
				LOCTEXT("DialogueTimeMode_Text_Label", "Use Text Time"),
				LOCTEXT("DialogueTimeMode_Text_ToolTip", "Use a time calculated from text length"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_TextTime),
				&SYapDialogueEditor::Value_TimeSetting_TextTime,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::ManualTime,
			{
				LOCTEXT("DialogueTimeMode_Manual_Label", "Use Specified Time"),
				LOCTEXT("DialogueTimeMode_Manual_ToolTip", "Use a manually entered time"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Timer),
				&SYapDialogueEditor::Value_TimeSetting_ManualTime,
				&SYapDialogueEditor::OnValueUpdated_ManualTime,
				&SYapDialogueEditor::OnValueCommitted_ManualTime,
			},
		}
	};

	const FText& LabelText =					Data[TimeMode].Get<0>();
	const FText& ToolTipText =					Data[TimeMode].Get<1>();
	const FSlateBrush* Icon =					Data[TimeMode].Get<2>();
	FValueFunction ValueFunction =				Data[TimeMode].Get<3>();
	FValueUpdatedFunction UpdatedFunction =		Data[TimeMode].Get<4>();
	FValueCommittedFunction CommittedFunction = Data[TimeMode].Get<5>();
	
	bool bHasCommittedDelegate =	Data[TimeMode].Get<5>() != nullptr;

	auto OnClickedDelegate = FOnClicked::CreateRaw(this, &SYapDialogueEditor::OnClicked_SetTimeModeButton, TimeMode);
	auto ButtonColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &SYapDialogueEditor::ButtonColorAndOpacity_UseTimeMode, TimeMode, TimeModeButtonColors[TimeMode], MaturitySetting);
	auto ForegroundColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &SYapDialogueEditor::ForegroundColor_TimeSettingButton, TimeMode, YapColor::White);

	TSharedPtr<SHorizontalBox> RowBox = SNew(SHorizontalBox);

	// Toss in a filler spacer to let widgets flow to the right side
	RowBox->AddSlot()
	.FillWidth(1.0)
	[
		SNew(SSpacer)
	];

	// On the left pane, add label texts and buttons
	if (MaturitySetting == EYapMaturitySetting::Mature)
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(LabelText)
		];
	}

	if (MaturitySetting == EYapMaturitySetting::Mature || TimeMode != EYapTimeMode::Default)
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.IsEnabled(MaturitySetting == EYapMaturitySetting::Mature)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
			.ContentPadding(FMargin(4, 3))
			.ToolTipText(ToolTipText)
			.OnClicked(OnClickedDelegate)
			.ButtonColorAndOpacity(ButtonColorDelegate)
			.ForegroundColor(ForegroundColorDelegate)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(Icon)
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	auto X = SNew(SNumericEntryBox<float>);

	// Add a numeric box if this row has a value getter
	if (ValueFunction)
	{
		RowBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(60)
			[
				bHasCommittedDelegate
				?
					SNew(SNumericEntryBox<float>)
					.IsEnabled_Lambda( [this] () { return GetFragment().GetTimeModeSetting() == EYapTimeMode::ManualTime; } )
					.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
					//.Justification(ETextJustify::Center) // Numeric Entry Box has a bug, when spinbox is turned on this doesn't work. So don't use it for any of the rows.
					.AllowSpin(true)
					.Delta(0.05f)
					.MaxValue(60) // TODO project setting?
					.MaxSliderValue(10) // TODO project setting?
					.MaxFractionalDigits(1) // TODO project setting?
					.OnValueChanged(this, UpdatedFunction, MaturitySetting)
					.MinValue(0)
					.Value(this, ValueFunction, MaturitySetting)
					.OnValueCommitted(this, CommittedFunction, MaturitySetting)
				:
					SNew(SNumericEntryBox<float>)
					.IsEnabled(false)
					.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
					//.Justification(ETextJustify::Center) // Numeric Entry Box has a bug, when spinbox is turned on this doesn't work. So don't use it for any of the rows.
					.MaxFractionalDigits(1) // TODO project setting?
					.Value(this, ValueFunction, MaturitySetting)
			]
		];
	}
	else
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(60)
		];
	}
	
	return SNew(SBox)
	.HeightOverride(24)
	[
		RowBox.ToSharedRef()
	];
}

FReply SYapDialogueEditor::OnClicked_AudioPreviewWidget(const TSoftObjectPtr<UObject>* Object)
{
	if (!Object)
	{
		return FReply::Handled();
	}

	if (Object->IsNull())
	{
		return FReply::Handled();
	}

	const UYapBroker* BrokerCDO = UYapProjectSettings::GetEditorBrokerDefault();

	if (IsValid(BrokerCDO))
	{
		if (BrokerCDO->ImplementsPreviewAudioAsset_Internal())
		{
			bool bResult = BrokerCDO->PreviewAudioAsset_Internal(Object->LoadSynchronous());

			if (!bResult)
			{
				Yap::EditorFuncs::PostNotificationInfo_Warning
				(
					LOCTEXT("AudioPreview_UnknownWarning_Title", "Cannot Play Audio Preview"),
					LOCTEXT("AudioPreview_UnknownWarning_Description", "Unknown error!")
				);
			}
		}
		else
		{
			Yap::EditorFuncs::PostNotificationInfo_Warning
			(
				LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"),
				LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Description", "Your Broker Class must implement the \"PlayDialogueAudioAssetInEditor\" function.")
			);
		}
	}
	else
	{
		Yap::EditorFuncs::PostNotificationInfo_Warning
		(
			LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"),
			LOCTEXT("AudioPreview_BrokerMissingWarning_Description", "Yap Broker class missing - you must set a Yap Broker class in project settings.")
		);
	}
	
	return FReply::Handled();
}

EYapErrorLevel SYapDialogueEditor::GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const
{
	// If asset isn't loaded, it's a mystery!!!!
	if (Asset.IsPending())
	{
		return EYapErrorLevel::Unknown;
	}
	
	// If we have an asset all we need to do is check if it's the correct class type. Always return an error if it's an improper type.
	if (Asset.IsValid())
	{
		const TArray<TSoftClassPtr<UObject>>& AllowedAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

		if (AllowedAssetClasses.ContainsByPredicate( [Asset] (const TSoftClassPtr<UObject>& Class)
		{
			if (Class.IsPending())
			{
				UE_LOG(LogYapEditor, Warning, TEXT("Synchronously loading audio asset class"));
			}
			
			return Asset->IsA(Class.LoadSynchronous()); // TODO verify this works?
		}))
		{
			return EYapErrorLevel::OK;
		}
		else
		{
			return EYapErrorLevel::Error;
		}
	}

	EYapMissingAudioErrorLevel MissingAudioBehavior = GetTypeGroup().GetMissingAudioErrorLevel();

	EYapTimeMode TimeModeSetting = GetFragment().GetTimeModeSetting();
	
	// We don't have any audio asset set. If the dialogue is set to use audio time but does NOT have an audio asset, we either indicate an error (prevent packaging) or indicate a warning (allow packaging) 
	if ((TimeModeSetting == EYapTimeMode::AudioTime) || (TimeModeSetting == EYapTimeMode::Default && GetTypeGroup().GetDefaultTimeModeSetting() == EYapTimeMode::AudioTime))
	{
		switch (MissingAudioBehavior)
		{
			case EYapMissingAudioErrorLevel::OK:
			{
				return EYapErrorLevel::OK;
			}
			case EYapMissingAudioErrorLevel::Warning:
			{
				return EYapErrorLevel::Warning;
			}
			case EYapMissingAudioErrorLevel::Error:
			{
				return EYapErrorLevel::Error;
			}
		}
	}

	return EYapErrorLevel::OK;
}

TOptional<float> SYapDialogueEditor::Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const
{
	if (GEditor && GEditor->IsPlaySessionInProgress())
	{
		return GetFragment().GetBitMutable(MaturitySetting).GetAudioTime(GEditor->GetCurrentPlayWorld(), EYapLoadContext::AsyncEditorOnly);
	}
	else
	{
		return GetFragment().GetBitMutable(MaturitySetting).GetAudioTime(GEditor->EditorWorld, EYapLoadContext::AsyncEditorOnly);
	}
}

TOptional<float> SYapDialogueEditor::Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const
{
	return GetFragment().GetBit(GEditor->EditorWorld, MaturitySetting).GetTextTime(DialogueNode->GetTypeGroupTag());
}

TOptional<float> SYapDialogueEditor::Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const
{
	return GetFragment().GetBit(GEditor->EditorWorld, MaturitySetting).GetManualTime();
}

void SYapDialogueEditor::OnValueUpdated_ManualTime(float NewValue, EYapMaturitySetting MaturitySetting)
{
	GetFragment().GetBitMutable(MaturitySetting).SetManualTime(NewValue);
	GetFragment().SetTimeModeSetting(EYapTimeMode::ManualTime);
}

void SYapDialogueEditor::OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType, EYapMaturitySetting MaturitySetting)
{
}

FReply SYapDialogueEditor::OnClicked_SetTimeModeButton(EYapTimeMode TimeMode)
{
	check(DialogueNode.IsValid());
	
	FYapTransactions::BeginModify(LOCTEXT("TimeModeChanged", "Time mode changed"), DialogueNode.Get());

	GetFragment().SetTimeModeSetting(TimeMode);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

FSlateColor SYapDialogueEditor::ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint, EYapMaturitySetting MaturitySetting) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GEditor->EditorWorld, GetDisplayMaturitySetting(), DialogueNode->GetTypeGroupTag()) == TimeMode)
	{
		// Implicit match through project defaults
		return ColorTint.Desaturate(0.50);
	}
	
	return YapColor::DarkGray;
}

FSlateColor SYapDialogueEditor::ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GEditor->EditorWorld, GetDisplayMaturitySetting(), DialogueNode->GetTypeGroupTag()) == TimeMode)
	{
		// Implicit match through project defaults
		return ColorTint;
	}
	
	return YapColor::Gray;
}

EYapMaturitySetting SYapDialogueEditor::GetDisplayMaturitySetting() const
{
	if (GEditor->PlayWorld)
	{
		return UYapSubsystem::GetCurrentMaturitySetting(GEditor->EditorWorld);
	}

	if (!bNeedsChildSafe)
	{
		return EYapMaturitySetting::Mature;
	}
	
	return Owner->GetIsChildSafeCheckBoxHovered() ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature;
}

const FYapTypeGroupSettings& SYapDialogueEditor::GetTypeGroup() const
{
	return UYapProjectSettings::GetTypeGroup(DialogueNode->GetTypeGroupTag());
}

void SYapDialogueEditor::SetFocus_MatureDialogue()
{
	FSlateApplication::Get().SetAllUserFocus(MatureDialogueText);
	PressEndKey();
}

void SYapDialogueEditor::SetFocus_MatureTitleText()
{
	FSlateApplication::Get().SetAllUserFocus(MatureTitleText);
	PressEndKey();	
}

void SYapDialogueEditor::SetFocus_ChildSafeDialogue()
{
	FSlateApplication::Get().SetAllUserFocus(ChildSafeDialogueText);
	PressEndKey();
}

void SYapDialogueEditor::SetFocus_ChildSafeTitleText()
{
	FSlateApplication::Get().SetAllUserFocus(ChildSafeTitleText);
	PressEndKey();
}

void SYapDialogueEditor::PressEndKey()
{
	FKeyEvent EndKey(EKeys::End, FModifierKeysState(), 0, false, 0, 0);
	
	FSlateApplication::Get().ProcessKeyDownEvent(EndKey);
	FSlateApplication::Get().ProcessKeyUpEvent(EndKey);
}

#undef LOCTEXT_NAMESPACE
