#include "all.h"
#include "sort_utility.h"

static void halver(main_array array, uint low, uint high)
{
	while (low < high)
	{
		compare_swap(array, low, high);
		++low;
		--high;
	}
}

void fold_sort_bottom_up(main_array array)
{
	uint size = array.size();
	uint log2 = fast_log2(size);

	for (uint n = 0; n <= log2; ++n)
	{
		for (uint step = 1; step <= size; step *= 2)
		{
			for (uint i = 0; i < size; i += step)
			{
				halver(array, i, i + step - 1);
			}
		}
	}
}