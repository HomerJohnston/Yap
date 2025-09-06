// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapText.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// ================================================================================================

/**
 * This is a simple wrapper for FText so that it can include a cached word count.
 */
USTRUCT(BlueprintType)
struct YAP_API FYapText
{
#if WITH_EDITOR
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class FYapEditableTextPropertyHandle;
	friend struct FYapBit;
	
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
	// INTERNAL
	// --------------------------------------------------------------------------------------------
#if WITH_EDITOR
public:
	
	void operator=(const FYapText& Other)
	{
		Text = Other.Text;
		WordCount = Other.WordCount;
	}
	
protected:
	
	void Set(const FText& InText);

	void UpdateInternalWordCount();

	void Clear();

	void operator=(const FText& NewText)
	{
		Set(NewText);
	}
#endif

};

// ================================================================================================

#undef LOCTEXT_NAMESPACE