// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapAudioAssetPinFactory.h"

#include "KismetPins/SGraphPinObject.h"
#include "KismetPins/SGraphPinString.h"
#include "YapEditor/GraphPins/YapAudioAssetGraphPin.h"

TSharedPtr<SGraphPin> FYapAudioAssetPinFactory::CreatePin(UEdGraphPin* InPin) const
{
    static FName YapPinMetaKey("YapPin");
    static FName DialogueAudioAssetPinName("DialogueAudioAsset");
    
    if (InPin->GetFName() == DialogueAudioAssetPinName)
    {
        FString MetaData = InPin->GetOuter()->GetPinMetaData(InPin->PinName, YapPinMetaKey);
    
        static FString DialogueAudioAssetMetaValue = "DialogueAudioAsset";
        
        if ( MetaData == DialogueAudioAssetMetaValue)
        {
            return SNew(SYapAudioAssetGraphPin, InPin);
        }
    }
    
    return nullptr;
}
