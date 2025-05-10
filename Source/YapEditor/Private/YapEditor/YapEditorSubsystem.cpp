// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorSubsystem.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "ILiveCodingModule.h"
#include "YapEditor/YapEditorColor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Yap/YapCharacterAsset.h"
#include "YapEditor/YapInputTracker.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorStyle.h"
#include "Engine/Texture2D.h"
#include "UObject/ObjectSaveContext.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "YapEditor/Globals/YapTagHelpers.h"

#define LOCTEXT_NAMESPACE "YapEditor"

bool UYapEditorSubsystem::bLiveCodingInProgress = true;
TArray<TWeakObjectPtr<UObject>> UYapEditorSubsystem::OpenedAssets = {};


void UYapEditorSubsystem::UpdateMoodTagBrushesIfRequired()
{
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();
	
	if (ProjectMoodTags.Num() != CachedMoodTags.Num() || !CachedMoodTags.HasAllExact(ProjectMoodTags))
	{
		UpdateMoodTagBrushes();
	}

	CachedMoodTags = ProjectMoodTags;
}

void UYapEditorSubsystem::UpdateMoodTagBrushes()
{
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();
	
	MoodTagIconBrushes.Empty(ProjectMoodTags.Num() + 1);

	for (const FGameplayTag& MoodTag : ProjectMoodTags)
	{
		BuildIcon(MoodTag);
	}

	BuildIcon(FGameplayTag::EmptyTag);

	CachedMoodTags = ProjectMoodTags;
}

void UYapEditorSubsystem::BuildIcon(const FGameplayTag& MoodTag)
{
	TSharedPtr<FSlateImageBrush> ImageBrush = nullptr;
	
	// Attempt to load SVG
	FString IconPath = UYapProjectSettings::GetMoodTagIconPath(MoodTag, "svg");
	ImageBrush = MakeShared<FSlateVectorImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);

	// Attempt to load PNG
	if (!ImageBrush)
	{
		IconPath = UYapProjectSettings::GetMoodTagIconPath(MoodTag, "png");
		ImageBrush = MakeShared<FSlateImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);
	}
	
	// Found nothing
	if (!ImageBrush)
	{
		return;
	}

	MoodTagIconBrushes.Add(MoodTag, ImageBrush);
}

TSharedPtr<FSlateImageBrush> UYapEditorSubsystem::GetMoodTagIcon(FGameplayTag MoodTag)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(MoodTag);

	if (Brush)
	{
		return *Brush;
	}

	return nullptr;
}

const FSlateBrush* UYapEditorSubsystem::GetMoodTagBrush(FGameplayTag Name)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(Name);

	return Brush ? Brush->Get() : FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodTag_Missing);
}

TSharedPtr<FSlateImageBrush> UYapEditorSubsystem::GetCharacterPortraitBrush(const UObject* Character, const FGameplayTag& MoodTag)
{
	static int32 iii = 0;

	if (!IsValid(Character))
	{
		return nullptr;
	}

	// TODO I should somehow cache this so that I'm not calling slow interface calls on tick in the widget
	// Or switch to event based updates

	const IYapCharacterInterface* SpeakerInterface = Cast<IYapCharacterInterface>(Character);

	const UTexture2D* Texture = nullptr;

	if (SpeakerInterface)
	{
		Texture = SpeakerInterface->GetYapCharacterPortrait(MoodTag);
	}
	else if (Character->Implements<UYapCharacterInterface>())
	{
		Texture = IYapCharacterInterface::Execute_K2_GetYapCharacterPortrait(Character, MoodTag);
	}	

	if (!Texture)
	{
		if (UYapProjectSettings::GetDefaultPortraitTextureAsset().IsNull())
		{
			return nullptr;
		}

		if (UYapProjectSettings::GetDefaultPortraitTextureAsset().IsValid())
		{
			Texture = Get()->MissingPortraitTexture = UYapProjectSettings::GetDefaultPortraitTextureAsset().Get();
		}
		else
		{
			Texture = Get()->MissingPortraitTexture = UYapProjectSettings::GetDefaultPortraitTextureAsset().LoadSynchronous();
		}
	}
	
	TSharedPtr<FSlateImageBrush>* PortraitBrushPtr = Get()->CharacterPortraitBrushes.Find(Texture);

	if (PortraitBrushPtr)
	{
		// TODO: somehow check if it is out of date
		return *PortraitBrushPtr;
	}

	TSharedPtr<FSlateImageBrush> NewPortraitBrush = MakeShared<FSlateImageBrush>((UObject*)Texture, FVector2D(128, 128));

	Get()->CharacterPortraitBrushes.Add(Texture, NewPortraitBrush);
	
	/*
	NewPortraitBrush.ImageSize = FVector2D(128, 128);
	NewPortraitBrush.SetResourceObject(const_cast<UTexture2D*>(Texture)); // TODO make sure this is safe and that there isn't a better system
	NewPortraitBrush.SetUVRegion(FBox2D(FVector2D(0,0), FVector2D(1,1)));
	NewPortraitBrush.DrawAs = ESlateBrushDrawType::Box;
	NewPortraitBrush.Margin = 0;
	*/

	return NewPortraitBrush;
}

void UYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (UGameplayTagsManager::Get().FindTagSource("YapGameplayTags.ini") == nullptr)
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagSource("YapGameplayTags.ini");
	}

	// TODO -- this does NOT work! Keep it hidden until I can figure out why. For some reason FSlateSVGRasterizer will NOT attempt to load a file that didn't exist on startup?
	// Also see: FDetailCustomization_YapProjectSetting for hidden/disabled button to run UpdateMoodTagBrushes().
	//UGameplayTagsManager::Get().OnEditorRefreshGameplayTagTree.AddUObject(this, &UYapEditorSubsystem::UpdateMoodTagBrushesIfRequired);
	UpdateMoodTagBrushes();

	if (IsValid(GUnrealEd))
	{
		InputTracker = MakeShared<FYapInputTracker>(this);
		auto& SlateApp = FSlateApplication::Get();
		SlateApp.RegisterInputPreProcessor(InputTracker);
	}

	SetupGameplayTagFiltering();
	
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddUObject(this, &UYapEditorSubsystem::OnPatchComplete);
	}
#endif

	FCoreUObjectDelegates::OnObjectPreSave.AddUObject(this, &ThisClass::OnObjectPresave);
}

void UYapEditorSubsystem::Deinitialize()
{
	if (IsValid(GUnrealEd))
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputTracker);
	}

	Super::Deinitialize();
}

void UYapEditorSubsystem::OnObjectPresave(UObject* Object, FObjectPreSaveContext Context)
{
	if (Object->IsA(UFlowAsset::StaticClass()))
	{
		CleanupDialogueTags();
	}
}

void UYapEditorSubsystem::CleanupDialogueTags()
{
	for (auto It = TagsPendingDeletion.CreateIterator(); It; ++It)
	{
		const FGameplayTag& Tag = *It;
		if (UGameplayTagsManager::Get().FindTagNode(Tag.GetTagName()) == nullptr)
		{
			It.RemoveCurrentSwap();
		}
	}
	
	Yap::Tags::DeleteTags(TagsPendingDeletion);
}

FYapInputTracker* UYapEditorSubsystem::GetInputTracker()
{
	return InputTracker.Get();
}

void UYapEditorSubsystem::SetupGameplayTagFiltering()
{
	FragmentTagFilterDelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
}

void UYapEditorSubsystem::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle->HasMetaData("Yap"))
	{
		return;
	}

	if (IsMoodTagProperty(PropertyHandle))
	{
		MetaString = GetDefault<UYapProjectSettings>()->GetMoodTagsParent().ToString();
		return;
	}
}

bool UYapEditorSubsystem::IsMoodTagProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	TArray<FName> PropertyNames
	{
		"MoodTag",
		"MoodTags",
		"MoodTags2"
	};
	
	if (PropertyNames.Contains(PropertyHandle->GetProperty()->GetFName()))
	{
		return true;
	}
		
	return false;
}

void UYapEditorSubsystem::UpdateLiveCodingState(bool bNewState)
{
	if (bNewState == bLiveCodingInProgress)
	{
		return;
	}

	bLiveCodingInProgress = bNewState;

	if (UYapDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
	{
		UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

		if (bLiveCodingInProgress)
		{
			TArray<UObject*> OpenedAssetsTemp = Subsystem->GetAllEditedAssets();

			for (UObject* Obj : OpenedAssetsTemp)
			{
				OpenedAssets.Add(Obj);
			}
		
			Subsystem->CloseAllAssetEditors();
		}
		else
		{
			GEditor->GetTimerManager()->SetTimerForNextTick(this, &UYapEditorSubsystem::ReOpenAssets);
		}
	}
}

void UYapEditorSubsystem::ReOpenAssets()
{
	UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	if (IsValid(Subsystem))
	{
		for (TWeakObjectPtr<UObject> Asset : OpenedAssets)
		{
			if (Asset.IsValid())
			{
				Subsystem->OpenEditorForAsset(Asset.Get());
			}
		}
	}

	OpenedAssets.Empty();
}

void UYapEditorSubsystem::Tick(float DeltaTime)
{
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		if (LiveCoding->IsCompiling())
		{
			UpdateLiveCodingState(true);
		}
		else
		{
			UpdateLiveCodingState(false);
		}
	}
#endif
}

TStatId UYapEditorSubsystem::UYapEditorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UYapEditorSubsystem, STATGROUP_Tickables);
}

void UYapEditorSubsystem::AddTagPendingDeletion(FGameplayTag Tag)
{	
	Get()->TagsPendingDeletion.AddUnique(Tag);

	// To avoid any noticeable issues, arbitrarily limit this feature to tracking 100 dead tags. If the list ever grows larger than this, it will require a manual cleanup run.
	if (Get()->TagsPendingDeletion.Num() > 100)
	{
		Get()->TagsPendingDeletion.RemoveAt(0, EAllowShrinking::No);
	}
}

void UYapEditorSubsystem::RemoveTagPendingDeletion(FGameplayTag Tag)
{
	Get()->TagsPendingDeletion.Remove(Tag);
}


void UYapEditorSubsystem::OnPatchComplete()
{
	if (UYapDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
	{
		ReOpenAssets();
	}
	
	bLiveCodingInProgress = false;
}

#undef LOCTEXT_NAMESPACE
