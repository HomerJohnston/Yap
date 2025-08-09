// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "KismetPins/SGraphPinObject.h"

class SYapAudioAssetGraphPin : public SGraphPinObject
{
    TSharedRef<SWidget> GenerateAssetPicker() override;
};
