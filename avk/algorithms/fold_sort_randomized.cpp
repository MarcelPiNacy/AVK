#include "all.h"
#include "../internal/prng.h"

static void halver(main_array array, uint low, uint high)
{
	while (low < high)
	{
		uint span = (high - low) / 2;
		uint target;
		if (span != 0)
		{
			target = (uint)romu2jr_get();
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
	uint size = array.size();
	for (uint i = size / 2; i > 0; i /= 2)
	{
		for (uint j = size; j >= i; j /= 2)
		{
			for (uint k = 0; k != size / j; ++k)
			{
				uint low = k * j;
				uint high = (k + 1) * j;
				halver(array, low, high - 1);
			}
		}
	}
}