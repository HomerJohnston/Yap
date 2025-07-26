// Originally written by Kyle Wilcox (HoJo/Homer Johnston), Ghost Pepper Games Inc.
// This file is public domain.

#pragma once

#include "GameplayTagsManager.h"

#if WITH_EDITOR
#include "PackageTools.h"
#endif

typedef TDelegate<const FGameplayTag&()> FGameplayTagFilterDelegate;

/**
 * - Use this copy-pastable header to make it easier to implement dynamic gameplay tag filters onto UObject properties.
 * - Inherit from this class, e.g.
 *
 *      class UE_API UYourFancyObject : public UObject, public GameplayTagFilterHelper<UYourFancyObject>
 *      {
 *          UPROPERTY(...)
 *          FGameplayTag WeaponType;
 *
 *          UPROPERTY(...)
 *          TArray<FWeaponDefinitions> WeaponDefinitions;
 *      }
 *
 * - In your constructor, register filters for any gameplay tag properties.
 * - When referencing sub-properties of structs or collections, use a dot to separate each level, e.g. MemberArray[6]->StructPropertyName would become MemberArray.StructPropertyName
 * - The property path MUST originate from "ThisClass" so if it's nested you may not be able to use GET_MEMBER_NAME_CHECKED.
 * - Example:
 * 
 *      UYourFancyObject::UYourFancyObject()
 *      {
 *          AddGameplayTagFilter(GET_MEMBER_NAME_CHECKED(ThisClass, WeaponType), FGameplayTagFilterDelegate::CreateUObject(this, &ThisClass::GetWeaponTypesRootTag));
 *          AddGameplayTagFilter("WeaponDefinitions.Thing", FGameplayTagFilterDelegate::CreateStatic(&ThisClass::GetWeaponTypesRootTag));
 *      }
 */
template<typename T>
class FGameplayTagFilterHelper
{
public:
    /** Use this function to register filters.
     *
     * PropertyPath: a path to the FGameplayTag property that we want to filter. Use dots to drive down one level, e.g. ArrayPropertyOfStructs.TagPropertyInsideStruct
     *
     * Func: a function to return an FGameplayTag to use as a parent in the selector widget, e.g. `const FGameplayTag&() const { return GetSomeParentTag(); }`
     */
    void AddGameplayTagFilter(FName PropertyPath, FGameplayTagFilterDelegate Func)
    {
        #if WITH_EDITOR
        if (static_cast<T*>(this)->IsTemplate())
        {
            RegisteredFilters.Add(PropertyPath.ToString(), Func);
        }
        #endif
    }
    
    // Internal. Holds registered filters.
    TMap<FString, FGameplayTagFilterDelegate> RegisteredFilters;

    // Internal. Registers the tag filter on startup using the CDO initialization.
    FGameplayTagFilterHelper()
    {
        #if WITH_EDITOR
        if (static_cast<T*>(this)->IsTemplate())
        {
            UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

            TagsManager.OnGetCategoriesMetaFromPropertyHandle.AddUObject(static_cast<T*>(this), &T::FilterGameplayTags);   
        }
        #endif
    }

    // Internal. Does nothing.
    virtual ~FGameplayTagFilterHelper()
    {
        #if WITH_EDITOR
        // Crashes on shutdown. For now this helper will simply exist for the duration of the program.
        // if (static_cast<T*>(this)->IsTemplate())
        // {
        //    UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
        //
        //    TagsManager.OnGetCategoriesMetaFromPropertyHandle.RemoveAll(this);   
        //}
        #endif
    }
    
    #if WITH_EDITOR
    void FilterGameplayTags(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString)
    {
        TArray<UObject*> OuterObjects;
        PropertyHandle->GetOuterObjects(OuterObjects);

        // This function will run for every gameplay tag property widget that gets spawned in a details panel.
        // We'll start by looking for an outer object that is of type 'T'...
        for (UObject* Object : OuterObjects)
        {
            UObject* Outer = Object;
            
            while (Outer)
            {
                const UObject* Target = Outer;

                // When the property is inside of an asset's detail panel, the outer will be a package instead of a blueprint class.
                // Check for this case and, if so, find the actual outer class object...
                if (UPackage* Package = Cast<UPackage>(Outer))
                {
                    TArray<UObject*> Objects;
                    TArray<UPackage*> Packages = {Package};
                    UPackageTools::GetObjectsInPackages(&Packages, Objects);
                    
                    for (UObject* TestObject : Objects)
                    {
                        if (TestObject->GetClass() == T::StaticClass())
                        {
                            // We found us!
                            Target = TestObject;
                            break;
                        }
                    }
                }

                const T* OuterAsTPtr = Cast<T>(Target);

                if (OuterAsTPtr)
                {
                    // We now have an outer which is of our type T!
                    // Next check if the gameplay tag being drawn (the IPropertyHandle above) actually has a func registered.
                    FString PropertyPath (PropertyHandle->GetPropertyPath());

                    PropertyPath = PropertyPath.Replace(TEXT("->"), TEXT("."));

                    FRegexPattern ArrayPattern("\\[(\\d+)\\]");
                    FRegexMatcher ArrayMatcher(ArrayPattern, PropertyPath);

                    while (ArrayMatcher.FindNext())
                    {
                        // Remove array index from the property path
                        PropertyPath = PropertyPath.Left(ArrayMatcher.GetMatchBeginning()) + PropertyPath.Mid(ArrayMatcher.GetMatchEnding());
                    }
                    
                    FGameplayTagFilterDelegate* FuncPtr = RegisteredFilters.Find(PropertyPath);

                    if (FuncPtr)
                    {
                        const FGameplayTag& NewMetaString = (*FuncPtr).Execute();

                        if (NewMetaString.IsValid())
                        {
                            MetaString = NewMetaString.ToString();
                            return;
                        }
                    }
                    
                    return;
                }
                
                Outer = Outer->GetOuter();
            }
        }
    }
    #endif
};
