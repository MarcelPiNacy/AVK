/*
	MIT License

	Copyright (c) 2020 Marcel Pi Nacy

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#include <iterator>
#include <utility>

namespace detail::binary_quicksort
{
	template <typename I, typename F>
	constexpr I binary_partition(I begin, I end, size_t current_bit, F& extract_bit)
	{
		while (true)
		{
			while (!extract_bit(*begin, current_bit) && begin < end)
			{
				scoped_highlight a(*begin);
				++begin;
			}
			do
			{
				scoped_highlight b(*begin);
				--end;
			} while (extract_bit(*end, current_bit) && begin < end);
			if (begin >= end)
				break;
			std::iter_swap(begin, end);
		}
		return begin;
	}

	template <typename I, typename F>
	constexpr void binary_quicksort_core(I begin, I end, size_t current_bit, size_t min_bit, F& extract_bit)
	{
		using K = typename std::iterator_traits<I>::difference_type;

		while (begin < end)
		{
			I middle = I();
			K left_size = 0;
			K right_size = 0;

			do
			{
				middle = binary_partition(begin, end, current_bit, extract_bit);
				if (current_bit == min_bit)
					return;
				--current_bit;
				left_size = std::distance(begin, middle);
				right_size = std::distance(middle, end);
			} while (left_size < 2 && right_size < 2);

			if (left_size <= right_size)
			{
				binary_quicksort_core(begin, middle, current_bit, min_bit, extract_bit);
				begin = middle;
			}
			else
			{
				binary_quicksort_core(middle, end, current_bit, min_bit, extract_bit);
				end = middle;
			}
		}
	}

	template <typename I, typename F, typename FFS>
	constexpr void entry_point_min_max(I begin, I end, F& extract_bit, FFS&& find_first_set)
	{
		I min = begin;
		I max = begin;
		for (I i = begin; i < end; ++i)
		{
			if (*i < *min)
				min = i;
			if (*i > *max)
				max = i;
		}

		const auto first = find_first_set(*max);
		const auto last = find_first_set(*min);
		binary_quicksort_core(begin, end, last, first, extract_bit);
	}
}

template <typename I, typename F>
constexpr void binary_quicksort(I begin, I end, size_t last_bit, F&& extract_bit)
{
	detail::binary_quicksort::binary_quicksort_core(begin, end, last_bit, 0, std::forward<F>(extract_bit));
}

template <typename I, typename F, typename FFS>
constexpr void binary_quicksort(I begin, I end, F&& extract_bit, FFS&& find_first_set)
{
	detail::binary_quicksort::entry_point_min_max(begin, end, std::forward<F>(extract_bit), std::forward<FFS>(find_first_set));
}