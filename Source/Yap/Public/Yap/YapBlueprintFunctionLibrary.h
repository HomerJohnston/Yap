// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"
#include "Yap/YapRunningFragment.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YapBlueprintFunctionLibrary.generated.h"

class UYapCharacterAsset;
struct FInstancedStruct;
struct FYapSpeechHandle;

#define LOCTEXT_NAMESPACE "Yap"

/**
 * 
 */
UCLASS()
class YAP_API UYapBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
#if WITH_EDITOR
	/** Built-in simple helper function to play an Unreal sound. */
	UFUNCTION(BlueprintCallable, Category = "Yap|EditorOnly")
	static void PlaySoundInEditor(USoundBase* Sound);
#endif
	
	/** Built-in simple helper function to retrieve the length of an Unreal sound. */
	UFUNCTION(BlueprintCallable, Category = "Yap|Audio")
	static float GetSoundLength(USoundBase* Sound);
	
	UFUNCTION(BlueprintCallable, Category = "Yap|Registration")
	static void RegisterConversationHandler(UObject* NewHandler, FGameplayTag TypeGroup);
	
	UFUNCTION(BlueprintCallable, Category = "Yap|Registration")
	static void RegisterFreeSpeechHandler(UObject* NewHandler);
	
	UFUNCTION(BlueprintCallable, Category = "Yap|Registration")
	static void UnregisterConversationHandler(UObject* HandlerToUnregister);
	
	UFUNCTION(BlueprintCallable, Category = "Yap|Registration")
	static void UnregisterFreeSpeechHandler(UObject* HandlerToUnregister);

	UFUNCTION(BlueprintCallable, Category = "Yap|Character", meta = (WorldContext = "WorldContext"))
	static AActor* FindYapCharacterActor(UObject* WorldContext, TScriptInterface<IYapCharacterInterface> Speaker);
};


#undef LOCTEXT_NAMESPACE
