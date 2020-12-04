#pragma once
#include "../internal/main_array.h"
#include <intrin.h>
#include <cassert>

template <typename T>
T fast_log2(T value)
{
	assert(value != 0);
	return (T)_tzcnt_u64(value);
}

template <typename T>
constexpr void guarded_insert(T* begin, T* end, const T& value)
{
	--end;
	for (; end >= begin && *end > value; --end)
		*(end + 1) = *end;
	*(end + 1) = value;
}

template <typename T>
constexpr void unguarded_insert(T* end)
{
	T tmp = *end;
	--end;
	for (; *end > tmp; --end)
		*(end + 1) = *end;
	*(end + 1) = tmp;
}