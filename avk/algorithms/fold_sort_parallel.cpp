#include "all.h"



static void halver(main_array array, size_t low, size_t high)
{
	size_t half_range = (high - low) / 2;
	--high;
	parallel_for<size_t>(0, half_range, [=](size_t i)
	{
		compare_swap(array, low + i, high - i);
	});
}

static void fold_sort_parallel_step(main_array array, size_t low, size_t high, size_t limit)
{
	size_t range = high - low;
	halver(array, low, high);
	if (range < limit)
		return;
	size_t mid = low + range / 2;
	size_t params[2][2] =
	{
		{ low, mid },
		{ mid, high },
	};

	parallel_for<size_t>(0, 2, [=](size_t i)
	{
		fold_sort_parallel_step(array, params[i][0], params[i][1], limit);
	});
}

void fold_sort_parallel(main_array array)
{
	as_parallel([=]()
	{
		for (size_t limit = array.size() / 2; limit > 0; limit /= 2)
		{
			fold_sort_parallel_step(array, 0, array.size(), limit);
		}
	});
}