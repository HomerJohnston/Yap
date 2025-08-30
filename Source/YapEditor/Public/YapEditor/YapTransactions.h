// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class UFlowGraphNode_YapBase;

#define LOCTEXT_NAMESPACE "YapEditor"

#define YAP_DECLARE_TRANSACTION(NAME) static FName NAME = "NAME"

namespace YapTransactions
{
	YAP_DECLARE_TRANSACTION(RefreshCharacterPortraitList);
	YAP_DECLARE_TRANSACTION(AddMoodTagPortrait);
	YAP_DECLARE_TRANSACTION(FixupOldPortraitsMap);
}

/** Helper class. I ran into a few odd issues using FScopedTransaction so I made this, seems to work more reliably. */
class FYapTransactions
{
public:
	static void BeginModify(const FText& TransactionText, UObject* Object);

	static void EndModify();
};

/**
 * The purpose of this is to make it more reliable to develop this plugin with less human errors, by:
 *
 * 1) Automatically calling Modify on UObject.
 * 2) Automatically broadcasting a simple named event delegate. This makes it easier for widgets to reliably update their state.
 */
class FYapScopedTransaction
{
	FName Event;

	TWeakObjectPtr<UFlowGraphNode_YapBase> PrimaryObject;
	
public:
	FYapScopedTransaction(FName InEvent, const FText& TransactionText, UObject* Object);

	void AbortAndUndo();
	
	void Cancel();
	
	bool IsOutstanding() const;
	
	~FYapScopedTransaction();
	
	int32 Index = -1;
};

#undef LOCTEXT_NAMESPACE