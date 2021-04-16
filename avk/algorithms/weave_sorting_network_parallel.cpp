#include "all.h"
#include "../internal/parallel_for.h"

static void weave_compare_swap(main_array array, size_t left, size_t right, size_t limit)
{
	if (right < limit && array[left] > array[right])
		swap(array, left, right);
}

static void circle(main_array array, size_t offset, size_t size, size_t limit, size_t gap)
{
	if (size < 2)
		return;

	size_t n = 0;
	for (size_t i = 0; 2 * i < (size - 1) * gap; i += gap)
		++n;
	
	parallel_for<size_t>(0, n, [&](size_t i)
	{
		i *= gap;
		weave_compare_swap(array, offset + i, offset + (size - 1) * gap - i, limit);
	});

	size_t params[2] =
	{
		offset,
		offset + size * gap / 2
	};
	
	parallel_for(0, 2, [&](int i)
	{
		if (params[i] < limit)
			circle(array, params[i], size / 2, limit, gap);
	});
}

static void weave_circle(main_array array, size_t offset, size_t size, size_t limit, size_t gap)
{
	if (size < 2)
		return;

	size_t params[2] =
	{
		offset,
		offset + gap
	};

	parallel_for(0, 2, [&](int i)
	{
		weave_circle(array, params[i], size / 2, limit, 2 * gap);
	});

	circle(array, offset, size, limit, gap);
}

void weave_sorting_network_parallel(main_array array)
{
	size_t i = 1;
	while (i < array.size())
		i *= 2;
	weave_circle(array, 0, i, array.size(), 1);
}