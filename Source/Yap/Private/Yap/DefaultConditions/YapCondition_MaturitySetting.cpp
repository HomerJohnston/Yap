// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Yap/DefaultConditions/YapCondition_MaturitySetting.h"

#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

bool UYapCondition_MaturitySetting::EvaluateCondition_Implementation() const
{
	if (RequiredSetting == EYapMaturitySetting::Unspecified)
	{
		return true;
	}
	
	return RequiredSetting == UYapSubsystem::GetCurrentMaturitySetting(GetWorld());
}

// ------------------------------------------------------------------------------------------------
#if WITH_EDITOR
FLinearColor UYapCondition_MaturitySetting::GetColor_Implementation() const
{
	return Super::GetColor_Implementation();
}
#endif
// ------------------------------------------------------------------------------------------------
#if WITH_EDITOR
FText UYapCondition_MaturitySetting::GetTitle_Implementation() const
{
	switch (RequiredSetting)
	{
		case EYapMaturitySetting::Mature:
		{
			return LOCTEXT("YapCondition_MatureLabel", "Mature");
		}
		case EYapMaturitySetting::ChildSafe:
		{
			return LOCTEXT("YapCondition_ChildSafeLabel", "Child-Safe");
		}
		default:
		{
			return LOCTEXT("YapCondition_ErrorLabel", "Unset");
		}
	}
}
#endif
// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE