#include "all.h"
#include "../internal/prng.h"

static void halver(main_array array, size_t low, size_t high)
{
	while (low < high)
	{
		size_t span = (high - low) / 2;
		size_t target;
		if (span != 0)
		{
			target = (size_t)romu2jr_get();
			if (target > span)
				target %= span;
			target += low;
		}
		else
		{
			target = low;
		}
		if (array[low] > array[target])
			swap(array, low, target);
		if (array[target] > array[high])
			swap(array, target, high);
		--high;
		++low;
	}
}

void fold_sort_randomized(main_array array)
{
	size_t size = array.size();
	for (size_t i = size / 2; i > 0; i /= 2)
	{
		for (size_t j = size; j >= i; j /= 2)
		{
			for (size_t k = 0; k != size / j; ++k)
			{
				size_t low = k * j;
				size_t high = (k + 1) * j;
				halver(array, low, high - 1);
			}
		}
	}
}