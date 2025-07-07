// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "YapPromptHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

DECLARE_DYNAMIC_DELEGATE(FYapPromptHandleChosen);

USTRUCT(BlueprintType)
struct YAP_API FYapPromptHandle
{
	GENERATED_BODY()
	
	// ------------------------------------------
	// STATE
	// ------------------------------------------

protected:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess, IgnoreForMemberInitializationTest))
	FGuid Guid;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UFlowNode_YapDialogue> NodeType;

	// ------------------------------------------
	// PUBLIC API - Your game should use these
	// ------------------------------------------

public:
	void RunPrompt(UObject* WorldContext);

	// ------------------------------------------
	// YAP API - These are called by Yap classes
	// ------------------------------------------

public:
	FYapPromptHandle();

	FYapPromptHandle(TSubclassOf<UFlowNode_YapDialogue> NodeType);

	void Invalidate();
	
	bool IsValid() const { return Guid.IsValid(); }

	FGuid GetGuid() const { return Guid; }

	TSubclassOf<UFlowNode_YapDialogue> GetNodeType() const { return NodeType; }
	
	bool operator==(const FYapPromptHandle& Other) const
	{
		return Guid == Other.Guid;
	}
	
	FString ToString()
	{
		return Guid.ToString();
	}
};

FORCEINLINE uint32 GetTypeHash(const FYapPromptHandle& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}

// ================================================================================================

/**
 * Function library for prompt handles.
 */
UCLASS()
class YAP_API UYapPromptHandleBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** After a prompt node is entered it will supply you with prompt handles for each option. Use this to select that prompt and continue the conversation. */
	UFUNCTION(BlueprintCallable, Category = "Yap|PromptHandle", meta = (WorldContext = "WorldContext"))
	static void RunPrompt(UObject* WorldContext, const FYapPromptHandle& Handle);
};