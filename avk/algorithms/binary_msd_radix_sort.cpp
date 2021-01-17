#include "all.h"



#include <iterator>

namespace detail::binary_quicksort
{
	template <typename I, typename F>
	constexpr I binary_partition(I begin, I end, size_t current_bit, F& extract_bit)
	{
		const auto limit = end;
		--end;
		
		while (true)
		{
			while (!extract_bit(*begin, current_bit) && begin < end)
				++begin;
			while (extract_bit(*end, current_bit) && begin < end)
				--end;
			if (begin >= end)
				break;
			std::iter_swap(begin, end);
		}

		if (!extract_bit(*begin, current_bit) && begin < limit)
			++begin;
		return begin;
	}

	template <typename I, typename F>
	constexpr void binary_quicksort_core(I begin, I end, size_t current_bit, F& extract_bit)
	{
		using K = typename std::iterator_traits<I>::difference_type;

		while (begin < end)
		{
			I middle = I();
			K left_size = 0;
			K right_size = 0;

			while (true)
			{
				middle = binary_partition(begin, end, current_bit, extract_bit);
				if (current_bit == 0)
					return;
				--current_bit;
				left_size = std::distance(begin, middle);
				right_size = std::distance(middle, end);
				const bool is_right_tiny = right_size < 2;
				if (!left_size < 2 || is_right_tiny)
					break;
				begin = is_right_tiny ? begin : middle;
				end = is_right_tiny ? middle : end;
			}

			if (left_size <= right_size)
			{
				binary_quicksort_core(begin, middle, current_bit, extract_bit);
				begin = middle;
			}
			else
			{
				binary_quicksort_core(middle, end, current_bit, extract_bit);
				end = middle;
			}
		}
	}
}

template <typename I, typename F>
constexpr void binary_quicksort(I begin, I end, size_t max_bit, F&& extract_bit)
{
	detail::binary_quicksort::binary_quicksort_core(begin, end, max_bit, extract_bit);
}



void binary_msd_radix_sort(main_array array)
{
	return binary_quicksort(array.begin(), array.end(), item::max_radix(2) - 1, [](const item& e, size_t bit_index) { return ((e.value >> bit_index) & 1) != 0; });
}