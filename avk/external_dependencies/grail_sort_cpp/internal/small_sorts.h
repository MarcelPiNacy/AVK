#pragma once
#include "util.h"

namespace grail_sort::detail
{
	template <typename Iterator>
	constexpr void insertion_sort_classic(Iterator begin, Iterator end)
	{
		for (Iterator i = begin + 1; i < end; ++i)
		{
			Iterator j = i - 1;
			
			auto tmp = std::move(*i);
			
			for (; j >= begin && *j > tmp; --j)
			{
				move_construct(*(j + 1), *j);
			}
			
			move_construct(*(j + 1), tmp);
		}
	}

	template <typename Iterator>
	constexpr void unguarded_insert(Iterator begin, Iterator target)
	{
		auto tmp = std::move(*target);
		
		--target;
		
		for (; *target > tmp; --target)
		{
			move_construct(*(target + 1), *target);
		}
		
		move_construct(*(target + 1), tmp);
	}

	template <typename Iterator>
	constexpr void sink_min_item(Iterator begin, Iterator end)
	{
		Iterator min = begin;
		
		for (Iterator i = min + 1; i < end; ++i)
		{
			if (*i < *min)
			{
				min = i;
			}
		}

		auto tmp = std::move(*min);
		
		for (Iterator i = min; i > 0; --i)
		{
			move_construct(*i, *(i - 1));
		}

		move_construct(*begin, tmp);
	}

	template <typename Iterator>
	constexpr void insertion_sort_stable(Iterator begin, Iterator end)
	{
		if (std::distance(begin, end) < 10)
		{
			return insertion_sort_classic(begin, end);
		}

		sink_min_item(begin, end);

		for (Iterator i = begin + 1; i < end; ++i)
		{
			unguarded_insert(begin, i);
		}
	}

	template <typename Iterator>
	constexpr void insertion_sort_unstable(Iterator begin, Iterator end)
	{
		for (Iterator i = begin + 1; i < end; ++i)
		{
			if (*i < *begin)
				swap(*begin, *i);
			unguarded_insert(begin, i);
		}
	}
}