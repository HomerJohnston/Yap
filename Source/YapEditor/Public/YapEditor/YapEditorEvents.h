// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

#define YAP_EVT_NAME(NAME) inline const FName NAME = "NAME"

// Doesn't do anything right now. Future class to help make UI more event-based and avoid developer bugs with forgetting to refresh UI elements.

namespace YapEditor
{
	namespace Event
	{
		YAP_EVT_NAME(None);
		
		namespace DialogueNode
		{
			YAP_EVT_NAME(Test);
		};
	}
}

#undef YAP_EVT_NAME

#undef LOCTEXT_NAMESPACE