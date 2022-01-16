#include "all.h"



static size_t make_triangular(size_t index)
{
	return index * (index + 1) / 2;
}

static void split(main_array array, size_t low, size_t high, size_t offset)
{
	size_t size_mask = (high - low) - 1;
	while (low < high)
	{
		size_t lhs = low + (make_triangular(low + offset) & size_mask);
		size_t rhs = high + (make_triangular(high + offset) & size_mask);
		if (lhs > rhs)
			std::swap(lhs, rhs);
		compare_swap(array[lhs], array[rhs]);
		++low;
		--high;
	}
}

void flipsort(main_array array)
{
	size_t size = array.size();
	for (size_t i = size / 2; i > 0; i /= 2)
	{
		for (size_t j = size; j >= i; j /= 2)
		{
			for (size_t k = 0; k < size / j; ++k)
			{
				split(array, k * j, (k + 1) * j - 1, i);
			}
		}
	}
}