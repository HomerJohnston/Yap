//-----------------------------------------------------------------------------------------------
// SquirrelNoise5.hpp
//
#pragma once


/////////////////////////////////////////////////////////////////////////////////////////////////
// SquirrelNoise5 - Squirrel's Raw Noise utilities (version 5)
//
// This code is made available under the Creative Commons attribution 3.0 license (CC-BY-3.0 US):
//	Attribution in source code comments (even closed-source/commercial code) is sufficient.
//	License summary and text available at: https://creativecommons.org/licenses/by/3.0/us/
//
// These noise functions were written by Squirrel Eiserloh as a cheap and simple substitute for
//	the [sometimes awful] bit-noise sample code functions commonly found on the web, many of which
//	are hugely biased or terribly patterned, e.g. having bits which are on (or off) 75% or even
//	100% of the time (or are excessively overkill/slow for our needs, such as MD5 or SHA).
//
// Note: This is work in progress; not all functions have been tested.  Use at your own risk.
//	Please report any bugs, issues, or bothersome cases to SquirrelEiserloh at gmail.com.
//
// The following functions are all based on a simple bit-noise hash function which returns an
//	unsigned integer containing 32 reasonably-well-scrambled bits, based on a given (signed)
//	integer input parameter (position/index) and [optional] seed.  Kind of like looking up a
//	value in an infinitely large [non-existent] table of previously rolled random numbers.
//
// These functions are deterministic and random-access / order-independent (i.e. state-free),
//	so they are particularly well-suited for use in smoothed/fractal/simplex/Perlin noise
//	functions and out-of-order (or or-demand) procedural content generation (i.e. that mountain
//	village is the same whether you generated it first or last, ahead of time or just now).
//
// The N-dimensional variations simply hash their multidimensional coordinates down to a single
//	32-bit index and then proceed as usual, so while results are not unique they should
//	(hopefully) not seem locally predictable or repetitive.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

using FSquirrelReal = double;

//-----------------------------------------------------------------------------------------------
// Raw pseudorandom noise functions (random-access / deterministic).  Basis of all other noise.
//
constexpr uint32 Get1dNoiseUint(int32 Index, uint32 Seed=0);
constexpr uint32 Get2dNoiseUint(int32 IndexX, int32 IndexY, uint32 Seed=0);
constexpr uint32 Get3dNoiseUint(int32 IndexX, int32 IndexY, int32 IndexZ, uint32 Seed=0);
constexpr uint32 Get4dNoiseUint(int32 IndexX, int32 IndexY, int32 IndexZ, int32 IndexT, uint32 Seed=0);

//-----------------------------------------------------------------------------------------------
// Same functions, mapped to floats in [0,1] for convenience.
//
constexpr FSquirrelReal Get1dNoiseZeroToOne(int32 Index, uint32 Seed=0);
constexpr FSquirrelReal Get2dNoiseZeroToOne(int32 IndexX, int32 IndexY, uint32 Seed=0);
constexpr FSquirrelReal Get3dNoiseZeroToOne(int32 IndexX, int32 IndexY, int32 IndexZ, uint32 Seed=0);
constexpr FSquirrelReal Get4dNoiseZeroToOne(int32 IndexX, int32 IndexY, int32 IndexZ, int32 IndexT, uint32 Seed=0);

//-----------------------------------------------------------------------------------------------
// Same functions, mapped to floats in [-1,1] for convenience.
//
constexpr FSquirrelReal Get1dNoiseNegOneToOne(int32 Index, uint32 Seed=0);
constexpr FSquirrelReal Get2dNoiseNegOneToOne(int32 IndexX, int32 IndexY, uint32 Seed=0);
constexpr FSquirrelReal Get3dNoiseNegOneToOne(int32 IndexX, int32 IndexY, int32 IndexZ, uint32 Seed=0);
constexpr FSquirrelReal Get4dNoiseNegOneToOne(int32 IndexX, int32 IndexY, int32 IndexZ, int32 IndexT, uint32 Seed=0);


/////////////////////////////////////////////////////////////////////////////////////////////////
// Inline function definitions below
/////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
// Fast hash of an int32 into a different (unrecognizable) uint32.
//
// Returns an unsigned integer containing 32 reasonably-well-scrambled bits, based on the hash
//	of a given (signed) integer input parameter (position/index) and [optional] seed.  Kind of
//	like looking up a value in an infinitely large table of previously generated random numbers.
//
// I call this particular approach SquirrelNoise5 (5th iteration of my 1D raw noise function).
//
// Many thanks to Peter Schmidt-Nielsen whose outstanding analysis helped identify a weakness
//	in the SquirrelNoise3 code I originally used in my GDC 2017 talk, "Noise-based RNG".
//	Version 5 avoids a noise repetition found in version 3 at extremely high position values
//	caused by a lack of influence by some of the high input bits onto some of the low output bits.
//
// The revised SquirrelNoise5 function ensures all input bits affect all output bits, and to
//	(for me) a statistically acceptable degree.  I believe the worst-case here is in the amount
//	of influence input position bit #30 has on output noise bit #0 (49.99%, vs. 50% ideal).
//
constexpr uint32 SquirrelNoise5(const int32 Position, const uint32 Seed)
{
	constexpr uint32 SQ5_BIT_NOISE1 = 0xd2a80a3f;	// 11010010101010000000101000111111
	constexpr uint32 SQ5_BIT_NOISE2 = 0xa884f197;	// 10101000100001001111000110010111
	constexpr uint32 SQ5_BIT_NOISE3 = 0x6C736F4B;	// 01101100011100110110111101001011
	constexpr uint32 SQ5_BIT_NOISE4 = 0xB79F3ABB;	// 10110111100111110011101010111011
	constexpr uint32 SQ5_BIT_NOISE5 = 0x1b56c4f5;	// 00011011010101101100010011110101

	uint32 MangledBits = static_cast<uint32>(Position);
	MangledBits *= SQ5_BIT_NOISE1;
	MangledBits += Seed;
	MangledBits ^= MangledBits >> 9;
	MangledBits += SQ5_BIT_NOISE2;
	MangledBits ^= MangledBits >> 11;
	MangledBits *= SQ5_BIT_NOISE3;
	MangledBits ^= MangledBits >> 13;
	MangledBits += SQ5_BIT_NOISE4;
	MangledBits ^= MangledBits >> 15;
	MangledBits *= SQ5_BIT_NOISE5;
	MangledBits ^= MangledBits >> 17;
	return MangledBits;
}

#define ONE_OVER_MAX_UINT (1.0 / static_cast<double>(TNumericLimits<uint32>::Max()))
#define ONE_OVER_MAX_INT (1.0 / static_cast<double>(TNumericLimits<int32>::Max()))
#define PRIME1 198491317 // Large prime number with non-boring bits
#define PRIME2 6542989 // Large prime number with distinct and non-boring bits
#define PRIME3 357239 // Large prime number with distinct and non-boring bits

constexpr uint32 Get1dNoiseUint(const int32 Index, const uint32 Seed)
{
	return SquirrelNoise5(Index, Seed);
}

constexpr uint32 Get2dNoiseUint(const int32 IndexX, const int32 IndexY, const uint32 Seed)
{
	return SquirrelNoise5(IndexX + PRIME1 * IndexY, Seed);
}

constexpr uint32 Get3dNoiseUint(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const uint32 Seed)
{
	return SquirrelNoise5(IndexX + PRIME1 * IndexY + PRIME2 * IndexZ, Seed);
}

constexpr uint32 Get4dNoiseUint(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const int32 IndexT, const uint32 Seed)
{
	return SquirrelNoise5(IndexX + PRIME1 * IndexY + PRIME2 * IndexZ + PRIME3 * IndexT, Seed);
}

constexpr FSquirrelReal Get1dNoiseZeroToOne(const int32 Index, const uint32 Seed)
{
	return ONE_OVER_MAX_UINT * static_cast<double>(SquirrelNoise5(Index, Seed));
}

constexpr FSquirrelReal Get2dNoiseZeroToOne(const int32 IndexX, const int32 IndexY, const uint32 Seed)
{
	return ONE_OVER_MAX_UINT * static_cast<double>(Get2dNoiseUint(IndexX, IndexY, Seed));
}

constexpr FSquirrelReal Get3dNoiseZeroToOne(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const uint32 Seed)
{
	return ONE_OVER_MAX_UINT * static_cast<double>(Get3dNoiseUint(IndexX, IndexY, IndexZ, Seed));
}

constexpr FSquirrelReal Get4dNoiseZeroToOne(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const int32 IndexT, const uint32 Seed)
{
	return ONE_OVER_MAX_UINT * static_cast<double>(Get4dNoiseUint(IndexX, IndexY, IndexZ, IndexT, Seed));
}

constexpr FSquirrelReal Get1dNoiseNegOneToOne(const int32 Index, const uint32 Seed)
{
	return ONE_OVER_MAX_INT * static_cast<double>(static_cast<int32>(SquirrelNoise5(Index, Seed)));
}

constexpr FSquirrelReal Get2dNoiseNegOneToOne(const int32 IndexX, const int32 IndexY, const uint32 Seed)
{
	return ONE_OVER_MAX_INT * static_cast<double>(static_cast<int32>(Get2dNoiseUint(IndexX, IndexY, Seed)));
}

constexpr FSquirrelReal Get3dNoiseNegOneToOne(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const uint32 Seed)
{
	return ONE_OVER_MAX_INT * static_cast<double>(static_cast<int32>(Get3dNoiseUint(IndexX, IndexY, IndexZ, Seed)));
}

constexpr FSquirrelReal Get4dNoiseNegOneToOne(const int32 IndexX, const int32 IndexY, const int32 IndexZ, const int32 IndexT, const uint32 Seed)
{
	return ONE_OVER_MAX_INT * static_cast<double>(static_cast<int32>(Get4dNoiseUint(IndexX, IndexY, IndexZ, IndexT, Seed)));
}