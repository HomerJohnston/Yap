// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"

#include "YapPromptHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

USTRUCT(BlueprintType)
struct YAP_API FYapPromptHandle
{
	GENERATED_BODY()

	// ------------------------------------------
	// STATE

protected:
	UPROPERTY(Transient)
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(Transient)
	uint8 FragmentIndex;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess, IgnoreForMemberInitializationTest))
	FGuid Guid;
	
	// ------------------------------------------
	// PUBLIC API - Your game should use these

public:
	void RunPrompt(UObject* WorldContextObject);

	// ------------------------------------------
	// YAP API - These are called by Yap classes

public:
	FYapPromptHandle();

	FYapPromptHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex);

	void Invalidate();
	
	bool IsValid() { return Guid.IsValid(); }
	
	UFlowNode_YapDialogue* GetDialogueNode() const { return DialogueNode; }

	uint8 GetFragmentIndex() const { return FragmentIndex; }

	FGuid GetGuid() const { return Guid; }
	
	bool operator==(const FYapPromptHandle& Other) const
	{
		return Guid == Other.Guid;
	}
};