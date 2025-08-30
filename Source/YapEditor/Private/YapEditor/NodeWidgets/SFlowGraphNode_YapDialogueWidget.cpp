// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "GameplayTagsEditorModule.h"
#include "NodeFactory.h"
#include "SGraphPanel.h"
#include "SLevelOfDetailBranchNode.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/YapEditorLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/SlateWidgets/SYapActivationCounterWidget.h"
#include "YapEditor/SlateWidgets/SYapConditionDetailsViewWidget.h"
#include "YapEditor/SlateWidgets/SYapConditionsScrollBox.h"
#include "YapEditor/SlateWidgets/SYapGraphPinExec.h"
#include "YapEditor/SlateWidgets/SYapProgressionSettingsWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

constexpr int32 YAP_MIN_NODE_WIDTH = 275;
constexpr int32 YAP_DEFAULT_NODE_WIDTH = 400;

// TODO move to a proper style
FButtonStyle SFlowGraphNode_YapDialogueWidget::MoveFragmentButtonStyle;
bool SFlowGraphNode_YapDialogueWidget::bStylesInitialized = false;

void SFlowGraphNode_YapDialogueWidget::AddOverlayWidget(TSharedPtr<SWidget> ParentWidget, TSharedPtr<SWidget> OverlayWidget, bool bClearExisting)
{
	if (bClearExisting)
	{
		OverlayWidgets.Empty();
	}
	
	OverlayWidgets.Emplace(FYapWidgetOverlay(ParentWidget, OverlayWidget));

	SetNodeSelected();
}

void SFlowGraphNode_YapDialogueWidget::RemoveOverlayWidget(TSharedPtr<SWidget> OverlayWidget)
{
	OverlayWidgets.RemoveAll( [OverlayWidget] (FYapWidgetOverlay& X) { return X.Overlay == OverlayWidget;} );
}

void SFlowGraphNode_YapDialogueWidget::ClearOverlayWidgets()
{
	OverlayWidgets.Empty();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	PreConstruct(InArgs, InNode);
	
	SFlowGraphNode::Construct(InArgs, InNode);
	
	PostConstruct(InArgs, InNode);
}

// ------------------------------------------
// CONSTRUCTION
// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::PreConstruct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = YapColor::DarkGray;

	ConnectedBypassPinColor = YapColor::LightBlue;
	DisconnectedBypassPinColor = YapColor::Red;
	
	ConnectedFragmentPinColor = YapColor::White;
	DisconnectedFragmentPinColor = YapColor::Red;
	
	bDragMarkerVisible = false;
	
	FocusedFragmentIndex.Reset();
	
	if (!bStylesInitialized)
	{
		MoveFragmentButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle");

		// Button colors
		MoveFragmentButtonStyle.Normal.TintColor = YapColor::Noir_Trans;
		MoveFragmentButtonStyle.Hovered.TintColor = YapColor::DarkGray_Trans;
		MoveFragmentButtonStyle.Pressed.TintColor = YapColor::DarkGrayPressed_Trans;

		// Text colors
		MoveFragmentButtonStyle.NormalForeground = YapColor::LightGray;
		MoveFragmentButtonStyle.HoveredForeground = YapColor::White;
		MoveFragmentButtonStyle.PressedForeground = YapColor::LightGrayPressed;

		bStylesInitialized = true;
	}
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::PostConstruct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	
}

// ------------------------------------------------------------------------------------------------
int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationCount();
}

// ------------------------------------------------------------------------------------------------
int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationLimit();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeActivationLimit", "Change activation limit"), GetDialogueNodeMutable());

	GetDialogueNodeMutable()->SetNodeActivationLimit(FCString::Atoi(*Text.ToString()));

	FYapTransactions::EndModify();
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::Text_DialogueID() const
{
	FName IDName = GetFlowYapDialogueNode()->GetDialogueID();

	if (IDName == NAME_None)
	{
		return FText::GetEmpty();
	}
	
	return FText::FromName(IDName);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnTextChanged_DialogueID(const FText& NewText, ETextCommit::Type CommitType)
{
	FString NewValueString = NewText.ToString();
	
	FText ErrorText;
	if (!Yap::EditorFuncs::IsValidEntryNameString(NewValueString, ErrorText))
	{
		Yap::EditorFuncs::PostNotificationInfo_Warning(LOCTEXT("Error_Title", "Error"), ErrorText);
		return;
	}

	FName NewName(NewValueString);
	
	FYapTransactions::BeginModify(LOCTEXT("ChangeFragmentID", "Change fragment ID"), GetDialogueNodeMutable());

	GetDialogueNodeMutable()->DialogueID = NewName;

	FYapTransactions::EndModify();
}

// ------------------------------------------------------------------------------------------------
FOptionalSize SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth() const
{
	const float GraphGridSize = 16;
	return FMath::Max(YAP_MIN_NODE_WIDTH + GetNodeConfig().GetPortraitSize(), YAP_DEFAULT_NODE_WIDTH + GraphGridSize * GetNodeConfig().GetDialogueWidthAdjustment());
}

// ------------------------------------------------------------------------------------------------
FOptionalSize SFlowGraphNode_YapDialogueWidget::GetMaxTitleWidth() const
{
	const int32 TITLE_LEFT_RIGHT_EXTRA_WIDTH = 44;

	return GetMaxNodeWidth().Get() - TITLE_LEFT_RIGHT_EXTRA_WIDTH;
}

bool SFlowGraphNode_YapDialogueWidget::UseLowDetail(EGraphRenderingLOD::Type LOD) const
{
	if (const SGraphPanel* MyOwnerPanel = GetOwnerPanel().Get())
	{
		//auto CurrentLOD = MyOwnerPanel->GetCurrentLOD();

		//float Zoom = MyOwnerPanel->GetZoomAmount();
		
		return (MyOwnerPanel->GetCurrentLOD() < LOD);
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnClick_NewConditionButton(int32 FragmentIndex)
{
	FYapTransactions::BeginModify(LOCTEXT("AddCondition", "Add condition"), GetDialogueNodeMutable());

	if (FragmentIndex == INDEX_NONE)
	{
		GetDialogueNodeMutable()->GetConditionsMutable().Add(nullptr);
	}
	else
	{
		GetFragmentMutable(FragmentIndex).GetConditionsMutable().Add(nullptr);
	}

	FYapTransactions::EndModify();

	GetDialogueNodeMutable()->ForceReconstruction();
}

// ------------------------------------------
// WIDGETS

// ================================================================================================
// TITLE WIDGET
// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged()
{
	GraphNode->ReconstructNode();

	ClearOverlayWidgets();
	
	UpdateGraphNode();
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	// TODO this duplicates code from YapBroker
	UFlowAsset* FlowAsset = GetFlowGraphNode_YapDialogue()->GetFlowAsset();
	
	FString Path = FPackageName::GetLongPackagePath(GetFlowGraphNode_YapDialogue()->GetFlowAsset()->GetPackage()->GetName());
	Path.RemoveFromStart("/Game/");
	Path.ReplaceCharInline('/', '.');
	
	FString AssetName = FlowAsset->GetName();
			
	TOptional<EYapInterruptibleFlags>* InterruptibleFlagsRaw = &GetDialogueNodeMutable()->InterruptibleFlags;
	
	TOptional<EYapAutoAdvanceFlags>* AutoAdvanceSettingRaw = &GetDialogueNodeMutable()->AutoAdvanceFlags;

	TSharedRef<SWidget> ProgressionPopupButton = SNew(SYapProgressionSettingsWidget)
		.DialogueNode(GetDialogueNodeMutable())
		.SkippableSettingRaw(InterruptibleFlagsRaw)
		.AutoAdvanceSettingRaw(AutoAdvanceSettingRaw);

	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

	Row->AddSlot()
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	.Padding(-10, -8, 14, -8)
	[
		SNew(SLevelOfDetailBranchNode)
		//.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			SNew(SYapActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit))
			.ActivationCount(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount)
			.ActivationLimit(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit)
			.FontHeight(10)
		]
		.LowDetail()
		[
			SNew(SSpacer)
			.Size(20)
		]
	];

	Row->AddSlot()
	.HAlign(HAlign_Fill)
	.Padding(-10,0,2,0)
	[
		SNew(SLevelOfDetailBranchNode)
		//.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			SAssignNew(DialogueConditionsScrollBox, SYapConditionsScrollBox)
			.DialogueNode_Lambda( [this] () { return GetDialogueNodeMutable(); } )
			.ConditionsArrayProperty(FindFProperty<FArrayProperty>(UFlowNode_YapDialogue::StaticClass(), GET_MEMBER_NAME_CHECKED(UFlowNode_YapDialogue, Conditions)))
			.ConditionsContainer_Lambda( [this] () { return GetDialogueNodeMutable(); } )
			.OnConditionsArrayChanged(this, &SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged)
		]
	];

	Row->AddSlot()
	.HAlign(HAlign_Right)
	.Padding(2,0,7,0)
	.AutoWidth()
	.VAlign(VAlign_Fill)
	[
		SNew(SLevelOfDetailBranchNode)
		//.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
		.HighDetail()
		[
			SNew(SEditableText)
			.HintText(LOCTEXT("FragmentID_HintText", "ID"))
			.Text(this, &SFlowGraphNode_YapDialogueWidget::Text_DialogueID)
			.OnTextCommitted(this, &SFlowGraphNode_YapDialogueWidget::OnTextChanged_DialogueID)
			.SelectAllTextWhenFocused(true)
			.SelectAllTextOnCommit(true)
		]
	];
	
	Row->AddSlot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(2, -2, -25, -2)
	[
		SNew(SBox)
		.WidthOverride(20)
		.HAlign(HAlign_Center)
		.Visibility_Lambda( [this] () { return GetFlowYapDialogueNode()->GetNodeType() == EYapDialogueNodeType::TalkAndAdvance ? EVisibility::Collapsed : EVisibility::Visible; } )
		[
			ProgressionPopupButton
		]
	];
	
	return SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
	.HighDetail()
	[
		SNew(SBox)
		.Visibility_Lambda([]() { return GEditor->PlayWorld == nullptr ? EVisibility::Visible : EVisibility::HitTestInvisible; })
		.MaxDesiredWidth(this, &SFlowGraphNode_YapDialogueWidget::GetMaxTitleWidth)
		[
			Row
		]
	]
	.LowDetail()
	[
		SNew(SBox)
		.HeightOverride(16)
	];
}

// ================================================================================================
// NODE CONTENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	TSharedPtr<SVerticalBox> Content; 

	return SNew(SBox)
	.WidthOverride(this, &SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth)
	.Visibility_Lambda([]() { return GEditor->PlayWorld == nullptr ? EVisibility::Visible : EVisibility::HitTestInvisible; })
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.Padding(-4, -24, 0, 2) // The top bar is 26 pixels high
		[
			// This is a helper identification bar on the left of the node, colored for the type group. Invisible for default type group.
			SNew(SBox)
			.WidthOverride(4)
			.Visibility_Lambda( [this] () { return GetFlowYapDialogueNode()->GetNodeConfig().GetGroupColor().IsSet() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
			[
				SNew(SImage)
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
				.ColorAndOpacity_Lambda( [this] () { return GetFlowYapDialogueNode()->GetNodeConfig().GetGroupColor().Get(YapColor::White).Desaturate(0.25F); }) // Epic's Desaturate command sets opacity as well, which I want for this
			]
		]
		+ SOverlay::Slot()
		[
			SAssignNew(Content, SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 3, 0, 4)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.Padding(0, -2, 0, -2)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
					.Visibility_Lambda( [this] ()
					{
						EFlowNodeState CurrentState = GetFlowYapDialogueNode()->GetActivationState();

						if (CurrentState == EFlowNodeState::Active)
						{
							return EVisibility::SelfHitTestInvisible;
						}
						
						bool bActivationLimitsOK = GetFlowYapDialogueNode()->CheckActivationLimits();
						
						if (!bActivationLimitsOK)
						{
							return EVisibility::SelfHitTestInvisible;
						}
						
						return GetFlowYapDialogueNode()->GetNodeType() == EYapDialogueNodeType::Talk ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible;
					})
					.ColorAndOpacity_Lambda( [this] ()
					{
						EFlowNodeState CurrentState = GetFlowYapDialogueNode()->GetActivationState();

						if (CurrentState == EFlowNodeState::Active)
						{
							return YapColor::White_Glass;
						}

						bool bActivationLimitsOK = GetFlowYapDialogueNode()->CheckActivationLimits();
						
						if (!bActivationLimitsOK)
						{
							return YapColor::Red_Glass;
						}

						switch (GetFlowYapDialogueNode()->GetNodeType())
						{
							case EYapDialogueNodeType::Talk:
							{
								return YapColor::Transparent;
							}
							case EYapDialogueNodeType::TalkAndAdvance:
							{
								return YapColor::LightBlue_Glass.Desaturate(0.4f);
							}
							case EYapDialogueNodeType::PlayerPrompt:
							{
								return YapColor::LightBlue_Glass;	
							}
							default:
							{
								return YapColor::Transparent;
							}
						}
					} )
				]
				+ SOverlay::Slot()
				[
					CreateContentHeader()
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateFragmentBoxes()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateContentFooter()
			]	
		]
	];
}

// ------------------------------------------------------------------------------------------------
FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton() const
{
	if (!GetFlowYapDialogueNode()->CheckActivationLimits() && GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active)
	{
		return YapColor::Red;
	}

	return YapColor::DarkGray;
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::Text_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return LOCTEXT("DialogueFragmentSequencingButtonText_RunAll", "Run All");
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return LOCTEXT("DialogueFragmentSequencingButtonText_RunTilFailure", "Run til Failure");
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return LOCTEXT("DialogueFragmentSequencingButtonText_SelectOne", "Select One");
		}
		case EYapDialogueTalkSequencing::SelectRandom:
		{
			return LOCTEXT("DialogueFragmentSequencingButtonText_SelectRandom", "Select Random");
		}
		default:
		{
			return LOCTEXT("DialogueFragmentSequencingButtonText_Error", "*** ERROR ***");
		}
	}
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt()
{
	{
		FYapScopedTransaction T("TODO", LOCTEXT("TransactionText_TogglePlayerPrompt", "Toggle Player Prompt"), GetDialogueNodeMutable());

		bool bHasOutputConnections = false;
		// Get all output pins of this node and iterate them
		for (UEdGraphPin* Pin : FlowGraphNode_YapDialogue->OutputPins)
		{
			if (Pin->HasAnyConnections())
			{
				bHasOutputConnections = true;
				break;
			}
		}
		
		if (GetDialogueNodeMutable()->ToggleNodeType(bHasOutputConnections))
		{
			GetDialogueNodeMutable()->ForceReconstruction();
		}

		NodeHeaderButtonToolTip->SetText(Text_NodeHeader());
	}
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentHeader()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4, 0, 0, 0)
	[
		SAssignNew(DialogueInputBoxArea, SBox)
	]
	+ SHorizontalBox::Slot()
	.FillWidth(1.0)
	.Padding(-2, 0, 0, 0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.SimpleTextMode(true)
			.Visibility_Lambda([this] ()
			{
				bool bShouldShowGroupLabel = true;

				/*
				if (GetDomainConfig().IsDefault())
				{
					 bShouldShowGroupLabel = false;
				}
				else if (GetFlowYapDialogueNode()->IsPlayerPrompt())
				{
					bShouldShowGroupLabel = !GetDomainConfig().bPromptModeTitle_Override;
				}
				else
				{
					bShouldShowGroupLabel = !GetDomainConfig().bTalkModeTitle_Override;
				}
				*/
				
				return bShouldShowGroupLabel ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_GroupLabel)
			.Text_Lambda( [this] () { return Text_GroupLabel(); })
			.ColorAndOpacity(YapColor::Gray)
		]
		+ SOverlay::Slot()
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
			.HighDetail()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 2, 0)
				[
					SAssignNew(NodeHeaderButton, SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					//.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton)
					.ContentPadding(FMargin(4, 0, 4, 0))
					//.ButtonColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton)
					.ForegroundColor(YapColor::White)
					.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt)
					.ToolTipText(LOCTEXT("ToggleDialogueModeToolTip", "Toggle between player prompt or normal speech"))
					[
						SAssignNew(NodeHeaderButtonToolTip, STextBlock)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeHeader)
						.SimpleTextMode(true)
						.Text_Lambda( [this] () { return Text_NodeHeader(); })
						//.ColorAndOpacity_Lambda( [this] () { return GetDomain().GetGroupColor(); })
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 2, 0)
				[
					SAssignNew(FragmentSequencingButton_Box, SBox)
					.Visibility(Visibility_FragmentSequencingButton())
					.WidthOverride(120)
					.Padding(0, 0, 0, 0)
					[
						SAssignNew(FragmentSequencingButton_Button, SButton)
						.Cursor(EMouseCursor::Default)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(FMargin(2, 1, 2, 1))
						.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton)
						.ToolTipText(ToolTipText_FragmentSequencingButton())
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(0, 0, 4, 0)
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SAssignNew(FragmentSequencingButton_Image, SImage)
								.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
								.DesiredSizeOverride(FVector2D(16, 16))
								.Image(Image_FragmentSequencingButton())
							]
							+ SHorizontalBox::Slot()
							.Padding(4, 0, 0, 0)
							.VAlign(VAlign_Center)
							.AutoWidth()
							[
								SAssignNew(FragmentSequencingButton_Text, STextBlock)
								.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeSequencing)
								.SimpleTextMode(true)
								.Text(Text_FragmentSequencingButton())
								.Justification(ETextJustify::Center)
								.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
							]
						]
					]
				]
			]
			.LowDetail()
			[
				SNew(STextBlock)
				.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeHeader)
				.SimpleTextMode(true)
				.Text_Lambda( [this] () { return Text_NodeHeader(); })
				//.ColorAndOpacity_Lambda( [this] () { return GetDomain().GetGroupColor(); })
			]
		]
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(0, 0, 4, 0)
	[
		SAssignNew(DialogueOutputBoxArea, SBox)
		.MinDesiredWidth(19)
	];

	return Box;
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentBoxes()
{
	bool bFirstFragment = true;

	TSharedRef<SVerticalBox> FragmentBoxes = SNew(SVerticalBox);

	FragmentWidgets.Empty();
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, bFirstFragment ? 0 : 13, 0, bFirstFragment ? 8 : 10)
		[
			CreateFragmentSeparatorWidget(FragmentIndex)
		];
		
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateFragmentRowWidget(FragmentIndex)
		];
		
		bFirstFragment = false;
	};

	return FragmentBoxes;
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::Text_NodeHeader() const
{
	switch (GetFlowYapDialogueNode()->GetNodeType())
	{
		case EYapDialogueNodeType::Talk:
		{
			return GetNodeConfig().GetTalkModeTitle();
		}
		case EYapDialogueNodeType::TalkAndAdvance:
		{
			return GetNodeConfig().GetTalkAndAdvanceModeTitle();
		}
		case EYapDialogueNodeType::PlayerPrompt:
		{
			return GetNodeConfig().GetPromptModeTitle();
		}
		default:
		{
			return LOCTEXT("DialogueNodeHeaderButtonText_Error", "*** ERROR ***");
		}
	}
}

FText SFlowGraphNode_YapDialogueWidget::Text_GroupLabel() const
{
	return GetFlowYapDialogueNode()->GetNodeConfig().General.NodeLabel;
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(uint8 FragmentIndex)
{
	return SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
	.HighDetail()
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ContentPadding(2)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
		.ButtonColorAndOpacity(YapColor::White)
		.ToolTipText(LOCTEXT("InsertFragment_Tooltip", "Insert Fragment"))
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSeparator, FragmentIndex)
		[
			SNew(SSeparator)
			.Thickness(3)
			.ColorAndOpacity(YapColor::White_SemiTrans)
		]
	]
	.LowDetail()
	[
		SNew(SSeparator)
		.Thickness(7)
		.ColorAndOpacity(YapColor::White_SemiTrans)
	];
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_FragmentSeparator() const
{
	return GetIsSelected() ? EVisibility::Visible : EVisibility::Hidden;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSeparator(uint8 Index)
{
	FYapTransactions::BeginModify(LOCTEXT("AddFragment", "Add fragment"), GetDialogueNodeMutable());

	GetFlowGraphNode_YapDialogueMutable()->AddFragment(Index);

	UpdateGraphNode();

	FYapTransactions::EndModify();

	SetNodeSelected();
	
	return FReply::Handled();
}

// ================================================================================================
// FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentRowWidget(uint8 FragmentIndex)
{
	TSharedRef<SFlowGraphNode_YapFragmentWidget> FragmentWidget = SNew(SFlowGraphNode_YapFragmentWidget)
		.InOwner(this)
		.InFragmentIndex(FragmentIndex);

	FragmentWidgets.Add(FragmentWidget);
	
	return FragmentWidget;
}

// ================================================================================================
// LEFT SIDE PANE
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftFragmentPane(uint8 FragmentIndex)
{
	return SNew(SBox)
	.WidthOverride(32)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			//.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateLeftSideNodeBox()
			]
		]
	];
}

// ================================================================================================
// INPUT NODE BOX (UPPER HALF OF LEFT SIDE PANE)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftSideNodeBox()
{
	TSharedRef<SVerticalBox> LeftSideNodeBox = SNew(SVerticalBox);

	return SNew(SBox)
	.MinDesiredHeight(16)
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
	[
		LeftSideNodeBox
	];
}

// ================================================================================================
// RIGHT PANE OF FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

// ================================================================================================
// BOTTOM BAR
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentFooter()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.Padding(31, 18, 7, -10)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &SFlowGraphNode_YapDialogueWidget::UseLowDetail, EGraphRenderingLOD::DefaultDetail)
			.HighDetail()
			[
				SNew(SBorder)
				.Cursor(EMouseCursor::Default)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FilledCircle))
				//.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Rounded))
				.BorderBackgroundColor(this, &SFlowGraphNode_YapDialogueWidget::BorderBackgroundColor_AppendFragmentButton)
				.Padding(0)
				[
					SNew(SBox)
					.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_AppendFragmentButton)
					.WidthOverride(20)
					.HeightOverride(20)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.Padding(0)
					[
						SAssignNew(AppendFragmentButton, SButton)
						.Cursor(EMouseCursor::Default)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_NoBorder)
						.ToolTipText(LOCTEXT("DialogueAddFragment_Tooltip", "Add Fragment"))
						.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_AppendFragmentButton)
						.ContentPadding(2)
						[
							//SNew(SBox)
							//.VAlign(VAlign_Center)
							//.Padding(0)
							//[
								SNew(SImage)
								.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_PlusSign))
								.DesiredSizeOverride(FVector2D(16, 16))
								.ColorAndOpacity(YapColor::DarkGray)
							//]
						]
					]
				]
			]
			.LowDetail()
			[
				SNew(SBox)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(0, 2, 1, 2)
		[
			SAssignNew(BypassOutputBox, SBox)
			.HAlign(HAlign_Center)
			.WidthOverride(24)
			.HeightOverride(24)
			.Padding(0)
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(1)
	[
		SNew(SSeparator)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_AddonsSeparator)
		.Thickness(1)
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.FillWidth(1.0f)
		[
			SAssignNew(LeftNodeBox, SVerticalBox)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SAssignNew(RightNodeBox, SVerticalBox)
		]
	];
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_FragmentSequencingButton() const
{
	if (GetFlowYapDialogueNode()->IsPlayerPrompt())
	{
		return EVisibility::Hidden; // Should be Collapsed but that destroys the parent widget layout for some reason
	}
	
	return (GetFlowYapDialogueNode()->GetNumFragments() > 1) ? EVisibility::Visible : EVisibility::Hidden;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton()
{
	{
		//FYapScopedTransaction(LOCTEXT("ChangeDialogueNodeSequencing", "Change dialogue node sequencing mode"), FlowGraphNode_YapDialogue, YapEditor::Events::DialogueNode::Test, true);
	
		FYapTransactions::BeginModify(LOCTEXT("ChangeSequencingSetting", "Change sequencing setting"), GetDialogueNodeMutable());

		GetDialogueNodeMutable()->CycleFragmentSequencingMode();

		//FragmentSequencingButton_Box->SetVisibility(Visibility_FragmentSequencingButton());

		FragmentSequencingButton_Button->SetToolTipText(ToolTipText_FragmentSequencingButton());
	
		FragmentSequencingButton_Image->SetImage(Image_FragmentSequencingButton());
		FragmentSequencingButton_Image->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

		FragmentSequencingButton_Text->SetText(Text_FragmentSequencingButton());
		FragmentSequencingButton_Text->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

		//FlowGraphNode->ReconstructNode();
		//UpdateGraphNode();
	
		FYapTransactions::EndModify();
	}
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
const FSlateBrush* SFlowGraphNode_YapDialogueWidget::Image_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return FAppStyle::Get().GetBrush("LevelEditor.Profile"); 
		}
		case EYapDialogueTalkSequencing::SelectRandom:
		{
			return FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Random);
		}
	}

	return FAppStyle::Get().GetBrush("Icons.Error"); 
}

// ------------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapDialogueWidget::ToolTipText_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return LOCTEXT("DialogueNodeSequence_RunAll", "Starting from the top, attempt to run all fragments");
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return LOCTEXT("DialogueNodeSequence_RunUntilFailure", "Starting from the top, attempt to run all fragments, stopping if one fails"); 
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return LOCTEXT("DialogueNodeSequence_SelectOne", "Starting from the top, attempt to run all fragments, stopping if one succeeds");
		}
		case EYapDialogueTalkSequencing::SelectRandom:
		{
			return LOCTEXT("DialogueNodeSequence_SelectRandom", "Selects a random fragment");
		}
		default:
		{
			return LOCTEXT("DialogueNodeSequence_Error", "ERROR");
		}
	}
}

// ------------------------------------------------------------------------------------------------
FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EYapDialogueTalkSequencing::RunAll:
		{
			return YapColor::LightBlue;
		}
		case EYapDialogueTalkSequencing::RunUntilFailure:
		{
			return YapColor::LightGreen;
		}
		case EYapDialogueTalkSequencing::SelectOne:
		{
			return YapColor::LightOrange;
		}
		case EYapDialogueTalkSequencing::SelectRandom:
		{
			return YapColor::LightCyan;
		}
		default:
		{
			return YapColor::White;
		}
	}
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_AppendFragmentButton() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

// ------------------------------------------------------------------------------------------------
FReply SFlowGraphNode_YapDialogueWidget::OnClicked_AppendFragmentButton()
{
	FYapTransactions::BeginModify(LOCTEXT("AddFragment", "Add fragment"), GetDialogueNodeMutable());

	GetFlowGraphNode_YapDialogueMutable()->AddFragment(GetFlowYapDialogueNode()->GetNumFragments());

	UpdateGraphNode();

	FYapTransactions::EndModify();

	SetNodeSelected();
	
	return FReply::Handled();
}

FSlateColor SFlowGraphNode_YapDialogueWidget::BorderBackgroundColor_AppendFragmentButton() const
{
	if (AppendFragmentButton->IsHovered())
	{
		return YapColor::DarkGray;
	}

	return YapColor::Noir;
}

// ------------------------------------------------------------------------------------------------
EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_AddonsSeparator() const
{
	return GetFlowYapDialogueNode()->AddOns.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnClick_DeleteConditionButton(int32 FragmentIndex, int32 ConditionIndex)
{
	FYapTransactions::BeginModify(LOCTEXT("DeleteCondition", "Delete condition"), GetDialogueNodeMutable());

	if (FragmentIndex == INDEX_NONE)
	{
		GetDialogueNodeMutable()->GetConditionsMutable().RemoveAt(ConditionIndex);
	}
	else
	{
		GetFragmentMutable(FragmentIndex).GetConditionsMutable().RemoveAt(ConditionIndex);
	}

	FYapTransactions::EndModify();

	GetDialogueNodeMutable()->ForceReconstruction();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::OnEditedConditionChanged(int32 FragmentIndex, int32 ConditionIndex)
{
}

// ------------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapDialogueWidget::IsEnabled_ConditionWidgetsScrollBox() const
{
	return true; //(SYapConditionsScrollBox::EditedConditionDetailsWidget.IsValid());
}

TArray<FOverlayWidgetInfo> SFlowGraphNode_YapDialogueWidget::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	for (const FYapWidgetOverlay& WidgetOverlay : OverlayWidgets)
	{
		FVector2D OwnerLTA = GetPaintSpaceGeometry().LocalToAbsolute(FVector2D(0, 0));

		const FGeometry& ParentGeo = WidgetOverlay.Parent->GetPaintSpaceGeometry();
	
		FVector2D WidgetOfsset = ParentGeo.LocalToAbsolute(FVector2D(0, 0)) - OwnerLTA;

		WidgetOfsset *= 1.0 / OwnerGraphPanelPtr.Pin()->GetZoomAmount();
	
		FOverlayWidgetInfo Info;
		Info.OverlayOffset = WidgetOfsset + FVector2D(0, ParentGeo.Size.Y);
		Info.Widget = WidgetOverlay.Overlay;

		Widgets.Add(Info);

		WidgetOverlay.Overlay->SetRenderOpacity(WidgetOverlay.Opacity);
	}

	return Widgets;
}

// ------------------------------------------------------------------------------------------------
// PUBLIC API & THEIR HELPERS

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetNodeSelected()
{
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}

	GraphEditor->SelectSingleNode(GraphNode);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetFocusedFragmentIndex(uint8 InFragment)
{
	if (FocusedFragmentIndex != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		FocusedFragmentIndex = InFragment;
	}

	SetTypingFocus();
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragmentIndex(uint8 FragmentIndex)
{
	if (FocusedFragmentIndex == FragmentIndex)
	{
		FocusedFragmentIndex.Reset();
	}
}

// ------------------------------------------------------------------------------------------------
const bool SFlowGraphNode_YapDialogueWidget::GetFocusedFragmentIndex(uint8& OutFragmentIndex) const
{
	if (FocusedFragmentIndex.IsSet())
	{
		OutFragmentIndex = FocusedFragmentIndex.GetValue();
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::SetTypingFocus()
{
	bKeyboardFocused = true;
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::ClearTypingFocus()
{
	bKeyboardFocused = false;
}

// ------------------------------------------------------------------------------------------------
UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetDialogueNodeMutable()
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
}

// ------------------------------------------------------------------------------------------------
const UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
}

const UFlowGraphNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowGraphNode_YapDialogue() const
{
	return Cast<UFlowGraphNode_YapDialogue>(GraphNode);
}

UFlowGraphNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowGraphNode_YapDialogueMutable()
{
	return Cast<UFlowGraphNode_YapDialogue>(GraphNode);
}

// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::SetFlashFragment(uint8 FragmentIndex)
{
	FlashFragmentIndex = FragmentIndex;
	FlashHighlight = 1.0;
}

// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::OnDialogueEnd(uint8 FragmentIndex)
{
}

// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::OnDialogueStart(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------

void SFlowGraphNode_YapDialogueWidget::OnDialogueSkipped(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------

const UYapNodeConfig& SFlowGraphNode_YapDialogueWidget::GetNodeConfig() const
{
	const UFlowNode_YapDialogue* DialogueNode = GetFlowYapDialogueNode();

	check(DialogueNode);

	return DialogueNode->GetNodeConfig();
}

// ------------------------------------------------------------------------------------------------
// OVERRIDES & THEIR HELPERS

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SFlowGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}
	
	// TODO cleanup
	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bShiftPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
	
	if (bIsSelected && bShiftPressed && !bKeyboardFocused)
	{
		bShiftHooked = true;
	}

	if (bIsSelected)
	{
		if (FocusedConditionWidget.IsValid() && FocusedConditionWidgetStartTime < 0)
		{
			FocusedConditionWidgetStartTime = FPlatformTime::Seconds();
		}
	}
	else
	{
		bShiftHooked = false;
		FocusedFragmentIndex.Reset();
		bKeyboardFocused = false;

		FocusedConditionWidget = nullptr;
		FocusedConditionWidgetStartTime = -1;

		OverlayWidgets.Empty();
	}

	FlashHighlight = FMath::Max(FlashHighlight, FlashHighlight -= 2.0 * InDeltaTime);

	if (FlashHighlight <= 0)
	{
		FlashFragmentIndex.Reset();
	}

	for (FYapWidgetOverlay& Overlay : OverlayWidgets)
	{
		float NewValue = Overlay.Opacity += 7.0 * InDeltaTime;
		Overlay.Opacity = FMath::Clamp(NewValue, 0.0, 1.0);
	}
}

static const FName OutPinName = FName("Out");
static const FName BypassPinName = FName("Bypass");
static const FName InputPinName = FName("In");

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::CreatePinWidgets()
{	
	TArray<TSet<FFlowPin>> FragmentPins;
	FragmentPins.SetNum(GetFlowYapDialogueNode()->GetFragments().Num());

	TSet<FFlowPin> OptionalPins;
	
	TMap<FFlowPin, int32> FragmentPinsFragmentIndex;

	TSet<FFlowPin> PromptOutPins;
	
	for (int32 i = 0; i < GetFlowYapDialogueNode()->GetFragments().Num(); ++i)
	{
		const FYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragments()[i];

		if (Fragment.UsesStartPin())
		{
			FFlowPin StartPin = Fragment.GetStartPin();
			
			FragmentPins[i].Add(StartPin);
			FragmentPinsFragmentIndex.Add(StartPin, i);
			OptionalPins.Add(StartPin);
		}

		if (Fragment.UsesEndPin())
		{
			FFlowPin EndPin = Fragment.GetEndPin();

			FragmentPins[i].Add(EndPin);
			FragmentPinsFragmentIndex.Add(EndPin, i);
			OptionalPins.Add(EndPin);
		}

		// We store all potential prompt pin names regardless of whether this is a Player Prompt node or not - this helps deal with orphaned pins easier if the user switches the dialogue node type
		FFlowPin PromptPin = Fragment.GetPromptPin();

		FragmentPins[i].Add(PromptPin);
		FragmentPinsFragmentIndex.Add(PromptPin, i);
		PromptOutPins.Add(PromptPin);
	}
	
	// Create Pin widgets for each of the pins.
	for (int32 PinIndex = 0; PinIndex < GraphNode->Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = GraphNode->Pins[PinIndex];

		if ( !ensureMsgf(Pin->GetOuter() == GraphNode
			, TEXT("Graph node ('%s' - %s) has an invalid %s pin: '%s'; (with a bad %s outer: '%s'); skiping creation of a widget for this pin.")
			, *GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString()
			, *GraphNode->GetPathName()
			, (Pin->Direction == EEdGraphPinDirection::EGPD_Input) ? TEXT("input") : TEXT("output")
			,  Pin->PinFriendlyName.IsEmpty() ? *Pin->PinName.ToString() : *Pin->PinFriendlyName.ToString()
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetClass()->GetName() : TEXT("UNKNOWN")
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetPathName() : TEXT("NULL")) )
		{
			continue;
		}

		const TSharedRef<SGraphPin> NewPinRef = OptionalPins.Contains(Pin->GetFName()) ? SNew(SYapGraphPinExec, Pin) : FNodeFactory::CreatePinWidget(Pin).ToSharedRef();

		NewPinRef->SetOwner(SharedThis(this));
		NewPinRef->SetShowLabel(false);
		NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		NewPinRef->SetColorAndOpacity(YapColor::White);

		FString PinToolTIpText = Pin->GetName();

		int32 UnderscoreIndex;
		if (PinToolTIpText.FindLastChar('_', UnderscoreIndex))
		{
			PinToolTIpText = PinToolTIpText.Left(UnderscoreIndex);
		}
		
		NewPinRef->SetToolTipText(FText::FromString(PinToolTIpText));
		
		if (OptionalPins.Contains(Pin->GetFName()))
		{
			NewPinRef->SetPadding(FMargin(-4, -2, 16, -2));
		}

		NewPinRef->SetHAlign(HAlign_Right);

		const bool bAdvancedParameter = Pin && Pin->bAdvancedView;
		if (bAdvancedParameter)
		{
			NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
		}

		TSharedPtr<SBox> PinBox = nullptr;

		if (Pin->GetFName() == OutPinName)
		{
			PinBox = DialogueOutputBoxArea;
			NewPinRef->SetColorAndOpacity(YapColor::White);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (Pin->GetFName() == BypassPinName)
		{
			PinBox = BypassOutputBox;
			NewPinRef->SetColorAndOpacity(NewPinRef->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (int32* FragmentIndex = FragmentPinsFragmentIndex.Find(Pin->GetFName()))
		{
			PinBox = FragmentWidgets[*FragmentIndex]->GetPinContainer(Pin->GetFName());

			FLinearColor PinColor = NewPinRef->IsConnected() ? ConnectedFragmentPinColor : DisconnectedFragmentPinColor;
			
			NewPinRef->SetColorAndOpacity(PinColor);
		}
		else if (Pin->GetFName() == InputPinName)
		{
			PinBox = DialogueInputBoxArea;
			NewPinRef->SetPadding(FMargin(4, -2, 0, -2));			
		}
		else
		{
			NewPinRef->SetShowLabel(true);
			
			if (bAdvancedParameter)
			{
				NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
			}

			if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				LeftNodeBox->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					.Padding(Settings->GetInputPinPadding())
					[
						NewPinRef
					];
				InputPins.Add(NewPinRef);
			}
			else // Direction == EEdGraphPinDirection::EGPD_Output
			{
				RightNodeBox->AddSlot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					.Padding(Settings->GetOutputPinPadding())
					[
						NewPinRef
					];
				OutputPins.Add(NewPinRef);
			}
		}

		if (PinBox.IsValid())
		{
			PinBox->SetContent(NewPinRef);

			if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				InputPins.Add(NewPinRef);
			}
			else
			{
				OutputPins.Add(NewPinRef);
			}
		}
		else
		{
			// TODO error handling
			UE_LOG(LogYapEditor, Warning, TEXT("Could not find pin box for pin %s"), *Pin->GetFName().ToString());
		}
	}
}

// ------------------------------------------------------------------------------------------------
const FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetDialogueNodeMutable()->GetFragmentByIndexMutable(FragmentIndex);
}

// ------------------------------------------------------------------------------------------------
void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	const bool bShowPin = ShouldPinBeHidden(Pin);

	if (bShowPin)
	{
		TSharedPtr<SGraphPin> NewPin = FNodeFactory::CreatePinWidget(Pin);
		check(NewPin.IsValid());

		this->AddPin(NewPin.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
