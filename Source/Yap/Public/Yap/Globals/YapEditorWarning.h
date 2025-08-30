// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Internationalization/Text.h"

namespace Yap
{
#if WITH_EDITOR
	namespace Editor
	{
		YAP_API void PostNotificationInfo_Warning(FText Title, FText Description, float Duration = 6.0f);
	}
#endif
}
