// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagsManager.h"
#include "PackageTools.h"
#include <type_traits>

/**
 * Use this copy-pastable header to make it easier to implement dynamic gameplay tag filters onto UObject properties.
 *
 * - Inherit from this class, e.g.
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
 *
 *      UYourFancyObject::UYourFancyObject()
 *      {
 *          AddGameplayTagFilter(GET_MEMBER_NAME_CHECKED(ThisClass, WeaponType), &UYourFancyObject::GetWeaponTypesRootTag);
 *          AddGameplayTagFilter("WeaponDefinitions.Thing", &UYourFancyObject::GetWeaponTypesRootTag);
 *      }
 * 
 *  That's it!
 */
template<typename T>
class FGameplayTagFilterHelper
{
    // Internal
    typedef const FGameplayTag& (T::*GameplayTagMemberFnPtr)() const;
    typedef const FGameplayTag& (*GameplayTagStaticFnPtr)();

public:
    /** Use this function to register filters */
    void AddGameplayTagFilter(FName PropertyPath, GameplayTagMemberFnPtr Func)
    {
        #if WITH_EDITOR
        if (static_cast<T*>(this)->IsTemplate())
        {
            RegisteredMemberFilters.Add(PropertyPath.ToString(), Func);
        }
        #endif
    }

    /** Use this function to register filters */
    void AddGameplayTagFilterStatic(FName PropertyPath, GameplayTagStaticFnPtr Func)
    {
#if WITH_EDITOR
        if (static_cast<T*>(this)->IsTemplate())
        {
            RegisteredStaticFilters.Add(PropertyPath.ToString(), Func);
        }
#endif
    }
    
    // Internal. Holds registered filters.
    TMap<FString, GameplayTagMemberFnPtr> RegisteredMemberFilters;
    TMap<FString, GameplayTagStaticFnPtr> RegisteredStaticFilters;

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
                    
                    GameplayTagMemberFnPtr* FuncPtr = RegisteredMemberFilters.Find(PropertyPath);

                    if (FuncPtr)
                    {
                        const FGameplayTag& NewMetaString = std::invoke(*FuncPtr, *OuterAsTPtr);

                        if (NewMetaString.IsValid())
                        {
                            MetaString = NewMetaString.ToString();
                            return;
                        }
                    }

                    GameplayTagStaticFnPtr* FuncPtrStatic = RegisteredStaticFilters.Find(PropertyPath);

                    if (FuncPtrStatic)
                    {
                        const FGameplayTag& NewMetaString = (*FuncPtrStatic)();

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
