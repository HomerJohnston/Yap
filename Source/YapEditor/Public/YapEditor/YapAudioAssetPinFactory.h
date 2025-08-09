// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "EdGraphUtilities.h"

class UEdGraphPin;
class SGraphPin;

class FYapAudioAssetPinFactory : public FGraphPanelPinFactory
{
    virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override;
};