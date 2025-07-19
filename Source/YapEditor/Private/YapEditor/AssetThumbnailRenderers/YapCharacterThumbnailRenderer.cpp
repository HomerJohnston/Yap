// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/AssetThumbnailRenderers/YapCharacterThumbnailRenderer.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Yap/YapCharacterAsset.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "Yap"

void UYapCharacterThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	OutWidth = 128;
	OutHeight = 128;
}

void UYapCharacterThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UYapCharacterAsset* Character = Cast<UYapCharacterAsset>(Object);

	if (!Character)
	{
		return;
	}

	const UTexture2D* TexturePtr = Character->GetCharacterPortrait(FGameplayTag::EmptyTag);

	FVector2D Center (Width * 0.5f, Height * 0.5f);
		
	FCanvasBoxItem Box(FVector2D(Width * 0.00f, Height * 0.00f), FVector2D(Width * 1.0, Height * 1.0));
	
	Box.SetColor(Character->GetCharacterColor() * YapColor::Gray);
	Box.LineThickness = 0.0625 * Width;

	Canvas->DrawNGon(Center, (Character->GetCharacterColor() * YapColor::DimGray).ToFColor(true), 16, Width);

	Canvas->DrawItem(Box);
	
	if (TexturePtr)
	{
		Canvas->DrawTile(
			Width * 0.0625,
			Height * 0.0625,
			Width * 0.875,
			Height * 0.875,
			0,
			0,
			1,
			1,
			YapColor::White_SemiTrans,
			TexturePtr->GetResource(),
			true
			);
	}
	else
	{
		FSlateFontInfo FontInfo = YapFonts.Font_CharacterAssetThumbnail;

		// Split into individual strings because Unreal is annoying and has no way to center-justify text
		FCanvasTextItem Text1(Center + FVector2D(0, Height * -0.2), INVTEXT("Default"), GEngine->GetLargeFont(), YapColor::DimWhite); // I'm just not going to localize these
		Text1.bCentreX = true;
		Text1.bCentreY = true;
		Text1.SlateFontInfo = FontInfo;

		FCanvasTextItem Text2(Center + FVector2D(0, Height * 0.0), INVTEXT("portrait"), GEngine->GetMediumFont(), YapColor::DimWhite);
		Text2.bCentreX = true;
		Text2.bCentreY = true;
		Text2.SlateFontInfo = FontInfo;
		
		FCanvasTextItem Text3(Center + FVector2D(0, Height * 0.2), INVTEXT("missing"), GEngine->GetLargeFont(), YapColor::DimWhite);
		Text3.bCentreX = true;
		Text3.bCentreY = true;
		Text3.SlateFontInfo = FontInfo;

		Canvas->DrawItem(Text1);
		Canvas->DrawItem(Text2);
		Canvas->DrawItem(Text3);
	}
}

EThumbnailRenderFrequency UYapCharacterThumbnailRenderer::GetThumbnailRenderFrequency(UObject* Object) const
{
	return EThumbnailRenderFrequency::OnAssetSave;
}

bool UYapCharacterThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	UYapCharacterAsset* Character = Cast<UYapCharacterAsset>(Object);
	
	return (!!Character);
}

#undef LOCTEXT_NAMESPACE