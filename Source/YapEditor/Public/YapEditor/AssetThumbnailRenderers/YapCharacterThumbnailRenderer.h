// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once
#include "ThumbnailRendering/TextureThumbnailRenderer.h"

#include "YapCharacterThumbnailRenderer.generated.h"

UCLASS(MinimalAPI)
class UYapCharacterThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()
	
public:
	void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;

	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;

	EThumbnailRenderFrequency GetThumbnailRenderFrequency(UObject* Object) const override;
	
	bool CanVisualizeAsset(UObject* Object) override;
};
