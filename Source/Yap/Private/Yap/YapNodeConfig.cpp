#// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapNodeConfig.h"

#include "GameplayTagsManager.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"

FYapNodeConfigGroup_FlowGraphSettings::FYapNodeConfigGroup_FlowGraphSettings()
{
}

FGameplayTagContainer FYapNodeConfigGroup_MoodTags::GetAllMoodTags()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTagsParent);
}

UYapNodeConfig::UYapNodeConfig()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
    
    General.AllowableNodeTypes = static_cast<int32>(EYapDialogueNodeType::COUNT) - 1;
}

#if WITH_EDITOR
void UYapNodeConfig::PostLoad()
{
    Super::PostLoad();
    
    RebuildMoodTagIcons();
}
#endif

#if WITH_EDITOR
void UYapNodeConfig::RebuildMoodTagIcons()
{
    FGameplayTagContainer AllMoodTags = MoodTags.GetAllMoodTags();
    MoodTagIconBrushes.Empty(AllMoodTags.Num() + 1);

    for (const FGameplayTag& MoodTag : AllMoodTags)
    {
        BuildIcon(MoodTag);
    }

    BuildIcon(FGameplayTag::EmptyTag);
}
#endif

#if WITH_EDITOR
void UYapNodeConfig::BuildIcon(const FGameplayTag& MoodTag)
{
    TSharedPtr<FSlateImageBrush> ImageBrush = nullptr;
	
    // Attempt to load SVG
    FString IconPath = GetMoodTagIconPath(MoodTag, "svg");
    ImageBrush = MakeShared<FSlateVectorImageBrush>(IconPath, FVector2f(16, 16), FLinearColor::White);

    // Attempt to load PNG
    if (!ImageBrush)
    {
        IconPath = GetMoodTagIconPath(MoodTag, "png");
        ImageBrush = MakeShared<FSlateImageBrush>(IconPath, FVector2f(16, 16), FLinearColor::White);
    }
	
    // Found nothing
    if (!ImageBrush)
    {
        return;
    }

    MoodTagIconBrushes.Add(MoodTag, ImageBrush);
}
#endif

#if WITH_EDITOR
FString UYapNodeConfig::GetMoodTagIconPath(FGameplayTag Key, FString FileExtension)
{
    /*
    FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";
    
    int32 Index;

    if (KeyString.FindLastChar('.', Index))
    {
        KeyString = KeyString.RightChop(Index + 1);
    }
    */

    FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";
    
    if (MoodTags.EditorIconsPath.Path == "")
    {
        static FString ResourcesDir = Yap::FileUtilities::GetPluginFolder();
		
        return Yap::FileUtilities::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodIcons/{0}.{1}"), { KeyString, FileExtension });
    }

    return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { MoodTags.EditorIconsPath.Path, KeyString, FileExtension });
}
#endif

#if WITH_EDITOR
TSharedPtr<FSlateImageBrush> UYapNodeConfig::GetMoodTagIcon(FGameplayTag MoodTag) const
{
    const TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(MoodTag);

    if (Brush)
    {
        return *Brush;
    }

    return nullptr;
}
#endif

#if WITH_EDITOR
const FSlateBrush* UYapNodeConfig::GetMoodTagBrush(FGameplayTag Name) const
{
    const TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(Name);

    return Brush ? Brush->Get() : nullptr; //FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodTag_Missing);
}
#endif

TArray<FString>& UYapNodeConfig::GetDefaultMoodTags()
{
    static TArray<FString> Tags
    {
        "Angry",
        "Calm",
        "Confused",
        "Disgusted",
        "Happy",
        "Injured",
        "Laughing",
        "Panicked",
        "Sad",
        "Scared",
        "Smirking",
        "Stressed",
        "Surprised",
        "Thinking",
        "Tired"
    };

    return Tags;
}

#if WITH_EDITOR
FGameplayTagContainer UYapNodeConfig::GetMoodTags() const
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTags.MoodTagsParent);
}
#endif
