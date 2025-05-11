// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define HOVER		1.20
#define PRESS		0.80
#define SEMITRANS	0.90
#define TRANS		0.70
#define SEMIGLASS	0.45
#define GLASS		0.20
#define SUPERGLASS	0.05

#define DECLARE_COLOR_TRAN(Name, RR, GG, BB, AA)\
inline FLinearColor Name = FLinearColor(RR, GG, BB, AA);\
inline FLinearColor Name##Hovered = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, AA);\
inline FLinearColor Name##Pressed = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, AA);\
inline FLinearColor Name##_SemiTrans = FLinearColor(RR, GG, BB, SEMITRANS);\
inline FLinearColor Name##Hovered_SemiTrans = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, SEMITRANS * AA);\
inline FLinearColor Name##Pressed_SemiTrans = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, SEMITRANS * AA);\
inline FLinearColor Name##_Trans = FLinearColor(RR, GG, BB, TRANS);\
inline FLinearColor Name##Hovered_Trans = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, TRANS * AA);\
inline FLinearColor Name##Pressed_Trans = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, TRANS * AA);\
inline FLinearColor Name##_SemiGlass = FLinearColor(RR, GG, BB, SEMIGLASS);\
inline FLinearColor Name##Hovered_SemiGlass = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, SEMIGLASS * AA);\
inline FLinearColor Name##Pressed_SemiGlass = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, SEMIGLASS * AA);\
inline FLinearColor Name##_Glass = FLinearColor(RR, GG, BB, GLASS);\
inline FLinearColor Name##Hovered_Glass = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, GLASS * AA);\
inline FLinearColor Name##Pressed_Glass = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, GLASS * AA);\
inline FLinearColor Name##_SuperGlass = FLinearColor(RR, GG, BB, SUPERGLASS);\
inline FLinearColor Name##Hovered_SuperGlass = FLinearColor(HOVER * RR, HOVER * GG, HOVER * BB, SUPERGLASS * AA);\
inline FLinearColor Name##Pressed_SuperGlass = FLinearColor(PRESS * RR, PRESS * GG, PRESS * BB, SUPERGLASS * AA)

#define DECLARE_COLOR(Name, RR, GG, BB) DECLARE_COLOR_TRAN(Name, RR, GG, BB, 1.000)

#define LOCTEXT_NAMESPACE "YapEditor"

namespace YapColor
{
	DECLARE_COLOR(LightOrange,		0.850, 0.550, 0.150);
	DECLARE_COLOR(BrightOrange,		1.000, 0.650, 0.050);
	DECLARE_COLOR(Orange,			0.780, 0.380, 0.050);
	DECLARE_COLOR(DarkOrange,		0.280, 0.180, 0.040);
	DECLARE_COLOR(DeepOrange,		0.100, 0.050, 0.010);

	DECLARE_COLOR(LightYellow,		0.900, 0.850, 0.250);
	DECLARE_COLOR(Yellow,			0.850, 0.800, 0.050);
	DECLARE_COLOR(YellowGray,		0.500, 0.450, 0.250);

	DECLARE_COLOR(OrangeRed,		0.920, 0.070, 0.020);
	DECLARE_COLOR(DarkOrangeRed,	0.600, 0.050, 0.010);
	DECLARE_COLOR(DeepOrangeRed,	0.300, 0.030, 0.005);
	DECLARE_COLOR(OrangeRedGray,	0.300, 0.130, 0.050);
	
	DECLARE_COLOR(LightRed,			0.950, 0.340, 0.420);
	DECLARE_COLOR(Red,				0.950, 0.040, 0.060);
	DECLARE_COLOR(DarkRed,			0.250, 0.010, 0.030);
	DECLARE_COLOR(DeepRed,			0.050, 0.010, 0.020);

	DECLARE_COLOR(LightGreen,		0.340, 1.000, 0.420);
	DECLARE_COLOR(Green,			0.180, 0.950, 0.220);
	DECLARE_COLOR(DarkGreen,		0.020, 0.150, 0.050);
	DECLARE_COLOR(DarkGreenGray,	0.080, 0.200, 0.100);
	DECLARE_COLOR(DeepGreen,		0.015, 0.040, 0.020);

	DECLARE_COLOR(LightBlue,		0.340, 0.420, 1.000);
	DECLARE_COLOR(BrightBlue,		0.020, 0.350, 0.750);
	DECLARE_COLOR(Blue,				0.040, 0.060, 0.950);
	DECLARE_COLOR(DimBlue,			0.030, 0.040, 0.350);
	DECLARE_COLOR(DarkBlue,			0.010, 0.010, 0.100);
	DECLARE_COLOR(DarkBlueGray,		0.100, 0.080, 0.200);
	DECLARE_COLOR(DeepBlue,			0.010, 0.010, 0.500);

	DECLARE_COLOR(LightCyan,		0.500, 0.950, 0.950);
	DECLARE_COLOR(Cyan,		0.05, 0.800, 0.800);
	
	DECLARE_COLOR(White,			1.000, 1.000, 1.000);
	DECLARE_COLOR(DimWhite,			0.900, 0.900, 0.900);
	DECLARE_COLOR(LightGray,		0.500, 0.500, 0.500);
	DECLARE_COLOR(Gray,				0.200, 0.200, 0.200);
	DECLARE_COLOR(DimGray,			0.100, 0.100, 0.100);
	DECLARE_COLOR(DarkGray,			0.030, 0.030, 0.030);
	DECLARE_COLOR(DeepGray,			0.010, 0.010, 0.010);
	DECLARE_COLOR(Noir,				0.005, 0.005, 0.005);
	DECLARE_COLOR(Black,			0.000, 0.000, 0.000);

	DECLARE_COLOR_TRAN(Transparent,	1.000, 1.000, 1.000, 0.0);
	DECLARE_COLOR(Error,		1.000, 0.000, 1.000);

	const FLinearColor Asset_Character = LightGreen;
	inline const FLinearColor Button_Unset() { return DarkGray; }

	/** FLinearColor::Desaturate also lerps alpha towards zero for some reason... use my own instead. */
	inline FLinearColor Desaturate(FLinearColor InColor, float Desaturation )
	{
		float Lum = InColor.GetLuminance();
		return FMath::Lerp( InColor, FLinearColor( Lum, Lum, Lum, InColor.A ), Desaturation );
	}
}

#undef LOCTEXT_NAMESPACE