// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "Fonts/SlateFontInfo.h"

#include "YapDeveloperSettings.generated.h"

class UFontFace;
class UFont;

#define LOCTEXT_NAMESPACE "YapEditor"

/** Per user settings for Yap, mostly graph appearance settings. */
UCLASS(Config = "EditorPerProjectUserSettings")
class UYapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UYapDeveloperSettings& Get() { return *StaticClass()->GetDefaultObject<UYapDeveloperSettings>(); }

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsWidth = 400;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsHeight = 400;

	/** Controls how bright the portrait borders are in the graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0, Delta = 0.01))
	float PortraitBorderAlpha = 1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bWrapExpandedEditorText = false;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -200, ClampMax = +9999, UIMin = -200, UIMax = +400, Delta = 1))
	int32 ExpandedEditorWidthAdjustment = 0;
	
	/** Allows the developer to override the dialogue font. Might be useful e.g. for team members with dyslexia. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FSlateFontInfo GraphDialogueFontUserOverride;

	
	/** This is for internal development only. This helps to reduce crashes from reloading slate styles after live coding. */
	UPROPERTY(Config, EditAnywhere, Category = "Development")
	bool bCloseAndReopenAssetsOnLiveCoding = false;
	
public:
	static float GetConditionDetailsWidth() { return Get().ConditionDetailsWidth; }
	
	static float GetConditionDetailsHeight() { return Get().ConditionDetailsHeight; }

	static float GetPortraitBorderAlpha() { return Get().PortraitBorderAlpha; }

	static bool GetWrapExpandedEditorText() { return Get().bWrapExpandedEditorText; }

	static int32 GetExpandedEditorWidthAdjustment() { return Get().ExpandedEditorWidthAdjustment; }
	
	static const FSlateFontInfo& GetGraphDialogueFontUserOverride() { return Get().GraphDialogueFontUserOverride; }

	static bool GetCloseAndReopenAssetsOnLiveCoding() { return Get().bCloseAndReopenAssetsOnLiveCoding; }
	
public:
	FName GetCategoryName() const override { return FName("Yap"); }

	FText GetSectionText() const override { return LOCTEXT("DeveloperSettings", "Developer Settings"); }
	
	FText GetSectionDescription() const override { return LOCTEXT("YapDeveloperSettingsDescription", "Local user settings for Yap"); }
};

#undef LOCTEXT_NAMESPACE
