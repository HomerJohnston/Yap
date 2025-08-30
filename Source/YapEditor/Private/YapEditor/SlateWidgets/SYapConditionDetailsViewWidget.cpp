// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/SlateWidgets/SYapConditionDetailsViewWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ----------------------------------------------
void SYapConditionDetailsViewWidget::Construct(const FArguments& InArgs)
{
	check(InArgs._Dialogue.IsValid());
	check(InArgs._ConditionIndex != INDEX_NONE)
	
	Dialogue = InArgs._Dialogue;
	FragmentIndex = InArgs._FragmentIndex;
	ConditionIndex = InArgs._ConditionIndex;
	OnClickedDelete = InArgs._OnClickedDelete;
	OnSelectedNewClass = InArgs._OnClickedNewClass;
	
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bShowOptions = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TSharedRef<IDetailsView> DetailsWidget = PropertyEditorModule.CreateDetailView(Args);
	DetailsWidgetWeakPtr = DetailsWidget;

	UYapCondition* Condition = GetCondition();

	DetailsWidget->SetObject(Condition);

	float WidgetWidth = 0;
	float WidgetHeight = 0;

	if (IsValid(Condition))
	{
		WidgetWidth = Condition->GetDetailsViewWidth();
		WidgetHeight = Condition->GetDetailsViewHeight();
	}
	
	if (WidgetWidth <= 100.0)
	{
		WidgetWidth = UYapDeveloperSettings::GetConditionDetailsWidth();
	}
	
	if (WidgetHeight <= 100.0)
	{
		WidgetHeight = UYapDeveloperSettings::GetConditionDetailsHeight();
	}

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(WidgetWidth)
		.HeightOverride(WidgetHeight)
		.Padding(0)
		[
			SNew(SBorder)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Deburred))
			.BorderBackgroundColor(YapColor::Button_Unset())
			.Padding(4)
			[
				SNew(SScrollBox)
				.ScrollBarStyle(FYapEditorStyle::Get(), YapStyles.ScrollBarStyle_DialogueBox)
				.ScrollBarPadding(FMargin(4,0,0,0))
				+ SScrollBox::Slot()
				.Padding(0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 2)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(0, 0, 2, 0)
						.FillWidth(1.0)
						[
							SNew(SClassPropertyEntryBox)
							.OnSetClass(this, &SYapConditionDetailsViewWidget::OnSetClass_ConditionProperty)
							.MetaClass(UYapCondition::StaticClass())
							.SelectedClass(this, &SYapConditionDetailsViewWidget::SelectedClass_ConditionProperty)
							.AllowAbstract(false)
							.ShowTreeView(false)
							.AllowNone(false)
						]
						+ SHorizontalBox::Slot()
						.Padding(2, 1, 0, 1)
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonColorAndOpacity(YapColor::DeepOrangeRed)
							.ContentPadding(FMargin(4, 2, 4, 2))
							.ForegroundColor(YapColor::White)
							.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
							.ToolTipText(LOCTEXT("DeleteCondition", "Delete condition"))
							.OnClicked(this, &SYapConditionDetailsViewWidget::OnClicked_Delete)
							[
								SNew(SImage)
								//.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Delete))
								.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
								.DesiredSizeOverride(FVector2D(16, 16))
								.ColorAndOpacity(FSlateColor::UseStyle())
							]
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0)
					.Padding(0, 0, 0, 0)
					[
						DetailsWidget
					]
				]
			]
		]
	];
}

// ----------------------------------------------
FReply SYapConditionDetailsViewWidget::OnClicked_Delete() const
{
	OnClickedDelete.Execute(ConditionIndex);
	return FReply::Handled();
}

// ----------------------------------------------
const UYapCondition* SYapConditionDetailsViewWidget::GetCondition() const
{
	if (FragmentIndex == INDEX_NONE)
	{
		return Dialogue->GetConditionsMutable()[ConditionIndex];		
	}
	else
	{
		return Dialogue->GetFragmentsMutable()[FragmentIndex].GetConditionsMutable()[ConditionIndex];
	}
}

// ----------------------------------------------
UYapCondition* SYapConditionDetailsViewWidget::GetCondition()
{
	return const_cast<UYapCondition*>(const_cast<const SYapConditionDetailsViewWidget*>(this)->GetCondition());
}

// ----------------------------------------------
const UClass* SYapConditionDetailsViewWidget::SelectedClass_ConditionProperty() const
{
	if (GetCondition())
	{
		return GetCondition()->GetClass();
	}

	return nullptr;
}

// ----------------------------------------------
void SYapConditionDetailsViewWidget::OnSetClass_ConditionProperty(const UClass* NewConditionClass)
{
	UClass* ExistingConditionClass = nullptr;

	UYapCondition* ExistingCondition = GetCondition();

	if (IsValid(ExistingCondition))
	{
		ExistingConditionClass = ExistingCondition->GetClass();
	}
	
	if (ExistingConditionClass == NewConditionClass)
	{
		return;
	}

	FYapTransactions::BeginModify(LOCTEXT("ChangeConditionType", "Change Condition type"), Dialogue.Get());

	if (FragmentIndex != INDEX_NONE)
	{
		TArray<TObjectPtr<UYapCondition>>& ConditionsArray = Dialogue->GetFragmentsMutable()[FragmentIndex].GetConditionsMutable();

		if (NewConditionClass)
		{
			ConditionsArray[ConditionIndex] = NewObject<UYapCondition>(Dialogue.Get(), NewConditionClass);
		}
		else
		{
			ConditionsArray[ConditionIndex] = nullptr;
		}
	}
	else
	{
		TArray<UYapCondition*>& ConditionsArray = Dialogue->GetConditionsMutable();

		if (NewConditionClass)
		{
			ConditionsArray[ConditionIndex] = NewObject<UYapCondition>(Dialogue.Get(), NewConditionClass);
		}
		else
		{
			ConditionsArray[ConditionIndex] = nullptr;
		}
	}

	FYapTransactions::EndModify();

	OnSelectedNewClass.Execute(ConditionIndex);
}

#undef LOCTEXT_NAMESPACE
