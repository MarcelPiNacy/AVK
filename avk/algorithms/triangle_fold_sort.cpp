#include "all.h"


// unused
static void trivial_halver(main_array array, uint low, uint high)
{
	--high;
	while (low < high)
	{
		compare_swap(array, low, high);
		--high;
		++low;
	}
}

static void triangle_halver(main_array array, uint low, uint high)
{
	uint range = high - low;
	uint half_range = range / 2;
	uint mod_mask = half_range - 1;
	uint mid = low + half_range;
	uint counter = 0;
	--high;
	for (uint i = 0; i != half_range; ++i)
	{
		uint a = low + i;
		uint b = high - i;
		uint c = mid + (((counter * (counter + 1)) / 2) & mod_mask);
		++counter;
		if (b > c)
			std::swap(b, c);
		compare_swap(array, a, c);
	}

	counter = 0;
	for (uint i = 0; i != half_range; ++i)
	{
		uint a = low + i;
		uint b = high - i;
		uint c = mid + (((counter * (counter + 1)) / 2) & mod_mask);
		++counter;
		if (b > c)
			std::swap(b, c);
		compare_swap(array, a, b);
	}
}

static void triangle_sort_recursive_step(main_array array, uint low, uint high, uint limit)
{
	uint range = high - low;
	if (range < limit || range < 2)
		return;
	uint mid = low + range / 2;
	triangle_halver(array, low, high);
	triangle_sort_recursive_step(array, low, mid, limit);
	triangle_sort_recursive_step(array, mid, high, limit);
}

void triangle_fold_sort(main_array array)
{
	for (uint limit = array.size() / 2; limit > 0; limit /= 2)
	{
		triangle_sort_recursive_step(array, 0, array.size(), limit);
	}
}