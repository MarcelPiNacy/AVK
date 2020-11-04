#pragma once
#include <cstdint>
#include <utility>
#include <iterator>



#if (defined(_DEBUG) || !defined(NDEBUG)) && !defined(GRAILSORT_DEBUG)
#define GRAILSORT_DEBUG
#endif



#ifdef _MSVC_LANG
#define GRAILSORT_ASSUME(expression) __assume((expression))
#ifdef GRAILSORT_DEBUG
#include <cassert>
// Assumes "expression" has no side-effects!
#define GRAILSORT_INVARIANT(expression) assert(expression)
#else
// Assumes "expression" has no side-effects!
#define GRAILSORT_INVARIANT(expression) GRAILSORT_ASSUME(expression)
#endif
#else
#define GRAILSORT_ASSUME(expression)
#ifdef GRAILSORT_DEBUG
#include <cassert>
// Assumes "expression" has no side-effects!
#define GRAILSORT_INVARIANT(expression) assert(expression)
#else
// Assumes "expression" has no side-effects!
#define GRAILSORT_INVARIANT(expression)
#endif
#endif



#ifndef GRAILSORT_NOTHROW
#define GRAILSORT_NOTHROW
#endif



namespace grail_sort::detail
{
	constexpr size_t pop_count(size_t value) GRAILSORT_NOTHROW
	{
#ifdef GRAILSORT_USE_BUILTIN
#if defined(_MSC_VER) || defined(_MSVC_LANG)
#if UINT32_MAX == UINTPTR_MAX
		return __popcnt(value);
#else
		return __popcnt64(value);
#endif
#endif
#else
		size_t r = 0;
		for (; value != 0; value >>= 1)
			r += value & 1;
		return r;
#endif
	}

	constexpr size_t fast_log2(size_t value) GRAILSORT_NOTHROW
	{
#ifdef GRAILSORT_USE_BUILTIN
#if defined(_MSC_VER) || defined(_MSVC_LANG)
#if UINT32_MAX == UINTPTR_MAX
		return _tzcnt_u32(value);
#else
		return _tzcnt_u64(value);
#endif
#endif
#else
#if UINTPTR_MAX == UINT64_MAX

		constexpr uint8_t lookup[] =
		{
			63, 0, 58, 1, 59, 47, 53, 2,
			60, 39, 48, 27, 54, 33, 42, 3,
			61, 51, 37, 40, 49, 18, 28, 20,
			55, 30, 34, 11, 43, 14, 22, 4,
			62, 57, 46, 52, 38, 26, 32, 41,
			50, 36, 17, 19, 29, 10, 13, 21,
			56, 45, 25, 31, 35, 16, 9, 12,
			44, 24, 15, 8, 23, 7, 6, 5
		};

		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value |= value >> 32;

		return lookup[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
#else
		constexpr uint8_t lookup[] =
		{
			0, 9, 1, 10, 13, 21, 2, 29,
			11, 14, 16, 18, 22, 25, 3, 30,
			8, 12, 20, 28, 15, 17, 24, 7,
			19, 27, 23, 6, 26, 5, 4, 31
		};

		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;

		return lookup[(uint32_t)(value * 0x07C4ACDD) >> 27];
#endif
#endif
	}

	template <typename T, bool CheckPow2 = true, bool CheckLesser = true>
	constexpr T smart_modulo(T value, T divisor) GRAILSORT_NOTHROW
	{
		if constexpr (!CheckPow2)
		{
			if (pop_count(divisor) == 1)
				return value & (divisor - 1);
		}

		if constexpr (CheckLesser)
		{
			if (value < divisor)
				return value;
		}

		return value % divisor;
	}

	template <typename T>
	constexpr int_fast8_t compare(const T& left, const T& right) GRAILSORT_NOTHROW
	{
		if (left == right)
			return 0;
		int_fast8_t result = 1UI8;
		if (left < right)
			result = -1UI8;
		return result;
	}

	template <typename T>
	constexpr void move_construct(T& to, T& from) GRAILSORT_NOTHROW
	{
		new (&to) T(std::move(from));
	}

	template <typename T>
	constexpr void swap(T& left, T& right) GRAILSORT_NOTHROW
	{
		T tmp(std::move(left));
		new (&left) T(std::move(right));
		new (&right) T(std::move(tmp));
	}

	template <typename Iterator>
	constexpr void block_move(Iterator in_begin, Iterator in_end, Iterator out_begin) GRAILSORT_NOTHROW
	{
		std::move(in_begin, in_end, out_begin);
	}

	template <typename Iterator, typename Int>
	constexpr void block_swap(Iterator left, Iterator right, Int size) GRAILSORT_NOTHROW
	{
		const auto left_end = left + size;
		while (left != left_end)
		{
			swap(*left, *right);
			++left;
			++right;
		}
	}

	template <typename Iterator, typename Int>
	constexpr void rotate_single(Iterator begin, Int left_size) GRAILSORT_NOTHROW
	{
		while (left_size != 0)
		{
			if (left_size <= 1)
			{
				block_swap(begin, begin + left_size, left_size);
				return;
			}
			else
			{
				swap(*(begin + (left_size - 1)), *(begin + left_size));
				--left_size;
			}
		}
	}

	template <typename Iterator, typename Int>
	constexpr void rotate(Iterator begin, Int left_size, Int right_size) GRAILSORT_NOTHROW
	{
		GRAILSORT_ASSUME(left_size >= 0);
		GRAILSORT_ASSUME(right_size >= 0);
		while (left_size != 0 && right_size != 0)
		{
			if (left_size <= right_size)
			{
				block_swap(begin, begin + left_size, left_size);
				begin += left_size;
				right_size -= left_size;
			}
			else
			{
				block_swap(begin + (left_size - right_size), begin + left_size, right_size);
				left_size -= right_size;
			}
		}
	}

	template <typename Iterator, typename Int>
	constexpr Int lower_bound(Iterator begin, Int size, const Iterator key) GRAILSORT_NOTHROW
	{
		Int low = 0;
		Int high = size;
		while (low < high)
		{
			const Int p = low + (high - low) / 2;
			const bool flag = *(begin + p) >= *key;

			if (flag)
				high = p;
			else
				low = p + 1;
		}
		return high;
	}

	template <typename Iterator, typename Int>
	constexpr Int upper_bound(Iterator begin, Int size, const Iterator key) GRAILSORT_NOTHROW
	{
		Int low = 0;
		Int high = size;
		while (low < high)
		{
			const Int p = low + (high - low) / 2;
			const bool flag = *(begin + p) > * key;

			if (flag)
				high = p;
			else
				low = p + 1;
		}
		return high;
	}

	template <typename Iterator>
	constexpr void block_swap(Iterator left_begin, Iterator left_end, Iterator right_begin) GRAILSORT_NOTHROW
	{
		while (left_begin != left_end)
		{
			swap(*left_begin, *right_begin);
			++left_begin;
			++right_begin;
		}
	}

	template <typename Iterator>
	constexpr void rotate_single(Iterator begin, Iterator end) GRAILSORT_NOTHROW
	{
		const Iterator limit = begin + 1;
		while (end != begin)
		{
			if (end <= limit)
			{
				block_swap(begin, end, end);
				return;
			}
			else
			{
				swap(*(end - 1), *end);
				--end;
			}
		}
	}

	template <typename Iterator>
	constexpr Iterator lower_bound(Iterator begin, Iterator end, const Iterator key) GRAILSORT_NOTHROW
	{
		while (begin < end)
		{
			const Iterator p = begin + std::distance(begin, end) / 2;
			if (*p >= *key)
				begin = p;
			else
				end = p + 1;
		}
		return end;
	}

	template <typename Iterator>
	constexpr Iterator upper_bound(Iterator begin, Iterator end, const Iterator key) GRAILSORT_NOTHROW
	{
		while (begin < end)
		{
			const Iterator p = begin + std::distance(begin, end) / 2;
			if (*p > * key)
				begin = p;
			else
				end = p + 1;
		}
		return end;
	}

}