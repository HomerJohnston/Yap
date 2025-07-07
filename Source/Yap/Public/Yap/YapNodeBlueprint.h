// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Engine/Blueprint.h"
#include "Nodes/FlowNodeBlueprint.h"
#include "YapNodeBlueprint.generated.h"

/**
 * Yap Node Blueprint class
 */
UCLASS(BlueprintType)
class YAP_API UYapNodeBlueprint : public UFlowNodeBlueprint
{
    GENERATED_BODY()

#if WITH_EDITOR
    // UBlueprint
    virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
    virtual bool SupportsDelegates() const override { return false; }
    // --
#endif
};
