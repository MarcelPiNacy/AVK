#pragma once
#include <cstdint>
#include <utility>
#include <algorithm>



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
		std::swap(left, right);
	}

	template <typename Iterator, typename Int>
	constexpr void block_move(Iterator to, Iterator from, Int count) GRAILSORT_NOTHROW
	{
		std::move(from, from + count, to);
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
				begin += left_size;
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
}