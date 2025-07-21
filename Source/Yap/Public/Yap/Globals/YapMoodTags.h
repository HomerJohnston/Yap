// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

class UYapNodeConfig;
class UFlowNode_YapDialogue;

namespace Yap
{
    YAP_API TArray<UFlowNode_YapDialogue*> GetYapNodeTypes();

    YAP_API FGameplayTagContainer GetMoodTagRoots();

    YAP_API const UYapNodeConfig& GetConfigUsingMoodRoot(const FGameplayTag& Root);
    
    YAP_API FGameplayTagContainer GetAllMoodTags();

    YAP_API FGameplayTagContainer GetAllMoodTagsUnder(const FGameplayTag& Root);
}
