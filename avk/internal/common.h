#pragma once
#include <cstdint>
#include <atomic>
#define VC_EXTRALEAN
#include <Windows.h>

#if defined(_DEBUG) || !defined(NDEBUG)
void avk_assertion_handler(const wchar_t* expression);
void avk_assertion_handler(const char* expression);
#define DEBUG
#define AVK_BREAKPOINT DebugBreak()
#define AVK_ASSERT(expression) if (!(expression)) { AVK_BREAKPOINT; avk_assertion_handler(#expression); }
#else
#define AVK_BREAKPOINT
#define AVK_ASSERT(expression) (expression)
#endif

#ifdef AVK_32BIT_INDICES
using sint = int32_t;
using uint = uint32_t;
#else
using sint = int32_t;
using uint = uint32_t;
#endif

template <typename R, typename... P>
using function_ptr = R(*)(P...);

template <typename T>
constexpr auto c_array_size(T&& graphics_array)
{
	return sizeof(graphics_array) / sizeof(graphics_array[0]);
}

template <typename T>
T non_atomic_load(const std::atomic<T>& from)
{
	return *(const T*)&from;
}

template <typename T, typename U = T>
void non_atomic_store(std::atomic<T>& where, U&& value)
{
	new ((T*)&where) T(std::forward<U>(value));
}

inline uint8_t fast_log2(uint32_t value)
{
	return (uint8_t)_tzcnt_u32(value);
}

inline uint8_t fast_log2(uint64_t value)
{
	return (uint8_t)_tzcnt_u64(value);
}

inline uint32_t round_pow2(uint32_t value)
{
	uint32_t log2 = (uint32_t)fast_log2(value);
	return 1U << log2;
}

inline uint64_t round_pow2(uint64_t value)
{
	uint64_t log2 = (uint64_t)fast_log2(value);
	return 1ULL << log2;
}