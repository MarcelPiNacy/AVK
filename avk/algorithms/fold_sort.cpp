#include "all.h"

static void halver(main_array array, size_t low, size_t high)
{
	while (low < high)
	{
		compare_swap(array, low, high);
		--high;
		++low;
	}
}

void fold_sort(main_array array)
{
	size_t size = array.size();
	for (size_t i = size / 2; i > 0; i /= 2)
	{
		for (size_t j = size; j >= i; j /= 2)
		{
			for (size_t k = 0; k < size / j; ++k)
			{
				halver(array, k * j, (k + 1) * j - 1);
			}
		}
	}
}