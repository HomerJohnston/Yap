// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "GameplayTagContainer.h"
#include "IDetailCustomization.h"

class UYapNodeConfig;
class UFlowNode_YapDialogue;
class UYapCharacterAsset;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapCharacterAsset : public IDetailCustomization, public FEditorUndoClient//, TSharedFromThis<FDetailCustomization_YapCharacter>
{
public:
	FDetailCustomization_YapCharacterAsset();
	
	virtual ~FDetailCustomization_YapCharacterAsset();

	FDelegateHandle Handle;
	
private:
	TWeakObjectPtr<UYapCharacterAsset> CharacterBeingCustomized;

	TMap<FGameplayTag, TWeakObjectPtr<const UYapNodeConfig>> NodeConfigs;

	TSet<FGameplayTag> MissingMoodTagIcons;
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapCharacterAsset());
	}

	FText Text_MoodTag(FGameplayTag MoodRoot, FGameplayTag MoodTag) const;
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
	void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder) override;
	
	EVisibility Visibility_MoodTagsOutOfDateWarning() const;
	
	FText Text_PortraitsListHint() const;

	void OnClicked_RefreshMoodTagsButton() const;

	float ButtonWidth;

	TWeakPtr<IDetailLayoutBuilder> CachedDetailBuilder = nullptr;

	TSharedPtr<IPropertyHandle> DefaultPortraitProperty;
	TSharedPtr<IPropertyHandle> OBSOLETE_PortraitsProperty_OBSOLETE;
	TSharedPtr<IPropertyHandle> PortraitsProperty;

	bool CacheEditedCharacterAsset(IDetailLayoutBuilder& DetailBuilder);

	void CachePropertyHandles(IDetailLayoutBuilder& DetailBuilder);
	
	void SortCategories(IDetailLayoutBuilder& DetailBuilder) const;
	
	void DrawObsoleteDataWarning(IDetailLayoutBuilder& DetailBuilder);

	bool GetHasPortraitData() const;
	
	bool GetHasObsoleteData() const;

	void OnClicked_FixupOldPortraitsMap();

	bool FixupOldPortraitsMap();

	void RefreshList(TSharedPtr<IPropertyHandle> MoodRootProperty, TSharedPtr<IPropertyHandle> List) const;

	void AddEntryToMap(TSharedPtr<IPropertyHandleMap> Map, const FGameplayTag& NewKey) const;

	void RemoveEntryFromMap(TSharedPtr<IPropertyHandleMap> Map, const FName& DeadKey) const;

	void RefreshDetails() const;

	void AddBottomControls(IDetailCategoryBuilder& Category) const;

	void PostUndo(bool bSuccess) override;

	void PostRedo(bool bSuccess) override;

	const FSlateBrush* Image_MoodTag(FGameplayTag MoodRoot, FGameplayTag MoodTag) const;
};

#undef LOCTEXT_NAMESPACE
