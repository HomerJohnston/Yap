// Repurposed from:
// https://github.com/Drakynfly/SquirrelUE
// Original by: Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Yap/YapSquirrelNoise.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Yap/SquirrelNoise5.hpp"

/*
 *					WARNING:
 *	READ BEFORE MAKING ANY CHANGES THIS FILE:
 *	This file generates seeded random numbers for
 *	game code. Any changes made here may affect
 *	generation such that existing seeds no longer
 *	function as they previously did. Only make
 *	changes to this file if you are aware of this,
 *	understand what you are doing, or don't care!
 */

#define LOCTEXT_NAMESPACE "Squirrel"

namespace YapSquirrel
{
	// The master seed used to set the game world to a consistent state that can be returned to.
	static uint32 GWorldSeed = 0;

	namespace Impl
	{
		constexpr uint32 SquirrelNoise5(int32& Position, const uint32 Seed)
		{
			return ::SquirrelNoise5(Position++, Seed);
		}
	}

	namespace Math
	{
		// The purpose of this function is to generate a random value in the full range of its type.
		// RandRange functions like that of FMath can only generate values in the range of ( Min/2+1 -> Max/2 ).
		template <typename T, typename RNG>
		constexpr T MaxRand(RNG Engine)
		{
			union
			{
				T Total;
				uint8 Pieces[sizeof(T)];
			} Value;

			for (size_t i = 0; i < sizeof Value.Pieces; i++)
			{
				Value.Pieces[i] = Engine();
			}

			return Value.Total;
		}

		// constexpr version of FMath::Ceil
		constexpr int64 SqCeil(const double Value)
		{
			const int64 Int = static_cast<int64>(Value);
			return Value > Int ? Int + 1 : Int;
		}

		// constexpr version of FMath::Floor
		constexpr int64 SqFloor(const double Value)
		{
			const int64 Int = static_cast<int64>(Value);
			return Value < Int ? Int - 1 : Int;
		}
	}

	uint32 HashCombine(int32 A, const int32 B)
	{
		return Impl::SquirrelNoise5(A, B);
	}

	uint32 GetGlobalSeed()
	{
		return GWorldSeed;
	}

	void SetGlobalSeed(const uint32 Seed)
	{
		GWorldSeed = Seed;
	}

	constexpr int32 NextInt32(FYapSquirrelState& State, const int32 Max)
	{
		return Max > 0 ? FMath::Min(FMath::TruncToInt(NextReal(State) * static_cast<double>(Max)), Max - 1) : 0;
	}

	constexpr int32 NextInt32InRange(FYapSquirrelState& State, const int32 Min, const int32 Max)
    {
		// @todo Min and Max must only cover *half* the int32 range, or it will cause an overflow in (Max - Min)
		// look into alternate ways to generate random values that don't have this limit

		const int32 Range = (Max - Min) + 1;
		return Min + NextInt32(State, Range);
    }

	constexpr double NextReal(FYapSquirrelState& State)
	{
		return Get1dNoiseZeroToOne(State.Position++, GWorldSeed);
	}

	constexpr double NextRealInRange(FYapSquirrelState& State, const double Min, const double Max)
	{
		// @todo Min and Max must only cover *half* the double range, or it will cause an overflow in (Max - Min)
		// look into alternate ways to generate random values that don't have this limit

		return Min + (Max - Min) * NextReal(State);
	}

	constexpr bool RollChance(FYapSquirrelState& State, double& Roll, const double Chance, const double RollModifier)
	{
		if (ensure((Chance >= 0.0) && (Chance <= 100.0)) ||
			ensure((RollModifier >= -100.0) && (RollModifier <= 100.0)))
		{
			// @todo log is broken with constexpr. fix/replace later
			//UE_LOG(LogSquirrel, Warning, TEXT("Bad inputs passed to USquirrel::RollChance - Chance: %f, Modifier: %f"), Chance, RollModifier);
		}

		Roll = NextRealInRange(State, 0.0, 100.0 - RollModifier) + RollModifier;
		return Roll >= Chance;
	}

	constexpr int32 RoundWithWeightByFraction(FYapSquirrelState& State, const double Value)
	{
		const double Whole = Math::SqFloor(Value);
		const double Remainder = Value - Whole;

		// If the Remainder equals to 0.f then always return the whole number.
		if (Remainder <= 0.0)
		{
			return Whole;
		}

		// Otherwise, return the whole number plus the random weighted bool.
		return Whole + (Remainder >= NextReal(State));
	}
}

#if WITH_EDITOR
void FYapSquirrelState::RandomizeState()
{
	// It is allowable and expected to get a non-seeded random value in the editor, hence using 'Rand' here.
	Position = YapSquirrel::Math::MaxRand<int32>(FMath::Rand);
}
#endif

UYapSquirrel::UYapSquirrel()
{
}

void UYapSquirrel::PostInitProperties()
{
	Super::PostInitProperties();

	if (!IsTemplate())
	{
		if (const UWorld* World = GetWorld())
		{
			// At runtime, Squirrels should be given random (but still seeded) positions
			if (World->HasBegunPlay() && GEngine)
			{
				if (UYapSquirrelSubsystem* Subsystem = GEngine->GetEngineSubsystem<UYapSquirrelSubsystem>())
				{
					State.Position = Subsystem->NewPosition();
				}
			}
#if WITH_EDITOR
			// In the editor, they should be given a new one in any-case
			else if (GIsEditor)
			{
				State.RandomizeState();
			}
#endif
		}
	}
}

void UYapSquirrel::Jump(const int32 NewPosition)
{
	State.Position = NewPosition;
}

int32 UYapSquirrel::GetPosition() const
{
	return State.Position;
}

int32 UYapSquirrel::NextInt32(const int32 Max)
{
	return YapSquirrel::NextInt32(State, Max);
}

int32 UYapSquirrel::NextInt32InRange(const int32 Min, const int32 Max)
{
	return YapSquirrel::NextInt32InRange(State, Min, Max);
}

bool UYapSquirrel::NextBool()
{
	return YapSquirrel::Next<bool>(State);
}

double UYapSquirrel::NextReal()
{
	return YapSquirrel::NextReal(State);
}

double UYapSquirrel::NextRealInRange(const double Min, const double Max)
{
	return YapSquirrel::NextRealInRange(State, Min, Max);
}

bool UYapSquirrel::RollChance(double& Roll, const double Chance, const double RollModifier)
{
	return YapSquirrel::RollChance(State, Roll, Chance, RollModifier);
}

int32 UYapSquirrel::RoundWithWeightByFraction(const double Value)
{
	return YapSquirrel::RoundWithWeightByFraction(State, Value);
}

void UYapSquirrelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	if (GIsEditor)
	{
		RuntimePositionsSquirrel.RandomizeState();
	}
#endif
}

void UYapSquirrelSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

int32 UYapSquirrelSubsystem::NewPosition()
{
	return YapSquirrel::Next<int32>(RuntimePositionsSquirrel);
}

int64 UYapSquirrelSubsystem::GetGlobalSeed() const
{
	return YapSquirrel::GetGlobalSeed();
}

void UYapSquirrelSubsystem::SetGlobalSeed(const int64 NewSeed)
{
	YapSquirrel::SetGlobalSeed(NewSeed);
}

FYapSquirrelWorldState UYapSquirrelSubsystem::SaveWorldState() const
{
	return FYapSquirrelWorldState{YapSquirrel::GetGlobalSeed(), RuntimePositionsSquirrel };
}

void UYapSquirrelSubsystem::LoadGameState(const FYapSquirrelWorldState State)
{
	YapSquirrel::SetGlobalSeed(State.GlobalSeed);
	RuntimePositionsSquirrel = State.RuntimeState;
}

#undef LOCTEXT_NAMESPACE