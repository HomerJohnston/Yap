#// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapNodeConfig.h"

#include "GameplayTagsManager.h"
#include "Brushes/SlateImageBrush.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"

#define LOCTEXT_NAMESPACE "YapEditor"

#if WITH_EDITOR
TMap<TSoftObjectPtr<UYapNodeConfig>, TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>>> UYapNodeConfig::MoodTagIconBrushes;
TUniquePtr<FSlateImageBrush> UYapNodeConfig::NullMoodTagIconBrush;
#endif

FYapNodeConfigGroup_FlowGraphSettings::FYapNodeConfigGroup_FlowGraphSettings()
{
}

FGameplayTagContainer FYapNodeConfigGroup_MoodTags::GetAllMoodTags()
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTagsRoot);
}

UYapNodeConfig::UYapNodeConfig()
{
#if WITH_EDITOR
    General.AllowableNodeTypes = static_cast<int32>(EYapDialogueNodeType::COUNT) - 1;

    AddGameplayTagFilter(GET_MEMBER_NAME_CHECKED(ThisClass, MoodTags.DefaultMoodTag), FGameplayTagFilterDelegate::CreateUObject(this, &ThisClass::GetMoodTagsRoot));

    if (IsTemplate())
    {
        const static FString ResourcesFolder = Yap::FileUtilities::GetResourcesFolder();

        const FString Result = ResourcesFolder / "DefaultMoodIcons/NullMoodIcon.svg";

        NullMoodTagIconBrush = MakeUnique<FSlateVectorImageBrush>(Result, FVector2f(16, 16), FLinearColor::White);
    }
#endif
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
    if (IsTemplate())
    {
        return;
    }
    
    FGameplayTagContainer AllMoodTags = MoodTags.GetAllMoodTags();

    TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>>* Map = MoodTagIconBrushes.Find(this);

    if (Map)
    {
        Map->Empty(AllMoodTags.Num() + 1);
    }
    else
    {
        Map = &MoodTagIconBrushes.Add(this, {});
    }

    for (const FGameplayTag& MoodTag : AllMoodTags)
    {
        BuildIcon(Map, MoodTag);
    }

    BuildIcon(Map, FGameplayTag::EmptyTag);
}
#endif

#if WITH_EDITOR
void UYapNodeConfig::BuildIcon( TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>>* Map, const FGameplayTag& MoodTag)
{
    TSharedPtr<FSlateImageBrush> ImageBrush = nullptr;

    {
        FString IconPath = GetMoodTagIconPath(MoodTag, "svg");
    
        if (FPaths::FileExists(IconPath))
        {
            UE_LOG(LogYap, Display, TEXT("Loading Mood Tag Icon: %s"), *IconPath);

            Map->Add(MoodTag, MakeUnique<FSlateVectorImageBrush>(IconPath, FVector2f(16, 16), FLinearColor::White));
            return;
        }
    }
    
    {
        FString IconPath = GetMoodTagIconPath(MoodTag, "png");
        
        if (FPaths::FileExists(IconPath))
        {
            UE_LOG(LogYap, Display, TEXT("Loading Mood Tag Icon: %s"), *IconPath);

            Map->Add(MoodTag, MakeUnique<FSlateImageBrush>(IconPath, FVector2f(16, 16), FLinearColor::White));
            return;
        }
    }

    UE_LOG(LogYap, Warning, TEXT("Could not find image file for mood icon: %s"), *MoodTag.ToString());
}
#endif

#if WITH_EDITOR
FString UYapNodeConfig::GetMoodTagIconPath(FGameplayTag Key, FString FileExtension) const
{
    FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";
    
    if (MoodTags.EditorIconsPath.Path == "")
    {
        const static FString ResourcesDir = Yap::FileUtilities::GetPluginFolder();

        const static FString ResourcesFolder = Yap::FileUtilities::GetResourcesFolder();

        const FString Result = ResourcesFolder / FString::Format(TEXT("DefaultMoodIcons/{0}.{1}"), { KeyString, FileExtension });

        return Result;
    }

    const static FString ProjectDir = FPaths::ProjectDir();

    const FString Result = ProjectDir / FString::Format(TEXT("{0}/{1}.{2}"), { MoodTags.EditorIconsPath.Path, KeyString, FileExtension }); 

    bool bExists = FPaths::FileExists(Result);
    
    return Result;
}
#endif

#if WITH_EDITOR
FSlateImageBrush* UYapNodeConfig::GetMoodTagIcon(FGameplayTag MoodTag) const
{
    TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>>* Map = MoodTagIconBrushes.Find(this);

    if (Map)
    {
        if (const TUniquePtr<FSlateImageBrush>* Brush = Map->Find(MoodTag))
        {
            FSlateImageBrush* Ptr = Brush->Get();
        
            return Ptr;
        }
    }
    
    return NullMoodTagIconBrush.Get();
}
#endif

#if WITH_EDITOR
const FSlateBrush* UYapNodeConfig::GetMoodTagBrush(FGameplayTag Name) const
{
    TMap<FGameplayTag, TUniquePtr<FSlateImageBrush>>* Map = MoodTagIconBrushes.Find(this);

    if (Map)
    {
        const TUniquePtr<FSlateImageBrush>* Brush = Map->Find(Name);
        return Brush ? Brush->Get() : NullMoodTagIconBrush.Get();
    }

    return NullMoodTagIconBrush.Get();    
}
#endif

#if WITH_EDITOR
bool UYapNodeConfig::CanEditChange(const FProperty* InProperty) const
{
    bool bSuper = Super::CanEditChange(InProperty);

    if (!bSuper)
    {
        return false;
    }

    TSet<FName> MoodTagPropertyNames =
    {
        GET_MEMBER_NAME_CHECKED(FYapNodeConfigGroup_MoodTags, DefaultMoodTag),
        GET_MEMBER_NAME_CHECKED(FYapNodeConfigGroup_MoodTags, EditorIconsPath),
    };
    
    if (MoodTagPropertyNames.Contains(InProperty->GetFName()))
    {
        return MoodTags.MoodTagsRoot.IsValid();
    }

    return true;
}
#endif

#if WITH_EDITOR
bool UYapNodeConfig::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
    return UObject::CanEditChange(PropertyChain);
}
#endif

bool UYapNodeConfig::GetUsesTitleText(EYapDialogueNodeType NodeType) const
{
    if (NodeType == EYapDialogueNodeType::PlayerPrompt)
    {
        return !Graph.bHideTitleTextOnPromptNodes;
    }

    return !Graph.bShowTitleTextOnTalkNodes;
}

#if WITH_EDITORONLY_DATA
const FYapAudioIDFormat& UYapNodeConfig::GetAudioIDFormat() const
{
    return Audio.AudioIDFormat.IsSet() ? Audio.AudioIDFormat.GetValue() : UYapProjectSettings::GetDefaultAudioIDFormat();
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

const FGameplayTag& UYapNodeConfig::GetMoodTagsRoot() const
{
    return MoodTags.MoodTagsRoot; 
}

#if WITH_EDITOR
FText UYapNodeConfig::GetTalkModeTitle() const
{
    if (General.TalkLabelOverride.IsEmpty())
    {
        return LOCTEXT("Talk_DefaultNodeTitle", "Talk");
    }

    return General.TalkLabelOverride;
}
#endif

#if WITH_EDITOR
FText UYapNodeConfig::GetTalkAndAdvanceModeTitle() const
{
    if (General.TalkAndAdvanceLabelOverride.IsEmpty())
    {
        return LOCTEXT("TalkAndAdvance_DefaultNodeTitle", "Talk & Advance");
    }

    return General.TalkAndAdvanceLabelOverride;
}
#endif

#if WITH_EDITOR
FText UYapNodeConfig::GetPromptModeTitle() const
{
    if (General.PromptLabelOverride.IsEmpty())
    {
        return LOCTEXT("PlayerPrompt_DefaultNodeTitle", "Prompt");
    }
		
    return General.PromptLabelOverride;
}
#endif

#if WITH_EDITOR
FGameplayTagContainer UYapNodeConfig::GetMoodTags() const
{
    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	
    return TagsManager.RequestGameplayTagChildren(MoodTags.MoodTagsRoot);
}
#endif

#undef LOCTEXT_NAMESPACE