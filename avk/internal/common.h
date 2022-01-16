#pragma once
#include <cstdint>
#include <atomic>
#include <bit>

#if defined(_DEBUG) || !defined(NDEBUG)
void avk_assertion_handler(const wchar_t* expression);
void avk_assertion_handler(const char* expression);
#define DEBUG
#define AVK_BREAKPOINT __debugbreak()
#define AVK_ASSERT(expression) if (!(expression)) { AVK_BREAKPOINT; avk_assertion_handler(#expression); }
#else
#define AVK_BREAKPOINT
#define AVK_ASSERT(expression) (expression)
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

template <typename T>
constexpr uint8_t floor_log2(T value)
{
	return (uint8_t)((sizeof(T) * 8 - 1) - std::countl_zero(value));
}

template <typename T>
constexpr uint8_t ceil_log2(T value)
{
	return (uint8_t)((sizeof(T) * 8) - std::countl_zero(value - 1));
}

template <typename T>
constexpr T round_pow2(T value)
{
	return (T)1 << ceil_log2(value);
}