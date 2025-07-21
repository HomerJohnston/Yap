// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "Nodes/FlowNode.h"
#include "Yap/YapFragment.h"
#include "Yap/YapBitReplacement.h"

#include "FlowNode_YapReplaceFragment.generated.h"

class UFlowNode_YapDialogue;

UCLASS( meta = (DisplayName = "Yap Replace Fragment"))
class YAP_API UFlowNode_YapReplaceFragment : public UFlowNode
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Default")
	FGameplayTag TargetFragmentTag;

    UPROPERTY(EditAnywhere, Category = "Default")
	FYapBitReplacement NewData;
	
public:
	UFlowNode_YapReplaceFragment();

public:
	virtual void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
public:
	FText GetNodeTitle() const override;
	
	FString GetNodeCategory() const override;

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
#endif
};