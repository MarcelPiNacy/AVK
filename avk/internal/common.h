#pragma once
#include <cstdint>
#include <atomic>
#include <intrin.h>

#if defined(_DEBUG) && !defined(NDEBUG)
#define DEBUG
#define BREAKPOINT DebugBreak()
#else
#define BREAKPOINT
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