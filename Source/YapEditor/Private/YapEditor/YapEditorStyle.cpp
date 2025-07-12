// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// This file is formatted to be viewable on tabs, 4-space tabs. If you use another setting, sorry!

#include "YapEditor/YapEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapEditorColor.h"

TArray<TStrongObjectPtr<UTexture2D>> FYapEditorStyle::Textures;

FYapFonts YapFonts;
FYapBrushes YapBrushes;
FYapStyles YapStyles;

#define YAP_QUOTE(X) #X

/** Makes a simple font definition copying default font */
#define YAP_DEFINE_FONT(NAME, STYLE, SIZE)\
	YapFonts.NAME = DEFAULT_FONT(STYLE, SIZE);\
	FSlateFontInfo& NAME = YapFonts.NAME

/** Loads a TTF from disk */
#define YAP_LOAD_FONT(NAME, RESOURCE_PATH, SIZE)\
	YapFonts.NAME = FSlateFontInfo(Yap::FileUtilities::GetResourcesFolder() / RESOURCE_PATH, SIZE);\
	FSlateFontInfo& NAME = YapFonts.NAME

/** Define a new brush */
#define YAP_DEFINE_BRUSH(TYPE, BRUSHNAME, FILENAME, EXTENSION, ...)\
	YapBrushes.BRUSHNAME = YAP_QUOTE(BRUSHNAME);\
	StyleInstance->Set(YAP_QUOTE(BRUSHNAME), new TYPE(StyleInstance->RootToContentDir(FILENAME, TEXT(EXTENSION)), __VA_ARGS__));\
	const TYPE& BRUSHNAME = *static_cast<const TYPE*>(StyleInstance->GetBrush(YAP_QUOTE(BRUSHNAME)))

/** Define a new style */
#define YAP_DEFINE_STYLE(TYPE, STYLENAME, TEMPLATE, MODS)\
	YapStyles.STYLENAME = YAP_QUOTE(STYLENAME);\
	StyleInstance->Set(YAP_QUOTE(STYLENAME), TYPE(TEMPLATE));\
	TYPE& STYLENAME = const_cast<TYPE&>(StyleInstance->GetWidgetStyle<TYPE>(YAP_QUOTE(STYLENAME)));\
	STYLENAME MODS
	
/** Used to copy an existing UE brush into Yap style for easier use */
#define YAP_REDEFINE_UE_BRUSH(TYPE, YAPNAME, UESTYLESET, UENAME, ...)\
	YapBrushes.YAPNAME = YAP_QUOTE(YAPNAME);\
	const TYPE& YAPNAME = *(new TYPE(UESTYLESET::GetBrush(UENAME)->GetResourceName().ToString(), __VA_ARGS__));\
	StyleInstance->Set(YAP_QUOTE(YAPNAME), const_cast<TYPE*>(&YAPNAME))



#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

#define IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BOX_BRUSH_SVG( RelativePath, ... ) FSlateVectorBoxBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BORDER_BRUSH_SVG( RelativePath, ... ) FSlateVectorBorderBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

#define CORE_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define CORE_BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define CORE_BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

#define CORE_IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define CORE_BOX_BRUSH_SVG( RelativePath, ... ) FSlateVectorBoxBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define CORE_BORDER_BRUSH_SVG( RelativePath, ... ) FSlateVectorBorderBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

#define LOCTEXT_NAMESPACE "YapEditor"

TSharedPtr<FSlateStyleSet> FYapEditorStyle::StyleInstance = nullptr;

ISlateStyle& FYapEditorStyle::Get()
{
	TSharedPtr<FSlateStyleSet> FUFKYOU = StyleInstance;
	return *StyleInstance;
}

FYapEditorStyle::FYapEditorStyle()
{
/*
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddRaw(this, &FYapEditorStyle::OnPatchComplete);
	}
#endif
*/
}

FYapEditorStyle::~FYapEditorStyle()
{
	Textures.Empty();
/*
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		LiveCoding->GetOnPatchCompleteDelegate().Remove(OnPatchCompleteHandle);
	}
#endif
*/	
	//FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
}

FName FYapEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("YapEditorStyle"));
	return StyleSetName;
}

void FYapEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}

	Initialize_Internal();
}

void FYapEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<class FSlateStyleSet> FYapEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetParentStyleName(FAppStyle::GetAppStyleSetName());
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Yap")->GetBaseDir() / TEXT("Resources"));
	Style->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	
	return Style;
}

#if WITH_LIVE_CODING
void FYapEditorStyle::OnPatchComplete()
{
	/*
	if (UYapDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
		Initialize_Internal();
		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}
	*/
}

const ISlateStyle* FYapEditorStyle::GetParentStyle()
{
	return &FAppStyle::Get();
}
#endif

#define YAP_COMMON_BRUSH "Common/ButtonHoverHint"
#define YAP_COMMON_MARGIN FMargin(4.0 / 16.0)
#define YAP_COMMON_PRESSED_PADDING FMargin(0, 1, 0, -1) // Push down by one pixel
#define YAP_COMMON_CHECKBOXSTYLE FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox")

void FYapEditorStyle::Initialize_Internal()
{
	YAP_REDEFINE_UE_BRUSH(FSlateImageBrush,			None,							FAppStyle,	"NoBorder",					FVector2f(16, 16), YapColor::Transparent);
	YAP_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,	Icon_FilledCircle,				FAppStyle,	"Icons.FilledCircle",		FVector2f(16, 16));
	YAP_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,	Icon_PlusSign,					FAppStyle,	"Icons.Plus",				FVector2f(16, 16));
	YAP_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,	Icon_ProjectSettings_TabIcon,	FAppStyle,	"ProjectSettings.TabIcon",	FVector2f(16, 16));
	
	// ============================================================================================
	// FONTS
	// ============================================================================================
	YAP_DEFINE_FONT(Font_DialogueText,		"Normal",	10);
	YAP_DEFINE_FONT(Font_TitleText,			"Italic",	10);
	YAP_DEFINE_FONT(Font_NodeHeader,		"Bold",		15);
	YAP_DEFINE_FONT(Font_GroupLabel,		"Normal",	15);
	YAP_DEFINE_FONT(Font_SectionHeader,		"Bold",		12);
	YAP_DEFINE_FONT(Font_NodeSequencing,	"Italic",	9);
	YAP_DEFINE_FONT(Font_CharacterAssetThumbnail, "Normal", 14);
	YAP_DEFINE_FONT(Font_WarningText,		"Italic",	10);

	YAP_LOAD_FONT(Font_OpenSans_Regular, "Fonts/OpenSans-Regular.ttf", 10);
	YAP_LOAD_FONT(Font_NotoSans_Regular, "Fonts/NotoSans-Regular.ttf", 10);
	YAP_LOAD_FONT(Font_NotoSans_SemiBold, "Fonts/NotoSans-SemiBold.ttf", 10);
	
	YAP_LOAD_FONT(Font_BeVietnam_Light, "Fonts/BeVietnam-Light.ttf", 10);
	YAP_LOAD_FONT(Font_BeVietnam_Regular, "Fonts/BeVietnam-Regular.ttf", 10);

	// ============================================================================================
	// BRUSHES
	// ============================================================================================
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_AudioTime,					"DialogueNodeIcons/AudioTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_PlaybackTimeHandle,		"Icon_PlaybackTimeHandle", ".png",	FVector2f(3, 5));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Baby,						"Icon_Baby", ".svg",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_MoodTag_Missing,			"Icon_MoodTag_Missing", ".svg",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_Delete,					"Icon_Delete", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_DownArrow,					"Icon_DownArrow", ".png",				FVector2f(8, 8));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_TextTime,					"DialogueNodeIcons/TextTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_Timer,						"DialogueNodeIcons/Timer", ".png",		FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_LocalLimit,				"DialogueNodeIcons/LocalLimit", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_Speaker,					"Icon_Audio", ".png",					FVector2f(16, 16));
	//YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Tag,						"Icon_Tag", ".svg",						FVector2f(16, 16));
	YapBrushes.Icon_Tag = "Icon_Tag";
	StyleInstance->Set("Icon_Tag", new FSlateVectorImageBrush(StyleInstance->RootToContentDir("Icon_Tag", TEXT(".svg")), FVector2f(16, 16)));

	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_Edit,						"Icon_Edit", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_CornerDropdown_Right,		"Icon_CornerDropdown_Right", ".svg",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Checkmark,					"Icon_Checkmark", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_CrossX,					"Icon_CrossX", ".svg",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Icon_UpArrow,					"Icon_UpArrow", ".png",					FVector2f(8, 8));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Skippable,					"Icon_Skippable", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_NotSkippable,				"Icon_NotSkippable", ".svg",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_AutoAdvance,				"Icon_AutoAdvance", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_ManualAdvance,				"Icon_ManualAdvance", ".svg",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Reset_Small,				"Icon_Reset_Small", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Notes,						"Icon_Notes", ".svg",					FVector2f(32, 32));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_FragmentData,				"Icon_FragmentData", ".svg",			FVector2f(32, 32));
	
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Reconciled,				"Icon_Reconciled", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Unreconciled,				"Icon_Unreconciled", ".svg",			FVector2f(16, 16));
	
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Switch_On,					"Icon_Switch_On", ".svg",				FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Switch_Off,				"Icon_Switch_Off", ".svg",				FVector2f(16, 16));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Icon_IDTag,						"Icon_IDTag", ".png",					FMargin(0.5, 0.5, 0.0, 0.0));
	
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Random,					"Icon_Random", ".svg",					FVector2f(32, 32));

	YAP_DEFINE_BRUSH(FSlateImageBrush,			Bar_NegativePadding,			"Bar_NegativePadding", ".png",			FVector2f(5, 3), FLinearColor::White, ESlateBrushTileType::Horizontal);
	YAP_DEFINE_BRUSH(FSlateImageBrush,			Bar_PositivePadding,			"Bar_PositivePadding", ".png",			FVector2f(3, 3), FLinearColor::White, ESlateBrushTileType::Horizontal);
	
	YAP_DEFINE_BRUSH(FSlateBorderBrush, 		Border_SharpSquare,				"Border_Sharp", ".png",					FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBorderBrush, 		Border_DeburredSquare,			"Border_Deburred", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBorderBrush, 		Border_RoundedSquare,			"Border_Rounded", ".png",				FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBorderBrush, 		Border_Thick_RoundedSquare,		"Border_Thick_Rounded", ".png",			FMargin(8.0/16.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Panel_Sharp,					"Panel_Sharp", ".png",					FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Panel_Deburred,					"Panel_Deburred", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Panel_Rounded,					"Panel_Rounded", ".png",				FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidWhite,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidWhite_Deburred,		"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidWhite_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidLightGray,				"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::LightGray);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidLightGray_Deburred,	"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::LightGray);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidLightGray_Rounded,		"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::LightGray);
			
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidRed,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Red);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidRed_Deburred,			"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Red);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidRed_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Red);
			
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidNoir,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Noir);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidNoir_Deburred,			"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Noir);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidNoir_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Noir);
			
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidBlack,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Black);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidBlack_Deburred,		"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Black);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Box_SolidBlack_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Black);
			
	YAP_DEFINE_BRUSH(FSlateBoxBrush,			Outline_White_Deburred,			"Outline_Deburred", ".png",				FMargin(4.0/8.0));
	
	// ============================================================================================
	// BRUSHES - SVGs
	// ============================================================================================
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Chevron_Right,				"Icon_Chevron_Right", ".svg",			FVector2f(16, 16), YapColor::White);
	YAP_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Caret_Right,				"Icon_Caret_Right", ".svg",				FVector2f(16, 16), YapColor::White);
	
	// ============================================================================================
	// SLIDER STYLES
	// ============================================================================================

	YAP_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
	);
	
	// ============================================================================================
	// BUTTON STYLES
	// ============================================================================================

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_NoBorder, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"), );
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_HoverHintOnly, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("HoverHintOnly"), );
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SimpleButton, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"), );
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_HeaderButton, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Gray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SequencingSelector, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Button_Unset()))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ActivationLimit, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Button_Unset()))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_FragmentControls, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Button_Unset()))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SimpleYapButton, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetNormalForeground(YapColor::Gray_SemiGlass)
		.SetHoveredForeground(YapColor::Gray_SemiTrans)
		.SetPressedForeground(YapColor::DarkGray)
		.SetPressedPadding(FMargin(0, 1, 0, -1))
		);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SpeakerPopup, FButtonStyle::GetDefault(),
		.SetNormal(None)
		.SetHovered(None)
		.SetPressed(None)
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_TimeSetting, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::LightGray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Gray))
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_TimeSettingOpener, FButtonStyle::GetDefault(),
		.SetNormal(None)
		.SetHovered(None)
		.SetPressed(None)
		.SetDisabled(None)
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ConditionWidget, FButtonStyle::GetDefault(),
		.SetNormal(Box_SolidLightGray_Deburred)
		.SetHovered(Box_SolidWhite_Deburred)
		.SetPressed(Box_SolidLightGray_Deburred)	
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_AudioPreview, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
		.SetNormalForeground(YapColor::Gray_SemiGlass)
		.SetHoveredForeground(YapColor::Gray_SemiTrans)
		.SetPressedForeground(YapColor::DarkGray)
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_TagButton, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"),
		.SetNormalPadding(FMargin(0,0,2,0))
		.SetPressedPadding(FMargin(0,0,2,0));
	);
	
	// ============================================================================================
	// COMBO BUTTON STYLES
	// ============================================================================================

	YAP_DEFINE_STYLE(FComboButtonStyle, ComboButtonStyle_YapGameplayTagTypedPicker, FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton"),
		.SetButtonStyle(ButtonStyle_TagButton)
		.SetDownArrowPadding(FMargin(0, 2, 0, 0))
		.SetDownArrowAlignment(VAlign_Top)
		);
	
	// ============================================================================================
	// CHECKBOX STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FCheckBoxStyle, CheckBoxStyle_Skippable, YAP_COMMON_CHECKBOXSTYLE,
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetCheckedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedImage)
		.SetCheckedHoveredImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedHoveredImage)
		.SetCheckedPressedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedPressedImage)
		.SetUndeterminedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedImage)
		.SetUndeterminedHoveredImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedHoveredImage)
		.SetUndeterminedPressedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedPressedImage)
	);

	YAP_DEFINE_STYLE(FCheckBoxStyle, CheckBoxStyle_TypeSettingsOverride, YAP_COMMON_CHECKBOXSTYLE,
		.SetCheckBoxType(ESlateCheckBoxType::CheckBox)
		.SetForegroundColor(YapColor::Gray_Trans) // Unchecked
		.SetHoveredForegroundColor(YapColor::White) // Unchecked, Hovered
		.SetPressedForegroundColor(YapColor::LightGray) // Unchecked, Pressed
		.SetCheckedForegroundColor(YapColor::LightGray) // Checked
		.SetCheckedHoveredForegroundColor(YapColor::White) // Checked, Hovered
		.SetCheckedPressedForegroundColor(YapColor::LightGray) // Checked, Pressed
		
		.SetCheckedImage(Icon_Switch_On)
		.SetCheckedHoveredImage(Icon_Switch_On)
		.SetCheckedPressedImage(Icon_Switch_On)

		.SetUncheckedImage(Icon_Switch_Off)
		.SetUncheckedHoveredImage(Icon_Switch_Off)
		.SetUncheckedPressedImage(Icon_Switch_Off)
	);
		
	// ============================================================================================
	// SCROLLBAR STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FScrollBarStyle, ScrollBarStyle_DialogueBox, FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"),
		.SetThickness(2)
		.SetHorizontalBackgroundImage(Box_SolidBlack)
		.SetHorizontalBottomSlotImage(Box_SolidWhite)
		.SetDraggedThumbImage(Box_SolidWhite)
		.SetHoveredThumbImage(Box_SolidWhite)
		.SetNormalThumbImage(Box_SolidLightGray)
	);

	// ============================================================================================
	// TEXT BLOCK STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_DialogueText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_DialogueText)
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFontSize(10)
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_TitleText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_TitleText)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeHeader, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeHeader)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_GroupLabel, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_GroupLabel)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeSequencing, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeSequencing)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);
	
	// ============================================================================================
	// EDITABLE TEXT BLOCK STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_Dialogue, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox) // This doesn't do dick, thanks Epic
		.SetTextStyle(TextBlockStyle_DialogueText)
		.SetForegroundColor(FSlateColor::UseForeground())
		.SetPadding(0)
		.SetBackgroundImageNormal(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageHovered(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageFocused(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageReadOnly(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);

	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_TitleText, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox)
		.SetTextStyle(TextBlockStyle_TitleText)
		.SetFont(Font_TitleText)
		.SetForegroundColor(FSlateColor::UseForeground())
		.SetPadding(0)
		.SetBackgroundColor(FStyleColors::Recessed)
	);
		
	// ============================================================================================
	// PROGRESS BAR STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FProgressBarStyle, ProgressBarStyle_FragmentTimePadding, FProgressBarStyle::GetDefault(),
		.SetBackgroundImage(None)
		.SetFillImage(Box_SolidWhite)
		.SetEnableFillAnimation(false)
	);

	StyleInstance->Set("YapEditor.PluginAction", new FSlateVectorImageBrush(StyleInstance->RootToContentDir(L"YapProjectSettings", TEXT(".svg")), CoreStyleConstants::Icon20x20));
}

#undef LOCTEXT_NAMESPACE