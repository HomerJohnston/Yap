// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "FlowAsset.h"
#include "Engine/World.h"
#include "PropertyCustomizationHelpers.h"
#include "SAssetDropTarget.h"
#include "SLevelOfDetailBranchNode.h"
#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/YapEditorStyle.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Yap/YapBitReplacement.h"
#include "YapEditor/SlateWidgets/SYapActivationCounterWidget.h"
#include "YapEditor/SlateWidgets/SYapConditionsScrollBox.h"
#include "YapEditor/SlateWidgets/SYapButtonPopup.h"
#include "YapEditor/SlateWidgets/SYapPropertyMenuAssetPicker.h"
#include "Templates/FunctionFwd.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapErrorLevel.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Yap/Enums/YapLoadContext.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Helpers/ProgressionSettingWidget.h"
#include "YapEditor/SlateWidgets/SYapDialogueEditor.h"
#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"
#include "YapEditor/SlateWidgets/SYapTimeProgressionWidget.h"

FSlateFontInfo SFlowGraphNode_YapFragmentWidget::DialogueTextFont;

TMap<EYapTimeMode, FLinearColor> SFlowGraphNode_YapFragmentWidget::TimeModeButtonColors =
	{
	{ EYapTimeMode::None, YapColor::Red },
	{ EYapTimeMode::Default, YapColor::Green },
	{ EYapTimeMode::AudioTime, YapColor::Cyan },
	{ EYapTimeMode::TextTime, YapColor::LightBlue },
	{ EYapTimeMode::ManualTime, YapColor::Orange },
};

#define LOCTEXT_NAMESPACE "YapEditor"

SLATE_IMPLEMENT_WIDGET(SFlowGraphNode_YapFragmentWidget)
void SFlowGraphNode_YapFragmentWidget::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer&)
{
	// TODO wtf does anything go in here?
}

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs)
{	
	Owner = InArgs._InOwner;
	
	FragmentIndex = InArgs._InFragmentIndex;

	if (UYapDeveloperSettings::GetGraphDialogueFontUserOverride().HasValidFont())
	{
		DialogueTextFont = UYapDeveloperSettings::GetGraphDialogueFontUserOverride();
	}
	else if (GetNodeConfig().GetGraphDialogueFont().HasValidFont())
	{
		DialogueTextFont = GetNodeConfig().GetGraphDialogueFont();
	}
	else
	{
		DialogueTextFont = YapFonts.Font_DialogueText;
	}
	
	ChildSlot
	[
		CreateFragmentWidget()
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreTextDisplayWidget()
{	
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.Padding(0, 0, 0, 0)
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.MaxDesiredHeight(49)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
			.HighDetail()
			[
				SNew(SYapButtonPopup)
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_NoBorder)
				.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_ExpandedEditor))
				.PopupPlacement(MenuPlacement_Center)
				.ButtonForegroundColor(YapColor::DarkGray_SemiGlass)
				.OnPostPopup(this, &ThisClass::OnPostPopup_TextEditor, EYapTextType::Speech, GetDisplayMaturitySetting())
				.ButtonContent()
				[
					CreateDialogueDisplayWidget()
				]
			]
			.LowDetail()
			[
				SNew(SBorder)
				.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Rounded))
				.BorderBackgroundColor(YapColor::DarkGray_Glass)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.AutoWrapText_Lambda( [this] () { return !GetNodeConfig().GetPreventDialogueTextWrapping(); })
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
					.Font(DialogueTextFont)
					.Text_Lambda( [this] () { return GetFragment().GetDialogueText(GEditor->EditorWorld, GetDisplayMaturitySetting()); } )
					.ColorAndOpacity(this, &ThisClass::GetColorAndOpacityForFragmentText, YapColor::LightGray)
				]
			]
		]
	]
	+ SVerticalBox::Slot()
	.Padding(0, 4, 0, 0)
	.AutoHeight()
	[
		SNew(SBox)
		.MaxDesiredHeight(20)
		.Visibility(this, &ThisClass::Visibility_TitleTextWidgets)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
			.HighDetail()
			[
				SNew(SYapButtonPopup)
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_NoBorder)
				.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_ExpandedEditor))
				.PopupPlacement(MenuPlacement_Center)
				.ButtonForegroundColor(YapColor::DarkGray_SemiGlass)
				.OnPostPopup(this, &ThisClass::OnPostPopup_TextEditor, EYapTextType::TitleText, GetDisplayMaturitySetting())
				.ButtonContent()
				[
					CreateTitleTextDisplayWidget()
				]
			]
			.LowDetail()
			[
				SNew(SBorder)
				.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
				.BorderBackgroundColor(YapColor::DarkGray_Glass)
				.ToolTip(nullptr)
				.Padding(0)
				[
					SNew(STextBlock)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
					.Text_Lambda( [this] () { return GetFragment().GetTitleText(GEditor->EditorWorld, GetDisplayMaturitySetting()); } )
					.ToolTipText(LOCTEXT("TitleTextDisplayWidget_ToolTipText", "Title text"))
					.ColorAndOpacity(this, &ThisClass::GetColorAndOpacityForFragmentText, YapColor::YellowGray)
				]
			]
		]
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_ExpandedEditor()
{
	TSharedRef<SFlowGraphNode_YapFragmentWidget> x = StaticCastSharedRef<SFlowGraphNode_YapFragmentWidget>(AsShared());
	
	return SAssignNew(ExpandedDialogueEditor, SYapDialogueEditor)
		.OwnerIn(x)
		.DialogueNodeIn(GetDialogueNodeMutable())
		.FragmentIndexIn(FragmentIndex)
		.bNeedsChildSafeIn(NeedsChildSafeData());
}

void SFlowGraphNode_YapFragmentWidget::OnPostPopup_TextEditor(bool& bOverrideFocus, EYapTextType TextType, EYapMaturitySetting Maturity)
{
	switch (TextType)
	{
		case EYapTextType::Speech:
		{
			if (Maturity == EYapMaturitySetting::Mature)
			{
				ExpandedDialogueEditor.Pin()->SetFocus_MatureDialogue();
			}
			else
			{
				ExpandedDialogueEditor.Pin()->SetFocus_ChildSafeDialogue();
			}
			break;
		}
		case EYapTextType::TitleText:
		{
			if (Maturity == EYapMaturitySetting::Mature)
			{
				ExpandedDialogueEditor.Pin()->SetFocus_MatureTitleText();
			}
			else
			{
				ExpandedDialogueEditor.Pin()->SetFocus_ChildSafeTitleText();
			}
			break;
		}
	}
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount() const
{
	return GetFragment().GetActivationCount();
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit() const
{
	return GetFragment().GetActivationLimit();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentControlsWidget() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->HasActiveOverlay())
	{
		return EVisibility::Collapsed;
	}
	
	return GetDialogueNode()->GetNumFragments() > 1 ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const
{
	if (FragmentIndex == 0 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Up)
	{
		return EVisibility::Hidden;
	}

	if (FragmentIndex == GetDialogueNode()->GetNumFragments() - 1 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Down)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection)
{
	int32 OtherIndex = YapFragmentControlsDirection == EYapFragmentControlsDirection::Up ? FragmentIndex - 1 : FragmentIndex + 1;
	
	GetDialogueNodeMutable()->SwapFragments(FragmentIndex, OtherIndex);

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentDelete()
{
	GetDialogueNodeMutable()->DeleteFragmentByIndex(FragmentIndex);

	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentControlsWidget()
{
	return SNew(SBox)
	.Visibility(this, &ThisClass::Visibility_FragmentControlsWidget)
	[
		SNew(SVerticalBox)
		// UP
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(0, 2)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
			.Visibility(this, &ThisClass::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Up)
			.OnClicked(this, &ThisClass::OnClicked_FragmentShift, EYapFragmentControlsDirection::Up)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronUp"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		// DELETE
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(0, 0)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
			.OnClicked(this, &ThisClass::OnClicked_FragmentDelete)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseStyle())
			]
		]
		// DOWN
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Center)
		.Padding(0, 2)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
			.Visibility(this, &ThisClass::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Down)
			.OnClicked(this, &ThisClass::OnClicked_FragmentShift, EYapFragmentControlsDirection::Down)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronDown"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

bool SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton(const TSoftObjectPtr<UObject>* Object) const
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

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_AudioPreviewWidget(const TSoftObjectPtr<UObject>* Object)
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

// ================================================================================================
// FRAGMENT HIGHLIGHT WIDGET
// ================================================================================================

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentHighlightWidget()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body")) // Filled, rounded nicely
	.Visibility(this, &ThisClass::Visibility_FragmentHighlight)
	.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_FragmentHighlight);
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentHighlight() const
{
	if (GetFragment().IsAwaitingManualAdvance() || FragmentRecentlyRan())
	{
		return EVisibility::HitTestInvisible;
	}

	if (GetFragment().IsActivationLimitMet())
	{
		return EVisibility::HitTestInvisible;
	}
	
	if (GetDialogueNode()->GetActivationState() != EFlowNodeState::Active && !GetDialogueNode()->CheckActivationLimits())
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_FragmentHighlight() const
{
	if (FragmentIsRunning())
	{
		return YapColor::White_Glass;
	}

	if (GetFragment().IsAwaitingManualAdvance())
	{
		return YapColor::Yellow_Glass;
	}
	
	if (GetFragment().IsActivationLimitMet())
	{
		return YapColor::Red_Glass;
	}
	
	if (GetDialogueNode()->GetActivationState() != EFlowNodeState::Active && !GetDialogueNode()->CheckActivationLimits())
	{
		return YapColor::Red_Glass;
	}

	FLinearColor C = YapColor::White_Glass;
	
	UWorld* World = GEditor->GetCurrentPlayWorld(GEditor->PlayWorld);
	
	if (World)// && GetFragment().GetStartTime() >= 0.0)
	{
		float MinOpaqueTime = 1.0f;
		float FadeTime = 0.5f;

		float EndTime = FMath::Max(GetFragment().GetStartTime() + MinOpaqueTime, GetFragment().GetEndTime());

		float Elapsed = World->GetTimeSeconds() - EndTime;

		C.A *= FMath::Lerp(1.0f, 0.0f, FMath::Clamp(Elapsed / FadeTime, 0.0f, 1.0f));
	}

	return C;
}


void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeActivationLimit", "Change activation limit"), GetDialogueNodeMutable());

	GetFragmentMutable().ActivationLimit = FCString::Atoi(*Text.ToString());

	GetDialogueNodeMutable()->OnReconstructionRequested.Execute();
	
	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateUpperFragmentBar()
{
	TOptional<bool>* SkippableSettingRaw = &GetFragmentMutable().Skippable;
	const TAttribute<bool> SkippableDefaultAttr = TAttribute<bool>::CreateLambda( [this] () { return GetDialogueNode()->GetSkippable(); });
	const TAttribute<bool> SkippableEvaluatedAttr = TAttribute<bool>::CreateLambda( [this, SkippableDefaultAttr] ()
	{
		return GetFragment().GetSkippable(SkippableDefaultAttr.Get());
	});
	TOptional<bool>* AutoAdvanceSettingRaw = &GetFragmentMutable().AutoAdvance;
	const TAttribute<bool> AutoAdvanceDefaultAttr = TAttribute<bool>::CreateLambda( [this] () { return GetDialogueNode()->GetNodeAutoAdvance(); });
	const TAttribute<bool> AutoAdvanceEvaluatedAttr = TAttribute<bool>::CreateLambda( [this, AutoAdvanceDefaultAttr] ()
	{
		return GetDialogueNode()->GetFragmentAutoAdvance(FragmentIndex);
	});

	TSharedRef<SWidget> ProgressionPopupButton = MakeProgressionPopupButton(SkippableSettingRaw, SkippableEvaluatedAttr, AutoAdvanceSettingRaw, AutoAdvanceEvaluatedAttr);

	return SNew(SBox)
	.Padding(0, 0, 32, 4)
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(6, -8, 0, -8)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(20)
				[
					SNew(SYapActivationCounterWidget, FOnTextCommitted::CreateSP(this, &ThisClass::OnTextCommitted_FragmentActivationLimit))
					.ActivationCount(this, &ThisClass::GetFragmentActivationCount)
					.ActivationLimit(this, &ThisClass::GetFragmentActivationLimit)
					.FontHeight(10)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(6, 0, 0, 0)
			[
				SNew(SYapConditionsScrollBox)
				.DialogueNode_Lambda( [this] () { return GetDialogueNodeMutable(); } )
				.FragmentIndex(FragmentIndex)
				.ConditionsArrayProperty(FindFProperty<FArrayProperty>(FYapFragment::StaticStruct(), GET_MEMBER_NAME_CHECKED(FYapFragment, Conditions)))
				.ConditionsContainer_Lambda( [this] () { return &GetFragmentMutable(); } )
				.OnConditionsArrayChanged(Owner, &SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(4, 0, 1, 0)
			[
				SNew(SLevelOfDetailBranchNode)
				.Visibility(this, &ThisClass::Visibility_FragmentTagWidget)
				.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
				.HighDetail()
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						CreateFragmentTagWidget()
					]
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(6, -2, -27, -2)
			[
				SNew(SBox)
				.WidthOverride(20)
				.Visibility_Lambda( [this] () { return GetDialogueNode()->GetNodeType() == EYapDialogueNodeType::TalkAndAdvance ? EVisibility::Collapsed : EVisibility::Visible; } )
				[
					MakeProgressionPopupButton(SkippableSettingRaw, SkippableEvaluatedAttr, AutoAdvanceSettingRaw, AutoAdvanceEvaluatedAttr)
				]
				[
					ProgressionPopupButton
				]
			]
		]
		.LowDetail()
		[
			SNew(SBox)
			.HeightOverride(20)
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget() const
{
	return GetDialogueNode()->GetDialogueTag().IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// CHILD-SAFE WIDGET
// ================================================================================================

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_ChildSafeSettings() const
{
	if (!NeedsChildSafeData())
	{
		return ECheckBoxState::Unchecked;
	}

	if (HasCompleteChildSafeData())
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Undetermined;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState)
{
	if (!NeedsChildSafeData())
	{
		FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOnChildSafe", "Enable child-safe settings"), GetDialogueNodeMutable());

		GetFragmentMutable().bEnableChildSafe = true;
	}
	else
	{
		// Turn off child safety settings if we don't have any data assigned; otherwise flash a warning
		if (!HasAnyChildSafeData())
		{
			FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNodeMutable());

			GetFragmentMutable().bEnableChildSafe = false;
		}
		else
		{
			EAppReturnType::Type ReturnType = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("TurnOffChildSafeSettingsDialog_DataWarning", "Node contains child-safe data: do you want to reset it? Press 'Yes' to remove child-safe data, or 'No' to leave it hidden."), LOCTEXT("TurnOffChildSafeSettingsDialog_Title", "Turn Off Child-Safe Settings"));

			switch (ReturnType)
			{
				case EAppReturnType::Yes:
				{
					FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("ResetChildSafeSettings", "Reset child-safe settings"), GetDialogueNodeMutable());

					GetFragmentMutable().GetChildSafeBitMutable().ClearAllData();
					GetFragmentMutable().bEnableChildSafe = false;

					break;
				}
				case EAppReturnType::No:
				{
					FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNodeMutable());
					
					GetFragmentMutable().bEnableChildSafe = false;

					break;
				}
				default: // Cancel
				{
					// Do nothing, just close the dialog
					break;
				}
			}
		}
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox() const
{
	if (NeedsChildSafeData())
	{
		return HasCompleteChildSafeData() ? YapColor::LightBlue : YapColor::Red;
	}
	else
	{
		return HasAnyChildSafeData() ? YapColor::YellowGray : YapColor::Button_Unset();
	}
}

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_ChildSafeButton(TArrayView<FAssetData> AssetDatas) const
{
	return OnAreAssetsAcceptableForDrop_TextWidget(AssetDatas);
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_ChildSafeButton(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	FYapTransactions::BeginModify(LOCTEXT("SetAudioAsset", "Set audio asset"), GetDialogueNodeMutable());

	GetFragmentMutable().GetChildSafeBitMutable().SetDialogueAudioAsset(Object);

	FYapTransactions::EndModify();	
}

// ================================================================================================
// FRAGMENT WIDGET
// ================================================================================================

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize();
	int32 PortraitBorder = 2;

	return SAssignNew(FragmentWidgetOverlay, SOverlay)
	.ToolTip(nullptr)
	+ SOverlay::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 3)
		[
			CreateUpperFragmentBar()
		]
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.AutoHeight()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(32)
					[
						SNew(SLevelOfDetailBranchNode)
						.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
						.HighDetail()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0, 0, 0, 2)
							[
								SNew(SBox)
								.Visibility_Lambda( [this] () { return GetNodeConfig().GetDisableChildSafe() ? EVisibility::Collapsed : EVisibility::Visible; } )
								.WidthOverride(22)
								.HeightOverride(22)
								[
									SNew(SAssetDropTarget)
									.bSupportsMultiDrop(false)
									.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_ChildSafeButton)
									.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_ChildSafeButton)
									[
										SAssignNew(ChildSafeCheckBox, SCheckBox)
										.Cursor(EMouseCursor::Default)
										.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
										.Padding(FMargin(0, 0))
										.CheckBoxContentUsesAutoWidth(true)
										.ToolTip(nullptr) // Don't show a tooltip, it's distracting
										.IsChecked(this, &ThisClass::IsChecked_ChildSafeSettings)
										.OnCheckStateChanged(this, &ThisClass::OnCheckStateChanged_MaturitySettings)
										.Content()
										[
											SNew(SBox)
											.WidthOverride(20)
											.HeightOverride(20)
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SNew(SImage)
												.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Baby))
												.DesiredSizeOverride(FVector2D(16, 16))
												.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_ChildSafeSettingsCheckBox)
											]
										]
									]
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0, 2, 0, 0)
							[
								SNew(SBox)
								.Visibility_Lambda( [this] () { return GetNodeConfig().GetDisableMoodTags() ? EVisibility::Collapsed : EVisibility::Visible; } )
								.WidthOverride(22)
								.HeightOverride(22)
								[
									CreateMoodTagSelectorWidget()
								]
							]
						]
						.LowDetail()
						[
							SNew(SBox)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(FragmentTextOverlay, SOverlay)
					.Visibility(EVisibility::Visible)
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.HeightOverride(PortraitSize + 2 * PortraitBorder)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Top)
							.AutoWidth()
							.Padding(0, 0, 0, 0)
							[
								CreateSpeakerWidget()
							]
							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.FillWidth(1.0f)
							.Padding(0, 0, 0, 0)
							[
								CreateCentreTextDisplayWidget()
							]
						]
					]
					+ SOverlay::Slot()
					[
						CreateFragmentHighlightWidget()
					]
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(32)
					[
						CreateRightFragmentPane()
					]
				]
			]
			+ SOverlay::Slot()
			.Padding(32, 0, 32, -7)
			.VAlign(EVerticalAlignment::VAlign_Bottom)
			[
				SNew(SLevelOfDetailBranchNode)
				.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
				.HighDetail()
				[
					SNew(SBox)
					.HeightOverride(3)
					.Visibility(this, &ThisClass::Visibility_TimeProgressionWidget)
					[
						SNew(SYapTimeProgressionWidget)
						.BarColor(this, &ThisClass::ColorAndOpacity_FragmentTimeIndicator)
						.PaddingIsSet(this, &ThisClass::Bool_PaddingTimeIsSet)
						.SpeechTime_Lambda( [this] () { return GetDialogueNode()->GetSpeechTime(FragmentIndex, GetDisplayMaturitySetting(), EYapLoadContext::AsyncEditorOnly); } )
						.PaddingTime_Lambda( [this] () { return GetDialogueNode()->GetPadding(FragmentIndex); } )
						.MaxDisplayTime_Lambda( [this] () { return UYapProjectSettings::GetDialogueTimeSliderMax(); } )
						.PlaybackTime_Lambda( [this] () { return Percent_FragmentTime(); } )
					]
				]
				.LowDetail()
				[
					SNew(SBox)
					.HeightOverride(3)
				]
			]
		]
	];
}

void SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType, EYapMaturitySetting MaturitySetting)
{
	FYapTransactions::BeginModify(LOCTEXT("EnterManualTimeValue", "Enter manual time value"), GetDialogueNodeMutable());

	if (CommitType != ETextCommit::OnCleared)
	{
		GetFragmentMutable().GetBitMutable(MaturitySetting).SetManualTime(NewValue);
	}

	FYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioSettingsButton() const
{
	if (GetNodeConfig().GetMissingAudioErrorLevel() != EYapMissingAudioErrorLevel::OK)
	{
		return EVisibility::Visible;
	}

	if (GetFragment().HasAudio())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_DialogueErrorState() const
{
	if (!NeedsChildSafeData())
	{
		return EVisibility::Collapsed;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();
	
	if (MatureBit.HasDialogueText() != ChildSafeBit.HasDialogueText())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// TODO code duplication with UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments()
bool CheckAudioAssetUsesAudioID(const UFlowNode_YapDialogue* Node, int32 FragmentIndex, TSoftObjectPtr<UObject> Asset, bool& bCorrectMatch)
{
	int32 AudioIDLen = Node->GetAudioID().Len();
	int32 FragmentIDLen = 3; // TODO magic number move this to project settings or some other constant
	
	// TODO make the naming pattern a project setting
	FRegexPattern RegexActual(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}"), {AudioIDLen, FragmentIDLen}));
	FRegexMatcher RegexMatcher(RegexActual, *Asset.ToString());

	if (RegexMatcher.FindNext())
	{
		FString ID = RegexMatcher.GetCaptureGroup(0);

		FString AudioID = ID.LeftChop(FragmentIDLen + 1); // TODO lots of duplicated code running around, centralize it all so this mechanism is consistent everywhere
				
		int32 IDInt = FCString::Atoi(*ID.RightChop(AudioIDLen + 1));

		if (AudioID == Node->GetAudioID() && IDInt == FragmentIndex)
		{
			bCorrectMatch = true;
		}
		else
		{
			bCorrectMatch = false;
		}

		// This audio asset name contains an Audio ID (meaning it has AAA-000 somewhere in the asset name)
		return true;
	}

	// This audio asset does not contain an Audio ID (it's just a random name)
	return false;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioIDText() const
{
	const TSoftObjectPtr<UObject>& MatureAudioAsset = GetFragment().GetMatureBit().AudioAsset;
	const TSoftObjectPtr<UObject>& SafeAudioAsset = GetFragment().GetChildSafeBit().AudioAsset;

	bool bNeedsChildSafeAudio = NeedsChildSafeData();

	const FLinearColor Error = YapColor::Red;
	const FLinearColor NoAudio = YapColor::White;
	const FLinearColor AllGood = YapColor::DarkGray;

	FLinearColor Color = AllGood;
	
	if (MatureAudioAsset.IsNull())
	{
		Color = NoAudio;
	}
	else if (SafeAudioAsset.IsNull() && bNeedsChildSafeAudio)
	{
		Color = Error;
	}
	else if (!MatureAudioAsset.IsNull())
	{
		bool bCorrectMatch = false;
		bool bAssetUsesAudioID = CheckAudioAssetUsesAudioID(GetDialogueNode(), FragmentIndex, MatureAudioAsset, bCorrectMatch);

		if (!bAssetUsesAudioID)
		{
			Color = YapColor::LightBlue;
		}
		else if (bAssetUsesAudioID && !bCorrectMatch)
		{
			Color = Error;
		}
	}
	else if (!SafeAudioAsset.IsNull() && bNeedsChildSafeAudio)
	{
		bool bCorrectMatch = false;
		bool bAssetUsesAudioID = CheckAudioAssetUsesAudioID(GetDialogueNode(), FragmentIndex, SafeAudioAsset, bCorrectMatch);

		if (!bAssetUsesAudioID || !bCorrectMatch)
		{
			Color = Error;
		}
	}
	
	if (AudioIDButton.IsValid() && AudioIDButton->IsHovered())
	{
		Color /= YapColor::LightGray;
	}

	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TimeProgressionWidget() const
{
	UWorld* World = GetDialogueNode()->GetWorld();

	if (GetFragment().GetTimeMode(World, GetNodeConfig()) == EYapTimeMode::None)
	{
		return EVisibility::Collapsed;
	}
	
	return EVisibility::SelfHitTestInvisible;
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------


TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueDisplayWidget()
{
	int32 TimeSliderSize = 8;

	FNumberFormattingOptions Args;
	Args.UseGrouping = false;
	Args.MinimumIntegralDigits = 3;

	const FString FragmentIndexText = FText::AsNumber(FragmentIndex, &Args).ToString();
	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueNode = GetDialogueNodeMutable();

	const FYapBit& Bit = GetFragment().GetBit(GEditor->EditorWorld, GetDisplayMaturitySetting());
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SAssetDropTarget)
		.bSupportsMultiDrop(false)
		.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_TextWidget)
		.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_TextWidget)
		[
			SNew(SBorder)
			.Cursor(EMouseCursor::Default)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
			.BorderBackgroundColor(FSlateColor::UseForeground())
			.ToolTipText_Lambda( [this] ()
			{
				if (NeedsChildSafeData() && GetDisplayMaturitySetting() != EYapMaturitySetting::ChildSafe)
				{
					return LOCTEXT("DialogueTextDisplayWidget_ToolTipText", "Hold shift to show child-safe");
				}

				return FText::GetEmpty();
			})
			.Padding(0)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.Padding(4, 4, 4, 2)
				[
					SNew(STextBlock)
					.AutoWrapText_Lambda( [this] () { return !GetNodeConfig().GetPreventDialogueTextWrapping(); })
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
					.Font(DialogueTextFont)
					.Text_Lambda( [this] () { return GetFragment().GetDialogueText(GEditor->EditorWorld, GetDisplayMaturitySetting()); } )
					.ColorAndOpacity(this, &ThisClass::GetColorAndOpacityForFragmentText, YapColor::LightGray)
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Visibility_Lambda( [this] ()
					{
						const FText& DialogueText =  GetFragment().GetDialogueText(GEditor->EditorWorld, GetDisplayMaturitySetting());
						return DialogueText.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;
					})
					.Justification(ETextJustify::Center)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
					.SimpleTextMode(true)
					.Text_Lambda( [this] ()
					{
						if (!NeedsChildSafeData())
						{
							return LOCTEXT("DialogueText_None", "Dialogue Text (None)");
						}
						
						return GetDisplayMaturitySetting() == EYapMaturitySetting::Mature ? LOCTEXT("MatureDialogueText_None", "Mature Dialogue Text (None)") : LOCTEXT("SafeDialogueText_None", "Child-Safe Dialogue Text (None)");
					})
					.ColorAndOpacity(YapColor::White_Glass)
				]
			]
		]
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	.Padding(-4)
	[
		CreateDirectedAtWidget()
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	.Padding(-2)
	[
		// TODO use tooltips to display error reasons
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("MarqueeSelection"))
		.Visibility(this, &ThisClass::Visibility_DialogueErrorState)
		.BorderBackgroundColor(YapColor::Red)
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Right)
	.Padding(0)
	[
		SAssignNew(AudioIDButton, SButton)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SimpleYapButton)
		.OnClicked_Lambda( [&Bit, this] () { return OnClicked_AudioPreviewWidget(&Bit.AudioAsset); } )
		.ContentPadding(0)
		//.ForegroundColor(YapColor::White_Trans)
		.ButtonColorAndOpacity(YapColor::Transparent)
		.ToolTipText_Lambda( [&Bit, this] ()
		{
			if (Bit.AudioAsset.IsNull())
			{
				return LOCTEXT("AudioIDTooltip_Unset", "No audio");
			}

			return FText::FromString(Bit.AudioAsset.GetAssetName());
		})
		[
			SNew(SBorder)
			.Visibility_Lambda( [this] () { return GetNodeConfig().GetHideAudioID() ? EVisibility::Collapsed : EVisibility::Visible; } )
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_IDTag))
			.BorderBackgroundColor(this, &ThisClass::ColorAndOpacity_AudioIDButton)
			.Padding(4)
			[
				SNew(STextBlock)
				.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AudioIDText)
				.SimpleTextMode(true)
				.Text_Lambda( [FragmentIndexText, DialogueNode] ()
				{
					if (DialogueNode.IsValid())
					{
						return FText::AsCultureInvariant(DialogueNode->GetAudioID() + "-" + FragmentIndexText);
					}

					return INVTEXT("");
				} )
			]
		]
	];
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------
TOptional<float> SFlowGraphNode_YapFragmentWidget::Percent_FragmentTime() const
{
	if (!GEditor->IsPlaySessionInProgress())
	{
		return NullOpt;
	}

	if (!FragmentIsRunning())
	{
		return NullOpt;
	}
	
	if (GetFragment().GetStartTime() >= GetFragment().GetEndTime())
	{
		return GEditor->PlayWorld->GetTimeSeconds() - GetFragment().GetStartTime();
	}
	else
	{
		return 0.0;
	}
}

FLinearColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_FragmentTimeIndicator() const
{
	FLinearColor Color;
	
	EYapTimeMode TimeMode = GetFragment().GetTimeModeSetting();

	Color = TimeModeButtonColors[TimeMode];

	if (TimeMode == EYapTimeMode::Default)
	{
		Color = YapColor::Desaturate(Color, 0.4f);
	}
	else
	{
		Color = YapColor::Desaturate(Color, 0.2f);
	}

	// Darken it during play if it isn't running
	if (GEditor->IsPlaySessionInProgress() && !FragmentIsRunning())
	{
		Color *= YapColor::Gray;
	}

	return Color;
}

bool SFlowGraphNode_YapFragmentWidget::Bool_PaddingTimeIsSet() const
{
	return GetFragment().GetPaddingSetting().IsSet();
}

// ---------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_SpeakerWidget(const UObject* Character)
{
	UE_LOG(LogYapEditor, VeryVerbose, TEXT("PopupContentGetter_SpeakerWidgetNew"));
	
	return SNew(SBorder)
	.Padding(1, 1, 1, 1)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(6, 0, 6, 0)
		[
			SNew(SBox)
			.WidthOverride(15) // Rotated widgets are laid out per their original transform, use negative padding and a width override for rotated text
			.Padding(-80) 
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Speaker_PopupLabel", "SPEAKER"))
				.SimpleTextMode(true)
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-90.0f))))
				.Font(YapFonts.Font_SectionHeader)
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SYapPropertyMenuAssetPicker)
			.OnShouldFilterAsset(this, &ThisClass::ShouldFilter_YapSpeaker) // This works lovely... but it loads all assets. It's slow, ugly, occupies memory, and fills the output log with every asset error in the project.
			//.AllowedClasses(UYapProjectSettings::GetCharacterClasses_SyncLoad())
			.AllowClear(true)
			.InitialObject(Character)
			.OnSet(this, &ThisClass::OnSetNewSpeakerAsset)
		]
	];
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewSpeakerAsset(const FAssetData& AssetData)
{
	if (AssetData.IsValid())
	{
		if (!IsAsset_YapSpeaker(AssetData))
		{
			Yap::EditorFuncs::PostNotificationInfo_Warning
			(
				LOCTEXT("InvalidSpeakerWarning_Title", "Invalid Speaker"),
				FText::Format(
					LOCTEXT("InvalidSpeakerWarning_Body", "[{0}] does not inherit the IYapSpeaker interface!"),
					FText::FromString(AssetData.AssetName.ToString())
				)
			);
	
			return;
		}
	}
	
	FYapTransactions::BeginModify(LOCTEXT("SetSpeakerCharacter", "Set speaker character"), GetDialogueNodeMutable());

	GetFragmentMutable().SetSpeakerNew(Cast<UObject>(AssetData.GetAsset()));

	FYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_TextWidget(TArrayView<FAssetData> AssetDatas) const
{
	if (AssetDatas.Num() != 1)
	{
		return false;
	}

	UClass* AssetClass = AssetDatas[0].GetClass();
	
	const TArray<TSoftClassPtr<UObject>>& AllowableClasses = UYapProjectSettings::GetAudioAssetClasses();
	
	for (const TSoftClassPtr<UObject>& AllowableClass : AllowableClasses)
	{
		if (AssetClass->IsChildOf(AllowableClass.LoadSynchronous())) // TODO return false instead and request an async load
		{
			return true;
		}
	}  

	return false;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_TextWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	FYapTransactions::BeginModify(LOCTEXT("SetAudioAsset", "Set audio asset"), GetDialogueNodeMutable());

	GetFragmentMutable().GetMatureBitMutable().SetDialogueAudioAsset(Object);

	FYapTransactions::EndModify();	
}

// ================================================================================================
// SPEAKER WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateSpeakerWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize();
	int32 BorderSize = 2;

	FYapFragment& Fragment = GetFragmentMutable();
	
	const UObject* SpeakerAsset = Fragment.GetSpeaker(EYapLoadContext::AsyncEditorOnly);

	return SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::MediumDetail)
	.HighDetail()
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			SAssignNew(SpeakerDropTarget, SAssetDropTarget)
			.bSupportsMultiDrop(false)
			.OnAreAssetsAcceptableForDrop(this, &ThisClass::IsDroppedAsset_YapSpeaker)
			.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_SpeakerWidget)
			[
				SNew(SYapButtonPopup)
				.PopupPlacement(MenuPlacement_BelowAnchor)
				//.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_SpeakerWidget, Character))
				.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_SpeakerWidget, SpeakerAsset))
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SpeakerPopup)
				.ButtonContent()
				[
					CreateSpeakerImageWidget(PortraitSize, BorderSize)
				]
			]
		]
		.LowDetail()
		[
			CreateSpeakerImageWidget(PortraitSize, BorderSize)
		]
	]
	.LowDetail()
	[
		CreateSpeakerImageWidget_LowDetail(PortraitSize, BorderSize)
	];
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	bool bIsValid = IsDroppedAsset_YapSpeaker(AssetDatas);

	if (!bIsValid)
	{
		return;
	}

	UObject* AssetObject = AssetDatas[0].GetAsset();

	FYapScopedTransaction Transaction(YapEditor::Event::None, LOCTEXT("SetSpeakerCharacter", "Set speaker character"), GetDialogueNodeMutable());
	GetFragmentMutable().SetSpeakerNew(AssetObject);
	
	if (!AssetObject->Implements<UYapCharacterInterface>())
	{
		// Verify if this asset has been added to project settings
		bool bAssetAlreadyRegistered = false;

		UBlueprint* Blueprint = Cast<UBlueprint>(AssetObject);
		TSoftClassPtr<UObject> GeneratedClassAsSoft;
		
		if (Blueprint)
		{
			GeneratedClassAsSoft = TSoftClassPtr<UObject>(Blueprint->GeneratedClass);

			const TArray<TSoftClassPtr<UObject>>& ProjectSettingsAdditionalCharacters = UYapProjectSettings::GetAdditionalCharacterClasses();

			for (const TSoftClassPtr<UObject>& SettingCharacter : ProjectSettingsAdditionalCharacters)
			{
				if (SettingCharacter == GeneratedClassAsSoft)
				{
					bAssetAlreadyRegistered = true;
					break;
				}
			}
		}

		if (!bAssetAlreadyRegistered)
		{
			if (EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AddBlueprintToCharacterClassSettings_Message", "Would you like to add this to Yap's \"Additional Character Classes\" setting?\nThis will allow Yap to discover this in character dropdowns.")))
			{
				//FYapScopedTransaction Transaction2(YapEditor::Event::None, LOCTEXT("AddBlueprintToCharacterClassSettings_Transaction", "Add blueprint to Yap Characters in project settings"), GetMutableDefault<UYapProjectSettings>());

				UYapProjectSettings::AddAdditionalCharacterClass(GeneratedClassAsSoft);
			}
		}
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateSpeakerImageWidget(int32 PortraitSize, int32 BorderSize)
{
	float SpeakerWidgetSize = GetSpeakerWidgetSize(PortraitSize, BorderSize);

	return SNew(SBox)
	.WidthOverride(SpeakerWidgetSize)
	.HeightOverride(SpeakerWidgetSize)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
			.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_SpeakerImage)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(BorderSize)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
			.Image(this, &ThisClass::Image_SpeakerImage)
			.ToolTipText(this, &ThisClass::ToolTipText_SpeakerWidget)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &ThisClass::Text_SpeakerWidget)
			.SimpleTextMode(true)
			.Font(FCoreStyle::GetDefaultFontStyle("Normal", 8))
			.ColorAndOpacity(YapColor::Red)
			.Justification(ETextJustify::Center)
		]
	];
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_SpeakerImage() const
{
	const UObject* Speaker = GetFragmentMutable().GetSpeaker(EYapLoadContext::DoNotLoad);
	
	FLinearColor Color = IYapCharacterInterface::GetColor(Speaker);
	
	Color.A *= UYapDeveloperSettings::GetPortraitBorderAlpha();

	if (!GetDialogueNode()->IsPlayerPrompt())
	{
		Color = YapColor::Desaturate(Color, 0.25f);
		Color.A *= 0.65f;
	}

	if (SpeakerDropTarget.IsValid() && SpeakerDropTarget->IsHovered())
	{
		Color /= YapColor::LightGray;
	}
	
	return Color;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage() const
{
	const UObject* SpeakerAsset = GetFragmentMutable().GetSpeaker(EYapLoadContext::AsyncEditorOnly);

	//const UYapCharacter* Speaker = GetFragmentMutable().GetSpeaker(EYapLoadContext::AsyncEditorOnly);
	const FGameplayTag& MoodTag = GetFragment().GetMoodTag();
	
	TSharedPtr<FSlateImageBrush> PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(SpeakerAsset, MoodTag);

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush.Get();
	}
	else
	{
		return FYapEditorStyle::GetImageBrush(YapBrushes.None);
	}
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_SpeakerWidget() const
{
	FYapFragment& Fragment = GetFragmentMutable();

	if (!Fragment.HasSpeakerAssigned())
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker Unset");
	}
	
	if (Fragment.IsSpeakerPendingLoad())
	{
		LOCTEXT("Loading...", "Loading...");
	}
	
	const UObject* Speaker = GetFragmentMutable().GetSpeaker(EYapLoadContext::DoNotLoad);	

	if (!IsValid(Speaker))
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker Unset");
	}

	FText SpeakerName = IYapCharacterInterface::GetName(Speaker);
	
	if (SpeakerName.IsEmpty())
	{
		SpeakerName = LOCTEXT("Unnamed", "Unnamed");
	}

	// TODO minor bug this is being reached when it shouldn't be
	return FText::Format(LOCTEXT("SpeakerImageMissing_Label", "{0}"), SpeakerName);
}

FText SFlowGraphNode_YapFragmentWidget::Text_SpeakerWidget() const
{
	FYapFragment& Fragment = GetFragmentMutable();

	if (!Fragment.HasSpeakerAssigned())
	{
		return LOCTEXT("SpeakerUnset_Label","Unset");
	}
	
	if (Fragment.IsSpeakerPendingLoad())
	{
		return LOCTEXT("Loading...", "Loading...");
	}

	const UObject* Speaker = Fragment.GetSpeaker(EYapLoadContext::DoNotLoad);

	// TODO I hate reading this twice although it probably isn't that expensive. Maybe I can toggle this on/off by events somehow later.		
	if (Image_SpeakerImage() == nullptr)
	{

		FText SpeakerName = IYapCharacterInterface::GetName(Speaker);
		
		if (SpeakerName.IsEmpty())
		{
			SpeakerName = LOCTEXT("Unnamed", "Unnamed");
		}
		
		return FText::Format(LOCTEXT("SpeakerImageMissing_Label", "{0}\n\nNo Portrait"), SpeakerName);
	}
	
	return FText::GetEmpty();
}

float SFlowGraphNode_YapFragmentWidget::GetSpeakerWidgetSize(int32 PortraitSize, int32 BorderSize) const
{
	int32 MinHeight = 72; // Hardcoded minimum height - this keeps pins aligned with the graph snapping grid

	int32 FinalHeight = FMath::Max(PortraitSize + 2 * BorderSize, MinHeight);

	return FinalHeight;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateSpeakerImageWidget_LowDetail(int32 PortraitSize, int32 BorderSize)
{
	float SpeakerWidgetSize = GetSpeakerWidgetSize(PortraitSize, BorderSize);

	return SNew(SBox)
	.WidthOverride(SpeakerWidgetSize)
	.HeightOverride(SpeakerWidgetSize)
	[
		SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Rounded))
		.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_SpeakerImage)
	];
}

// ================================================================================================
// DIRECTED AT WIDGET
// ================================================================================================

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDirectedAtWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize() / 3;
	
	return SAssignNew(DirectedAtWidget, SBorder)
	.Cursor(EMouseCursor::Default)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FilledCircle))
	.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_DirectedAtImage)
	.Visibility_Lambda( [this] ()
	{
		return (FragmentTextOverlay.IsValid() && FragmentTextOverlay->IsHovered()) || IsValid(GetFragmentMutable().GetDirectedAt(EYapLoadContext::DoNotLoad)) ? EVisibility::Visible : EVisibility::Collapsed;
	} )
	.Padding(3)
	[
		SNew(SBox)
		.WidthOverride(PortraitSize + 3)
		.HeightOverride(PortraitSize + 3)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
			.HighDetail()
			[
				SNew(SAssetDropTarget)
				.bSupportsMultiDrop(false)
				.OnAreAssetsAcceptableForDrop(this, &ThisClass::IsDroppedAsset_YapSpeaker)
				.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_DirectedAtWidget)
				[
					SNew(SYapButtonPopup)
					.PopupPlacement(MenuPlacement_BelowAnchor)
					.OnClicked(this, &ThisClass::OnClicked_DirectedAtWidget)
					.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_DirectedAtWidget))
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
					.ButtonBackgroundColor(YapColor::DarkGray)
					.ButtonContent()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
						.Image(this, &ThisClass::Image_DirectedAtWidget)
					]
				]
			]
			.LowDetail()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
				.Image(this, &ThisClass::Image_DirectedAtWidget)
			]
		]
	];
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage() const
{
	const UObject* DirectedAt = GetFragmentMutable().GetDirectedAt(EYapLoadContext::DoNotLoad);
	
	FLinearColor Color = IYapCharacterInterface::GetColor(DirectedAt);

	if (DirectedAtWidget.IsValid() && DirectedAtWidget->IsHovered())
	{
		Color /= YapColor::LightGray;
	}
	
	return Color;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}
	
	UYapCharacterAsset* Character = Cast<UYapCharacterAsset>(AssetDatas[0].GetAsset());
	
	if (Character)
	{
		FYapScopedTransaction Transaction(YapEditor::Event::None, LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNodeMutable());
		GetFragmentMutable().SetDirectedAt(Character);
	}
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_DirectedAtWidget()
{
	if (bCtrlPressed)
	{
		FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNodeMutable());

		GetFragmentMutable().SetDirectedAt(nullptr);

		FYapTransactions::EndModify();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_DirectedAtWidget()
{
	return SNew(SBorder)
	.Padding(1, 1, 1, 1)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(6, 0, 6, 0)
		[
			SNew(SBox)
			.WidthOverride(15) // Rotated widgets are laid out per their original transform, use negative padding and a width override for rotated text
			.Padding(-80)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DirectedAt_PopupLabel", "DIRECTED AT"))
				.SimpleTextMode(true)
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-90.0f))))
				.Font(YapFonts.Font_SectionHeader)
			]
		]
		
		+ SHorizontalBox::Slot()
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacterAsset::StaticClass()})
			.AllowClear(true)
			.InitialObject(GetFragmentMutable().GetDirectedAt(EYapLoadContext::AsyncEditorOnly))
			.OnSet(this, &ThisClass::OnSetNewDirectedAtAsset)
		]
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget() const
{
	//const UYapCharacter* Character = GetFragmentMutable().GetDirectedAt(EYapLoadContext::AsyncEditorOnly);
	const UObject* SpeakerAsset = GetFragmentMutable().GetDirectedAt(EYapLoadContext::AsyncEditorOnly);

	TSharedPtr<FSlateImageBrush> PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(SpeakerAsset, FGameplayTag::EmptyTag);

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush.Get();
	}
	else
	{
		return FYapEditorStyle::GetImageBrush(YapBrushes.None);
	}
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNodeMutable());

	GetFragmentMutable().SetDirectedAt(Cast<UYapCharacterAsset>(AssetData.GetAsset()));

	FYapTransactions::EndModify();
}

// ================================================================================================
// TITLE TEXT WIDGET
// ================================================================================================

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextDisplayWidget()
{
	return SNew(SBorder)
	.Cursor(EMouseCursor::Default)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
	.BorderBackgroundColor(FSlateColor::UseForeground())
	.ToolTip(nullptr)
	.Padding(0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(4, 0)
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text_Lambda( [this] () { return GetFragment().GetTitleText(GEditor->EditorWorld, GetDisplayMaturitySetting()); } )
			.ToolTipText(LOCTEXT("TitleTextDisplayWidget_ToolTipText", "Title text"))
			.ColorAndOpacity(this, &ThisClass::GetColorAndOpacityForFragmentText, YapColor::YellowGray)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility_Lambda( [this] ()
			{
				const FText& TitleText = GetFragment().GetTitleText(GEditor->EditorWorld, GetDisplayMaturitySetting());
				return TitleText.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
			})
			.Justification(ETextJustify::Center)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.SimpleTextMode(true)
			.Text_Lambda( [this] ()
			{
				if (!NeedsChildSafeData())
				{
					return LOCTEXT("TitleText_None", "Title Text (None)");
				}
							
				return GetDisplayMaturitySetting() == EYapMaturitySetting::Mature ? LOCTEXT("MatureTitleText_None", "Mature Title Text (None)") : LOCTEXT("SafeTitleText_None", "Child-Safe Title Text (None)");	
			})
			.ColorAndOpacity(YapColor::White_Glass)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(-1)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("MarqueeSelection"))
			.Visibility(this, &ThisClass::Visibility_TitleTextErrorState)
			.BorderBackgroundColor(YapColor::Red)
		]
	];
}
 
EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextErrorState() const
{
	if (!NeedsChildSafeData())
	{
		return EVisibility::Collapsed;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	if (MatureBit.HasTitleText() != ChildSafeBit.HasTitleText())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextWidgets() const
{
	return GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagWidget()
{
	auto TagAttribute = TAttribute<FGameplayTag>::CreateSP(this, &ThisClass::Value_FragmentTag);
	FString FilterString = GetDialogueNodeMutable()->GetDialogueTag().ToString();
	auto OnTagChanged = TDelegate<void(const FGameplayTag)>::CreateSP(this, &ThisClass::OnTagChanged_FragmentTag);

	return SNew(SYapGameplayTagTypedPicker)
		.Tag(TagAttribute)
		.Filter(FilterString)
		.OnTagChanged(OnTagChanged)
		.ToolTipText(LOCTEXT("FragmentTag_ToolTip", "Fragment tag"))
		.Asset(GetDialogueNodeMutable()->GetFlowAsset());
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::Value_FragmentTag() const
{
	return GetFragment().FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag(FGameplayTag GameplayTag)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeFragmentTag", "Change fragment tag"), GetDialogueNodeMutable());

	GetFragmentMutable().FragmentTag = GameplayTag;

	FYapTransactions::EndModify();

	Owner->RequestUpdateGraphNode();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_SetTimeModeButton(EYapTimeMode TimeMode)
{
	FYapTransactions::BeginModify(LOCTEXT("TimeModeChanged", "Time mode changed"), GetDialogueNodeMutable());

	GetFragmentMutable().SetTimeModeSetting(TimeMode);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

void SFlowGraphNode_YapFragmentWidget::OnValueUpdated_ManualTime(float NewValue, EYapMaturitySetting MaturitySetting)
{
	GetFragmentMutable().GetBitMutable(MaturitySetting).SetManualTime(NewValue);
	GetFragmentMutable().SetTimeModeSetting(EYapTimeMode::ManualTime);
}

// ---------------------


FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint, EYapMaturitySetting MaturitySetting) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GEditor->EditorWorld, GetDisplayMaturitySetting(), GetNodeConfig()) == TimeMode)
	{
		// Implicit match through project defaults
		return YapColor::Desaturate(ColorTint, 0.50);
	}
	
	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_PaddingButton() const
{
	if (!GetFragment().Padding.IsSet())
	{
		return YapColor::Green;
	}
	
	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GEditor->EditorWorld, GetDisplayMaturitySetting(), GetNodeConfig()) == TimeMode)
	{
		// Implicit match through project defaults
		return ColorTint;
	}
	
	return YapColor::Gray;
}

bool SFlowGraphNode_YapFragmentWidget::OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const
{
	const TArray<TSoftClassPtr<UObject>>& Classes = UYapProjectSettings::GetAudioAssetClasses();

	// TODO async loading
	if (Classes.ContainsByPredicate( [&AssetData] (const TSoftClassPtr<UObject>& Class) { return AssetData.GetClass(EResolveClass::Yes) == Class; } ))
	{
		return true;
	}

	return false;	
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset)
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
		FYapTransactions::BeginModify(LOCTEXT("SettingAudioAsset", "Setting audio asset"), GetDialogueNodeMutable());

		if (Type == EYapMaturitySetting::Mature)
		{
			GetFragmentMutable().GetMatureBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		else
		{
			GetFragmentMutable().GetChildSafeBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		
		FYapTransactions::EndModify();
	});
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.IsEnabled(bFoundAssetClass)
		.AllowedClass(bSingleDialogueAssetClass ? DialogueAssetClass : UObject::StaticClass()) // Use this feature if the project only has one dialogue asset class type
		.DisplayBrowse(true)
		.DisplayUseSelected(true)
		.DisplayThumbnail(true)
		.OnShouldFilterAsset(this, &ThisClass::OnShouldFilterAsset_AudioAssetWidget)
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
		.Visibility(this, &ThisClass::Visibility_AudioAssetErrorState, &Asset)
		.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AudioAssetErrorState, &Asset)
	];
	
	return Widget;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	if (GetAudioAssetErrorLevel(*Asset) > EYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioIDButton() const
{
	FLinearColor Color;

	switch (GetFragmentAudioErrorLevel())
	{
		case EYapErrorLevel::OK:
		{
			Color = YapColor::Noir;
			break;
		}
		case EYapErrorLevel::Warning:
		{
			Color = YapColor::Red;
			break;
		}
		case EYapErrorLevel::Error:
		{
			Color = YapColor::Red;
			break;
		}
		case EYapErrorLevel::Unknown:
		{
			Color = YapColor::Error;
			break;
		}
	}

	Color = YapColor::Desaturate(Color, 0.4f);

	if (AudioIDButton.IsValid() && AudioIDButton->IsHovered())
	{
		Color /= YapColor::LightGray;
	}
	else
	{
		Color *= YapColor::LightGray;
	}
	
	return Color;
}

// TODO handle child safe settings somehow
EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetFragmentAudioErrorLevel() const
{
	const TSoftObjectPtr<UObject>& MatureAsset = GetFragment().GetMatureBit().AudioAsset;
	const TSoftObjectPtr<UObject>& SafeAsset = GetFragment().GetChildSafeBit().AudioAsset;

	// If we need child-safe data and if any audio is set, we need both set
	if (NeedsChildSafeData())
	{
		if ((!MatureAsset.IsNull() || !SafeAsset.IsNull()) && (MatureAsset.IsNull() || SafeAsset.IsNull()))
		{
			return EYapErrorLevel::Error;
		}
	}

	// We should never allow dialogue to only contain child-safe content; child-safe content is the special case
	if (MatureAsset.IsNull() && !SafeAsset.IsNull())
	{
		return EYapErrorLevel::Unknown;
	}
	
	// Determine the error level based on project settings (does the project want all fragments to have audio?)
	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(MatureAsset);

	return ErrorLevel;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
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

EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const
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

	EYapMissingAudioErrorLevel MissingAudioBehavior = GetNodeConfig().GetMissingAudioErrorLevel();

	EYapTimeMode TimeModeSetting = GetFragment().GetTimeModeSetting();
	
	// We don't have any audio asset set. If the dialogue is set to use audio time but does NOT have an audio asset, we either indicate an error (prevent packaging) or indicate a warning (allow packaging) 
	if ((TimeModeSetting == EYapTimeMode::AudioTime) || (TimeModeSetting == EYapTimeMode::Default && GetNodeConfig().GetDefaultTimeModeSetting() == EYapTimeMode::AudioTime))
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

// ================================================================================================
// HELPERS
// ================================================================================================

const UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetDialogueNode() const
{
	return Owner->GetFlowYapDialogueNodeMutable();
}

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetDialogueNodeMutable()
{
	return const_cast<UFlowNode_YapDialogue*>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetDialogueNode());
}

const FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragmentMutable()
{
	return const_cast<FYapFragment&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetFragment());
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragmentMutable() const
{
	return const_cast<FYapFragment&>(GetFragment());
}

EYapMaturitySetting SFlowGraphNode_YapFragmentWidget::GetDisplayMaturitySetting() const
{
	if (GEditor->PlayWorld)
	{
		return UYapSubsystem::GetCurrentMaturitySetting(GEditor->PlayWorld);
	}

	if (!NeedsChildSafeData())
	{
		return EYapMaturitySetting::Mature;
	}
	
	return (bShiftPressed && Owner->IsHovered() && NeedsChildSafeData()) ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature;
}

bool SFlowGraphNode_YapFragmentWidget::NeedsChildSafeData() const
{
	return GetFragment().bEnableChildSafe;
}

bool SFlowGraphNode_YapFragmentWidget::HasAnyChildSafeData() const
{
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	return (ChildSafeBit.HasDialogueText() || ChildSafeBit.HasTitleText() || ChildSafeBit.HasAudioAsset());
}

bool SFlowGraphNode_YapFragmentWidget::HasCompleteChildSafeData() const
{
	if (!HasAnyChildSafeData())
	{
		return false;
	}

	if (!NeedsChildSafeData())
	{
		return true;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	// If any one item is set, then both items must be set
	bool bDialogueTextOK = MatureBit.HasDialogueText() == ChildSafeBit.HasDialogueText();
	bool bTitleTextOK = MatureBit.HasTitleText() == ChildSafeBit.HasTitleText(); 
	bool bAudioAssetOK = MatureBit.HasAudioAsset() == ChildSafeBit.HasAudioAsset();

	return bDialogueTextOK && bTitleTextOK && bAudioAssetOK;
}

bool SFlowGraphNode_YapFragmentWidget::FragmentIsRunning() const
{
	return GetFragment().GetStartTime() > GetFragment().GetEndTime();
}

bool SFlowGraphNode_YapFragmentWidget::FragmentRecentlyRan() const
{
	if (FragmentIsRunning())
	{
		return true;
	}
	
	UWorld* World = GEditor->GetCurrentPlayWorld(GEditor->PlayWorld);
	
	if (World && GetFragment().GetStartTime() >= 0.0)
	{
		float Elapsed = World->GetTimeSeconds() - GetFragment().GetEndTime();
		return Elapsed <= 2.0f;
	}

	return false;
}

bool SFlowGraphNode_YapFragmentWidget::IsDroppedAsset_YapSpeaker(TArrayView<FAssetData> AssetDatas) const
{
	if (AssetDatas.Num() != 1)
	{
		return false;
	}

	FAssetData& AssetData = AssetDatas[0];

	bool bSuccess = IsAsset_YapSpeaker(AssetData);

	// That function just checks if it has the interface natively (C++) or if it's listed in the project settings. We should force load it and check if it is a blueprint with the interface still.
	
	if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
	{
		if (Blueprint->GeneratedClass->ImplementsInterface(UYapCharacterInterface::StaticClass()))
		{
			bSuccess = true;
		}
	}
	
	return bSuccess;
}

bool SFlowGraphNode_YapFragmentWidget::IsAsset_YapSpeaker(const FAssetData& AssetData) const
{
	const UClass* Class = AssetData.GetClass();

	if (!Class)
	{
		return false;
	}

	if (Class->ImplementsInterface(UYapCharacterInterface::StaticClass()))
	{
		//UE_LOG(LogYapEditor, VeryVerbose, TEXT("Found valid speaker class from asset: %s"), *Class->GetName());
		return true;
	}
	
	const TArray<TSoftClassPtr<UObject>>& AllowableClasses = UYapProjectSettings::GetAdditionalCharacterClasses();

	FString AllowableClassStringTemp;
	
	if (AllowableClasses.Num() > 0)
	{
		for (TSoftClassPtr<UObject> AllowableClass : AllowableClasses)
		{
			if (!AllowableClass)
			{
				continue;
			}
			
			const FString PackageName = AssetData.PackageName.ToString();
			
			AllowableClassStringTemp = AllowableClass->GetPackage()->GetName();
			
			if (PackageName == AllowableClassStringTemp)
			{
				//UE_LOG(LogYapEditor, VeryVerbose, TEXT("Found valid speaker class from package path comparison: %s"), *Class->GetName());
				return true;
			}
		}
	}
	
	return false;
}

bool SFlowGraphNode_YapFragmentWidget::ShouldFilter_YapSpeaker(const FAssetData& AssetData) const
{
	return !IsAsset_YapSpeaker(AssetData);
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetColorAndOpacityForFragmentText(FLinearColor BaseColor) const
{
	FLinearColor Color = BaseColor;

	if (GetDisplayMaturitySetting() == EYapMaturitySetting::ChildSafe)
	{
		Color *= YapColor::LightBlue;
	}
	
	return Color;
}

const UYapNodeConfig& SFlowGraphNode_YapFragmentWidget::GetNodeConfig() const
{
	return GetDialogueNode()->GetNodeConfig();
}

// ================================================================================================
// OVERRIDES
// ================================================================================================

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetDialogueNode()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// TODO might use modifier keys to dynamically spawn widgets like fragment up/down. Might save a few CPU cycles.
	//bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
	//bShiftPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();

	bool bOwnerSelected = Owner->GetIsSelected();
	
	if (bOwnerSelected && !MoveFragmentControls.IsValid())
	{
		MoveFragmentControls = CreateFragmentControlsWidget();
		MoveFragmentControls->SetCursor(EMouseCursor::Default);
		FragmentWidgetOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(-28, 0, 0, 0)
		[
			MoveFragmentControls.ToSharedRef()
		];
	}
	else if (MoveFragmentControls.IsValid() && (!bOwnerSelected))
	{
		FragmentWidgetOverlay->RemoveSlot(MoveFragmentControls.ToSharedRef());
		MoveFragmentControls = nullptr;
	}

	/*
	if (FragmentIsRunning())
	{
		if (RowHighlight == nullptr)
		{
			RowHighlight = CreateFragmentHighlightWidget();
			FragmentTextOverlay->AddSlot()
			[
				RowHighlight.ToSharedRef()
			];
		}
	}
	else
	{
		if (RowHighlight != nullptr)
		{
			FragmentTextOverlay->RemoveSlot(RowHighlight.ToSharedRef());
			RowHighlight = nullptr;
		}
	}
	*/
	
	if (DirectedAtWidget.IsValid())
	{
		float DesiredRenderOpacity = IsHovered() ? 1.0f : 0.6f;

		if (DirectedAtWidget->GetRenderOpacity() != DesiredRenderOpacity)
		{
			DirectedAtWidget->SetRenderOpacity(DesiredRenderOpacity);
		}
	}

	bChildSafeCheckBoxHovered = ChildSafeCheckBox->IsHovered();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_FragmentDataIcon() const
{
	return GetFragment().HasData() ? YapColor::LightBlue : YapColor::Transparent;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateRightFragmentPane()
{
	SAssignNew(StartPinBox, SBox)
	.WidthOverride(16)
	.HeightOverride(16);
	
	SAssignNew(EndPinBox, SBox)
	.WidthOverride(16)
	.HeightOverride(16);

	bool bIsPlayerPrompt = GetDialogueNodeMutable()->IsPlayerPrompt();
	
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Top)
	.Padding(4, 0, 2, 4)
	.AutoHeight()
	[
		SAssignNew(PromptOutPinBox, SBox)
	]
	+ SVerticalBox::Slot()
	.HAlign(bIsPlayerPrompt ? HAlign_Right : HAlign_Center)
	.VAlign(VAlign_Top)
	//.Padding(3, 0, 2, 0)
	//.Padding(3, -5, -2, 0)
	.Padding(bIsPlayerPrompt ? FMargin(3, -5, -2, 0) : FMargin(3, 0, 2, 0))
	.AutoHeight()
	[
		SNew(SImage)
		.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FragmentData))
		.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_FragmentDataIcon)
		.DesiredSizeOverride(FVector2D(12, 12))
	]
	+ SVerticalBox::Slot()
	[
		SNew(SSpacer)
	]
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Bottom)
	.Padding(3, 0, 2, 0)
	.AutoHeight()
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			/*
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(16)
				.HeightOverride(8)
				.Visibility(this, &ThisClass::Visibility_EnableOnStartPinButton)
				[
					SNew(SButton)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton) // TODO custom style
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &ThisClass::OnClicked_EnableOnStartPinButton)
					.ButtonColorAndOpacity(YapColor::DimGray_Trans)
					.ToolTipText(LOCTEXT("ClickToEnableOnStartPin_Label", "Click to enable 'On Start' Pin"))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				StartPinBox.ToSharedRef()
			]
			*/
			StartPinBox.ToSharedRef()
		]
		.LowDetail()
		[
			StartPinBox.ToSharedRef()
		]
	]
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Bottom)
	.Padding(3, 0, 2, 0)
	.AutoHeight()
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			/*
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(16)
				.HeightOverride(8)
				.Visibility(this, &ThisClass::Visibility_EnableOnEndPinButton)
				[
					SNew(SButton)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton) // TODO custom style
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &ThisClass::OnClicked_EnableOnEndPinButton)
					.ButtonColorAndOpacity(YapColor::DimGray_Trans)
					.ToolTipText(LOCTEXT("ClickToEnableOnEndPin_Label", "Click to enable 'On End' Pin"))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				EndPinBox.ToSharedRef()
			]
			*/
			EndPinBox.ToSharedRef()
		]
		.LowDetail()
		[
			EndPinBox.ToSharedRef()
		]
	];
}

TSharedPtr<SBox> SFlowGraphNode_YapFragmentWidget::GetPinContainer(const FFlowPin& Pin)
{
	if (Pin == GetFragmentMutable().GetStartPin())
	{
		return StartPinBox;
	}

	if (Pin == GetFragmentMutable().GetEndPin())
	{
		return EndPinBox;
	}

	if (Pin == GetFragmentMutable().GetPromptPin())
	{
		return PromptOutPinBox;
	}

	return nullptr;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnStartPinButton() const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
	
	return GetFragment().UsesStartPin() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnEndPinButton() const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
	
	return GetFragment().UsesEndPin() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnStartPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnStart Pin"), GetDialogueNodeMutable());

	GetFragmentMutable().bShowOnStartPin = true;
	
	GetDialogueNodeMutable()->ForceReconstruction();
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnEnd Pin"), GetDialogueNodeMutable());

	GetFragmentMutable().bShowOnEndPin = true;

	GetDialogueNodeMutable()->ForceReconstruction();
	
	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

// ================================================================================================
// MOOD TAG SELECTOR WIDGET
// ================================================================================================

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodTagSelectorWidget()
{
	FGameplayTag SelectedMoodTag = GetCurrentMoodTag();

	TSharedRef<SUniformWrapPanel> MoodTagSelectorPanel = SNew(SUniformWrapPanel)
		.NumColumnsOverride(4);

	MoodTagSelectorPanel->AddSlot()
	[
		CreateMoodTagMenuEntryWidget(FGameplayTag::EmptyTag, SelectedMoodTag == FGameplayTag::EmptyTag)
	];

	for (const FGameplayTag& MoodTag : GetNodeConfig().GetMoodTags())
	{
		if (!MoodTag.IsValid())
		{
			UE_LOG(LogYap, Warning, TEXT("Warning: Portrait keys contains an invalid entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodTag == SelectedMoodTag;
		
		MoodTagSelectorPanel->AddSlot()
		[
			CreateMoodTagMenuEntryWidget(MoodTag, bSelected)
		];
	}
	
	// TODO ensure that system works and displays labels if user does not supply icons but only FNames. Use Generic mood icon?
	return SNew(SComboButton)
		.Cursor(EMouseCursor::Default)
		.HasDownArrow(false)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SimpleYapButton) // TODO fix style
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		//.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.5f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		//.OnMenuOpenChanged(this, &ThisClass::OnMenuOpenChanged_MoodTagSelector)
		.ToolTipText(this, &ThisClass::ToolTipText_MoodTagSelector)
		.ForegroundColor(this, &ThisClass::ForegroundColor_MoodTagSelectorWidget)
		.ButtonContent()
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(this, &ThisClass::Image_MoodTagSelector)
		]
		.MenuContent()
		[
			MoodTagSelectorPanel
		];
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodTagSelector() const
{
	TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(GetFragment().GetMoodTag());

	if (TagNode.IsValid())
	{
		return FText::FromName(TagNode->GetSimpleTagName());
	}

	return LOCTEXT("Default", "Default");
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodTagSelectorWidget() const
{
	if (GetFragment().GetMoodTag() == FGameplayTag::EmptyTag)
	{
		return YapColor::Button_Unset();
	}
	
	return YapColor::White_Trans;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_MoodTagSelector() const
{
	return GetDialogueNode()->GetNodeConfig().GetMoodTagBrush(GetCurrentMoodTag());
	//return GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodTagBrush(GetCurrentMoodTag());
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::GetCurrentMoodTag() const
{
	return GetFragment().GetMoodTag();
}

// ================================================================================================
// MOOD TAG MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodTagMenuEntryWidget(FGameplayTag MoodTag, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	//TSharedPtr<FSlateImageBrush> MoodTagBrush = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodTagIcon(MoodTag);
	TSharedPtr<FSlateImageBrush> MoodTagBrush = GetDialogueNode()->GetNodeConfig().GetMoodTagIcon(MoodTag);
	
	if (MoodTag.IsValid())
	{
		HBox->AddSlot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(MoodTagBrush.Get())
		];
	}

	FText ToolTipText;
	
	if (MoodTag.IsValid())
	{
		TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(MoodTag);
		ToolTipText = FText::FromName(TagNode->GetSimpleTagName());
	}
	else
	{
		ToolTipText = LOCTEXT("Default", "Default");
	}
	
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.ContentPadding(FMargin(4, 4))
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ClickMethod(EButtonClickMethod::MouseDown)
	.OnClicked(this, &ThisClass::OnClicked_MoodTagMenuEntry, MoodTag)
	.ToolTipText(ToolTipText)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(-3)
		[
			SNew(SBorder)
			.Visibility_Lambda([this, MoodTag]()
			{
				if (GetFragmentMutable().GetMoodTag() == MoodTag)
				{
					return EVisibility::Visible;
				}
				
				return EVisibility::Collapsed;
			})
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_RoundedSquare))
			.BorderBackgroundColor(YapColor::White_Trans)
		]
		+ SOverlay::Slot()
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(MoodTagBrush.Get())
		]
	];
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_MoodTagMenuEntry(FGameplayTag NewValue)
{	
	FYapTransactions::BeginModify(LOCTEXT("NodeMoodTagChanged", "Portrait Key Changed"), GetDialogueNodeMutable());

	GetFragmentMutable().SetMoodTag(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE