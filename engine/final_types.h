#pragma once

#include <inttypes.h>
#include <limits.h>
#include <float.h>

namespace fs {
	// Booleans
	typedef int32_t b32;

	// Unsigned
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	constexpr u8 U8_MAX = UINT8_MAX;
	constexpr u16 U16_MAX = UINT16_MAX;
	constexpr u32 U32_MAX = UINT32_MAX;
	constexpr u64 U64_MAX = UINT64_MAX;

	// Signed
	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
	typedef int64_t s64;
	constexpr s8 S8_MAX = INT8_MAX;
	constexpr s8 S8_MIN = INT8_MIN;
	constexpr s16 S16_MAX = INT16_MAX;
	constexpr s16 S16_MIN = INT16_MIN;
	constexpr s32 S32_MAX = INT32_MAX;
	constexpr s32 S32_MIN = INT32_MIN;
	constexpr s64 S64_MAX = INT64_MAX;
	constexpr s64 S64_MIN = INT64_MIN;

	// Floats
	typedef float f32;
	typedef double f64;
	constexpr f32 F32_MAX = FLT_MAX;
	constexpr f32 F32_MIN = -FLT_MAX;
	constexpr f64 F64_MAX = DBL_MAX;
	constexpr f64 F64_MIN = -DBL_MAX;
};