#include "all.h"


static void halver(main_array array, size_t low, size_t high)
{
	size_t half_range = (high - low) / 2;
	--high;
	for (size_t i = 0; i != half_range; ++i)
		compare_swap(array, low + i, high - i);
}

static void fold_sort_recursive_step(main_array array, size_t low, size_t high, size_t limit)
{
	size_t range = high - low;
	halver(array, low, high);
	if (range < limit)
		return;
	size_t mid = low + range / 2;
	fold_sort_recursive_step(array, low, mid, limit);
	fold_sort_recursive_step(array, mid, high, limit);
}

void fold_sort_recursive(main_array array)
{
	for (size_t limit = array.size() / 2; limit > 0; limit /= 2)
	{
		fold_sort_recursive_step(array, 0, array.size(), limit);
	}
}