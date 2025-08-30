// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapInputTracker.h"

#include "Input/Events.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FYapInputTracker::FYapInputTracker(UYapEditorSubsystem* InOwner)
{
	Owner = InOwner;
}

void FYapInputTracker::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FYapInputTracker::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::LeftShift)
	{
		bLeftShift = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightShift)
	{
		bRightShift = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::LeftControl)
	{
		bLeftControl = true;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightControl)
	{
		bRightControl = true;
	}
	
	return false;
}

bool FYapInputTracker::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::LeftShift)
	{
		bLeftShift = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightShift)
	{
		bRightShift = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::LeftControl)
	{
		bLeftControl = false;
	}
	else if (InKeyEvent.GetKey() == EKeys::RightControl)
	{
		bRightControl = false;
	}
	
	return false;
}

bool FYapInputTracker::GetShiftPressed() const
{
	return bLeftShift || bRightShift;
}

bool FYapInputTracker::GetControlPressed() const
{
	return bLeftControl || bRightControl;
}

#undef LOCTEXT_NAMESPACE