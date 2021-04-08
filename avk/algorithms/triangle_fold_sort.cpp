#include "all.h"


// unused
static void trivial_halver(main_array array, size_t low, size_t high)
{
	--high;
	while (low < high)
	{
		compare_swap(array, low, high);
		--high;
		++low;
	}
}

static void triangle_halver(main_array array, size_t low, size_t high)
{
	size_t range = high - low;
	size_t half_range = range / 2;
	size_t mod_mask = half_range - 1;
	size_t mid = low + half_range;
	size_t counter = 0;
	--high;
	for (size_t i = 0; i != half_range; ++i)
	{
		size_t a = low + i;
		size_t b = high - i;
		size_t c = mid + (((counter * (counter + 1)) / 2) & mod_mask);
		++counter;
		if (b > c)
			std::swap(b, c);
		compare_swap(array, a, c);
	}

	counter = 0;
	for (size_t i = 0; i != half_range; ++i)
	{
		size_t a = low + i;
		size_t b = high - i;
		size_t c = mid + (((counter * (counter + 1)) / 2) & mod_mask);
		++counter;
		if (b > c)
			std::swap(b, c);
		compare_swap(array, a, b);
	}
}

static void triangle_sort_recursive_step(main_array array, size_t low, size_t high, size_t limit)
{
	size_t range = high - low;
	if (range < limit || range < 2)
		return;
	size_t mid = low + range / 2;
	triangle_halver(array, low, high);
	triangle_sort_recursive_step(array, low, mid, limit);
	triangle_sort_recursive_step(array, mid, high, limit);
}

void triangle_fold_sort(main_array array)
{
	for (size_t limit = array.size() / 2; limit > 0; limit /= 2)
	{
		triangle_sort_recursive_step(array, 0, array.size(), limit);
	}
}