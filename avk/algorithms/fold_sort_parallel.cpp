#include "all.h"



static void halver(main_array array, uint low, uint high)
{
	uint half_range = (high - low) / 2;
	--high;
	if (half_range > 2)
	{
		parallel_for<uint>(0, half_range, [=](uint i)
		{
			compare_swap(array, low + i, high - i);
		});
	}
	else
	{
		compare_swap(array, low, high);
	}
}

static void fold_sort_parallel_step(main_array array, uint low, uint high, uint limit)
{
	uint range = high - low;
	if (range < limit || range < 2)
		return;
	halver(array, low, high);
	uint mid = low + range / 2;
	uint params[2][2] =
	{
		{ low, mid },
		{ mid, high },
	};

	parallel_for<uint>(0, 2, [=](uint i)
	{
		fold_sort_parallel_step(array, params[i][0], params[i][1], limit);
	});
}

void fold_sort_parallel(main_array array)
{
	as_parallel([=]()
	{
		for (uint limit = array.size() / 2; limit > 0; limit /= 2)
		{
			fold_sort_parallel_step(array, 0, array.size(), limit);
		}
	});
}