#include "all.h"

static void halver(main_array array, uint low, uint high)
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
	uint size = array.size();
	for (uint i = size / 2; i > 0; i /= 2)
	{
		for (uint j = size; j >= i; j /= 2)
		{
			for (uint k = 0; k < size / j; ++k)
			{
				uint low = k * j;
				uint high = (k + 1) * j;
				halver(array, low, high - 1);
			}
		}
	}
}