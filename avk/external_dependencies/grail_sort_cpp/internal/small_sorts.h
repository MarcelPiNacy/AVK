#pragma once
#include "util.h"

namespace grail_sort::detail
{
	template <typename Iterator, typename Int>
	constexpr void insertion_sort_classic(Iterator begin, Int size) GRAILSORT_NOTHROW
	{
		for (Int i = 1; i < size; ++i)
		{
			Int j = i - 1;
			auto tmp = std::move(*(begin + i));
			for (; j >= 0 && *(begin + j) > tmp; --j)
				move_construct(*(begin + (j + 1)), *(begin + j));
			move_construct(*(begin + (j + 1)), tmp);
		}
	}

	template <typename Iterator, typename Int>
	constexpr void unguarded_insert(Iterator begin, Int index) GRAILSORT_NOTHROW
	{
		auto tmp = std::move(*(begin + index));
		--index;
		for (; *(begin + index) > tmp; --index)
			move_construct(*(begin + (index + 1)), *(begin + index));
		move_construct(*(begin + (index + 1)), tmp);
	}

	template <typename Iterator, typename Int>
	constexpr void sink_min_item(Iterator begin, Int size) GRAILSORT_NOTHROW
	{
		Int min = 0;
		for (Int i = 1; i < size; ++i)
			if (*(begin + i) < *(begin + min))
				min = i;
		auto tmp = std::move(*(begin + min));
		for (Int i = min; i > 0; --i)
			move_construct(*(begin + i), *(begin + i - 1));
		move_construct(*begin, tmp);
	}

	template <typename Iterator, typename Int>
	constexpr void insertion_sort_stable(Iterator begin, Int size) GRAILSORT_NOTHROW
	{
		if (size < 8)
			return insertion_sort_classic(begin, size);
		sink_min_item(begin, size);
		for (Int i = 1; i < size; ++i)
			unguarded_insert(begin, i);
	}

	template <typename Iterator, typename Int>
	constexpr void insertion_sort_unstable(Iterator begin, Int size) GRAILSORT_NOTHROW
	{
		for (Int i = 1; i < size; ++i)
		{
			if (*(begin + i) < *begin)
				swap(*begin, *(begin + i));
			unguarded_insert(begin, i);
		}
	}
}