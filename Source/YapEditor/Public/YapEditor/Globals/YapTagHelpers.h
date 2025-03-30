// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

struct FAssetIdentifier;

namespace Yap
{
	static inline FString NodeCategory = "Yap";

	namespace  Tags
	{
		/**  */
		FString GetFilteredSubTag(const FString& Filter, const FGameplayTag& PropertyTag);

		/**  */
		FString GetFilteredSubTag(const FGameplayTag& ParentContainer, const FGameplayTag& PropertyTag);

		/**  */
		FName GetLeafTag(const FGameplayTag& Tag);
		
		/**  */
		const FGameplayTag& GetOrAddTag(FString NewTagString, FString Comment = "");

		void ParseChangedTag(FString OldTagString, FString NewTagString);
		
		/** Erases tags over two (three) editor ticks. First ongoing tick assumes an asset is finishing up a transaction, then next tick it can be saved, then tick after that the tags can be safely deleted. */
		void DeleteTags(TArray<FGameplayTag> TagsToErase, bool bShowPrompt = false, TWeakObjectPtr<UObject> Asset = nullptr);

		/**  */
		void RedirectTags(TArray<TPair<FGameplayTag, FGameplayTag>> Redirects, bool bShowPrompt = false, TWeakObjectPtr<UObject> Asset = nullptr);

		
		/**  */
		void RedirectTags(TArray<TPair<FString, FString>> Redirects, bool bShowPrompt = false, TWeakObjectPtr<UObject> Asset = nullptr);
		
		/**  */
		TArray<FAssetIdentifier> FindTagReferences(FName TagName);
	}
}
