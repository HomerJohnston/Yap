// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapCondition.generated.h"

struct FPropertyChangedEvent;

#define LOCTEXT_NAMESPACE "Yap"

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class YAP_API UYapCondition : public UObject
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	bool ImplementsGetWorld() const override { return true; }
#endif

	// --------------------------------------------------------------------------------------------
	// SETTINGS
	// --------------------------------------------------------------------------------------------
protected:

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditDefaultsOnly, Category = "Default")
    FText DefaultTitle = LOCTEXT("UnnamedYapCondition", "Unnamed Condition");

    UPROPERTY(EditInstanceOnly, Category = "Default")
	TOptional<FString> TitleOverride;

    UPROPERTY(EditDefaultsOnly, Category = "Default")
	FLinearColor Color = FLinearColor(0.080, 0.200, 0.100, 1.0);

	/** This is OPTIONAL. If you set this, the details view widget will have this minimum height in the Flow Graph.
	 *  Setting this large enough to contain the whole view will prevent the stupid view widget from bouncing all over the place on creation. */
    UPROPERTY(EditDefaultsOnly, Category = "Default")
	int32 DetailsViewHeight = 200;
	
	/** This is OPTIONAL. If you set this, the details view widget will have this minimum width in the Flow Graph.
	 *  Setting this large enough to contain the whole view will prevent the stupid view widget from bouncing all over the place on creation. */
    UPROPERTY(EditDefaultsOnly, Category = "Default")
	int32 DetailsViewWidth = 400;
#endif

	// --------------------------------------------------------------------------------------------
	// STATE
	// --------------------------------------------------------------------------------------------
public:
#if WITH_EDITORONLY_DATA
	TOptional<bool> LastEvaluation;
#endif	
	
	// --------------------------------------------------------------------------------------------
	// PUBLIC API
	// --------------------------------------------------------------------------------------------
public:
	UFUNCTION(BlueprintNativeEvent)
	bool EvaluateCondition() const;
	
#if WITH_EDITOR
public:
	UFUNCTION(BlueprintNativeEvent)
	FText GetTitle() const;

	UFUNCTION(BlueprintNativeEvent)
	FLinearColor GetColor() const;

	int32 GetDetailsViewHeight() const { return DetailsViewHeight; }
	
	int32 GetDetailsViewWidth() const { return DetailsViewWidth; }
#endif
	
	// --------------------------------------------------------------------------------------------
	// INTERNAL
	// --------------------------------------------------------------------------------------------
public:

	bool EvaluateCondition_Internal();
};

#undef LOCTEXT_NAMESPACE