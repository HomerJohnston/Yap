// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "SFlowGraphNode_YapFragmentWidget.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

struct FYapTypeGroupSettings;
class SCanvas;
class SYapConditionDetailsViewWidget;
class SVirtualWindow;
struct FYapFragment;
class SFlowGraphNode_YapFragmentWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
class SYapConditionsScrollBox;

#define LOCTEXT_NAMESPACE "YapEditor"

struct FYapWidgetOverlay
{
	TSharedPtr<SWidget> Parent;
	TSharedPtr<SWidget> Overlay;

	FYapWidgetOverlay(TSharedPtr<SWidget> InParent, TSharedPtr<SWidget> InOverlay) : Parent(InParent), Overlay(InOverlay) { }
	
	float Opacity = 0;
};

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode
{
	// ------------------------------------------
	// STATE
protected:
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;

	TSharedPtr<SBox> DialogueInputBoxArea;
	TSharedPtr<SBox> DialogueOutputBoxArea;
	
	TSharedPtr<SBox> BypassOutputBox;

	// TODO make style set junk, for both this and Fragment widget. Pull it out of
	// TODO search engine source for final : public FSlateStyleSet for examples
	FTextBlockStyle NormalText;

	FLinearColor DialogueButtonsColor;
	
	FLinearColor ConnectedBypassPinColor;
	FLinearColor DisconnectedBypassPinColor;

	FLinearColor ConnectedFragmentPinColor;
	FLinearColor DisconnectedFragmentPinColor;
	
	// TODO move to a proper style
	FCheckBoxStyle SkippableCheckBoxStyle;
	TSharedPtr<SButton> NodeHeaderButton;
	TSharedPtr<STextBlock> NodeHeaderButtonToolTip;
	static FButtonStyle MoveFragmentButtonStyle;
	static bool bStylesInitialized;
	
	TOptional<uint8> FocusedFragmentIndex;

	bool bIsSelected = false;
	bool bShiftHooked = false;
	bool bKeyboardFocused = false;
	bool bShiftPressed = false;
	bool bCtrlPressed = false;

	TOptional<uint8> FlashFragmentIndex; 
	double FlashHighlight = 0.0;

	TSharedPtr<SBox> FragmentSequencingButton_Box;
	TSharedPtr<SButton> FragmentSequencingButton_Button;
	TSharedPtr<SImage> FragmentSequencingButton_Image;
	TSharedPtr<STextBlock> FragmentSequencingButton_Text;
	
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;
	
	TSharedPtr<SYapConditionDetailsViewWidget> FocusedConditionWidget;
	
	double FocusedConditionWidgetStartTime = -1;
	
public:
	TArray<FYapWidgetOverlay> OverlayWidgets;
	
	void AddOverlayWidget(TSharedPtr<SWidget> ParentWidget, TSharedPtr<SWidget> OverlayWidget, bool bClearExisting = true);
	void RemoveOverlayWidget(TSharedPtr<SWidget> OverlayWidget);
	void ClearOverlayWidgets();
	
	// ------------------------------------------
	// CONSTRUCTION
public:

	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);
	
	void PreConstruct(const FArguments& InArgs, UFlowGraphNode* InNode);// override;
	
	void PostConstruct(const FArguments& InArgs, UFlowGraphNode* InNode);// override;
	
	void RequestUpdateGraphNode() { UpdateGraphNode(); }

	bool HasActiveOverlay() { return OverlayWidgets.Num() > 0; }
	
	// ------------------------------------------
	// WIDGETS
protected:
	int32 GetDialogueActivationCount() const;

	int32 GetDialogueActivationLimit() const;
	void OnTextCommitted_DialogueActivationLimit(const FText& Text, ETextCommit::Type Arg);
	FGameplayTag Value_DialogueTag() const;
	void OnTagChanged_DialogueTag(FGameplayTag NewDialogueTag);

public:
	void OnTagChanged_DialogueTag_PostEdit(TArray<TPair<FString, FString>>);

protected:
	FOptionalSize GetMaxNodeWidth() const;
	FOptionalSize GetMaxTitleWidth() const;

public:
	virtual bool UseLowDetail() const { return false; };

public:
	void OnConditionsArrayChanged();

	// ------------------------------------------
protected:
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;

	// ------------------------------------------
protected:
	TSharedRef<SWidget>	CreateNodeContentArea() override;
	
	// ------------------------------------------
protected:
	FSlateColor ColorAndOpacity_NodeHeaderButton() const;
	FText Text_FragmentSequencingButton() const;
	FReply OnClicked_TogglePlayerPrompt();

	TSharedRef<SWidget> CreateContentHeader();

	EVisibility			Visibility_FragmentSequencingButton() const;
	FReply				OnClicked_FragmentSequencingButton();
	const FSlateBrush*	Image_FragmentSequencingButton() const;
	FText				ToolTipText_FragmentSequencingButton() const;
	FSlateColor			ColorAndOpacity_FragmentSequencingButton() const;
	
	FText				Text_NodeHeader() const;
	FText				Text_GroupLabel() const;

	// ------------------------------------------
protected:
	TSharedRef<SWidget> CreateFragmentBoxes();

	TSharedRef<SWidget>	CreateFragmentSeparatorWidget(uint8 FragmentIndex);

	EVisibility			Visibility_FragmentSeparator() const;
	FReply				OnClicked_FragmentSeparator(uint8 Index);

	// ------------------------------------------
protected:
	TSharedRef<SWidget>	CreateFragmentRowWidget(uint8 FragmentIndex);
	
	TSharedRef<SBox>	CreateLeftFragmentPane(uint8 FragmentIndex);

	// ------------------------------------------
protected:
	TSharedRef<SBox>	CreateLeftSideNodeBox();

	// ------------------------------------------
protected:
	TSharedRef<SWidget> CreateContentFooter();

	EVisibility			Visibility_BottomAddFragmentButton() const;
	FReply				OnClicked_BottomAddFragmentButton();
	EVisibility Visibility_AddonsSeparator() const;

public:
	void OnClick_DeleteConditionButton(int32 FragmentIndex, int32 ConditionIndex);
	void OnEditedConditionChanged(int32 FragmentIndex, int32 ConditionIndex);
	void OnClick_NewConditionButton(int32 FragmentIndex);
	bool IsEnabled_ConditionWidgetsScrollBox() const;
	
	// ------------------------------------------
	// PUBLIC API & THEIR HELPERS
public:
	bool GetIsSelected() const { return bIsSelected; };

	void SetNodeSelected();

	bool GetShiftHooked() const { return bShiftHooked; };
	
	void SetFocusedFragmentIndex(uint8 FragmentIndex);

	void ClearFocusedFragmentIndex(uint8 FragmentIndex);
	
	const bool GetFocusedFragmentIndex(uint8& OutFragmentIndex) const;

	void SetTypingFocus();

	void ClearTypingFocus();
	
	UFlowNode_YapDialogue* GetFlowYapDialogueNodeMutable();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;
	const UFlowGraphNode_YapDialogue* GetFlowGraphNode_YapDialogue() const;
	UFlowGraphNode_YapDialogue* GetFlowGraphNode_YapDialogueMutable();
	
	TOptional<uint8> GetFocusedFragmentIndex() { return FocusedFragmentIndex; };

protected:
	void SetFlashFragment(uint8 FragmentIndex);
	
	void OnDialogueEnd(uint8 FragmentIndex);
	void OnDialogueStart(uint8 FragmentIndex);
	void OnDialogueSkipped(uint8 FragmentIndex);

	const FYapTypeGroupSettings& GetTypeGroup() const;
	
	// ------------------------------------------
	// OVERRIDES & THEIR HELPERS
public:
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
	void CreatePinWidgets() override;

	const FYapFragment& GetFragment(uint8 FragmentIndex) const;

	FYapFragment& GetFragmentMutable(uint8 FragmentIndex);

	void CreateStandardPinWidget(UEdGraphPin* Pin) override;

	TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;

	virtual TSharedPtr<IToolTip> GetToolTip() override { return nullptr; };

	TSharedPtr<SYapConditionsScrollBox> DialogueConditionsScrollBox;

	TMap<FName, FName> EventUpdateMemberMap;
	TMap<FName, FSimpleDelegate> MemberUpdateDelegateMap;
};


#undef LOCTEXT_NAMESPACE
