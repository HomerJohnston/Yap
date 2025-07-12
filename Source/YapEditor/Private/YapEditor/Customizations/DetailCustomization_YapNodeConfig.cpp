// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Customizations/DetailCustomization_YapNodeConfig.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Yap/YapNodeConfig.h"
#include "YapEditor/YapEditorLog.h"


void SortCategory(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap, int32& Order, TSet<FName>& SortedCategories, FName NextCategory)
{
	SortedCategories.Add(NextCategory);

	(*AllCategoryMap.Find(NextCategory))->SetSortOrder(Order);
}

void CustomSortYapProjectSettingsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap )
{
	int i = 0;

	TSet<FName> SortedCategories;

	SortCategory(AllCategoryMap, i, SortedCategories, "Core");
	SortCategory(AllCategoryMap, i, SortedCategories, "Editor");
	SortCategory(AllCategoryMap, i, SortedCategories, "Flow Graph Settings");
	SortCategory(AllCategoryMap, i, SortedCategories, "Error Handling");
	SortCategory(AllCategoryMap, i, SortedCategories, "Other");
	SortCategory(AllCategoryMap, i, SortedCategories, "Characters");
	
	if (SortedCategories.Num() != AllCategoryMap.Num())
	{
		UE_LOG(LogYapEditor, Error, TEXT("Not all categories were sorted!"));
	}
}

void FDetailCustomization_YapNodeConfig::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	
	DetailFont = DetailBuilder.GetDetailFont();
	
	for (TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (Object->IsA<UYapNodeConfig>())
		{
			break;
		}
	}

	DetailBuilder.SortCategories(&CustomSortYapProjectSettingsCategories);
	
	IDetailCategoryBuilder& CharactersCategory = DetailBuilder.EditCategory("Characters");
	
	ProcessCategory(CharactersCategory);
}

void FDetailCustomization_YapNodeConfig::ProcessCategory(IDetailCategoryBuilder& Category) const
{
	
}

