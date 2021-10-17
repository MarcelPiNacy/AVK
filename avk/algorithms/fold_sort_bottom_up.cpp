#include "all.h"
#include "sort_utility.h"

static void halver(main_array array, size_t low, size_t high)
{
	while (low < high)
	{
		compare_swap(array, low, high);
		--high;
		++low;
	}
}

void fold_sort_bottom_up(main_array array)
{
	size_t size = array.size();
	size_t log2 = floor_log2(size);

	for (size_t n = 1; n <= size; n *= 2)
	{
		for (size_t step = 1; step <= size; step *= 2)
		{
			for (size_t i = 0; i < size; i += step)
			{
				halver(array, i, i + step - 1);
			}
		}
	}
}