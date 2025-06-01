// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapText.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

USTRUCT(BlueprintType)
struct YAP_API FYapText
{
#if WITH_EDITOR
	friend class SFlowGraphNode_YapFragmentWidget;
#endif
	
	GENERATED_BODY()

	// --------------------------------------------------------------------------------------------
	// SETTINGS
	// --------------------------------------------------------------------------------------------
private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess), Category = "Default")
	FText Text;

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess), Category = "Default")
	int32 WordCount = 0;

	// --------------------------------------------------------------------------------------------
	// PUBLIC API
	// --------------------------------------------------------------------------------------------
public:
	
	const FText& Get() const { return Text; }

	int32 GetWordCount() const { return WordCount; }

	// --------------------------------------------------------------------------------------------
	// EDITOR API
	// --------------------------------------------------------------------------------------------
public:
#if WITH_EDITOR
	void Set(const FText& InText);

	void UpdateInternalWordCount();

	void Clear();
#endif

	void operator=(const FYapText& Other)
	{
		Text = Other.Text;
		WordCount = Other.WordCount;
	}
	
	void operator=(const FText& NewText)
	{
#if WITH_EDITOR
		Set(NewText);
#else
		checkNoEntry() // Make sure I don't try to use this in any shipping code
#endif
	}
};

#undef LOCTEXT_NAMESPACE