#include "all.h"

static void weave_compare_swap(main_array array, size_t left, size_t right, size_t limit)
{
	if (right < limit && array[left] > array[right])
		swap(array, left, right);
}

static void circle(main_array array, size_t offset, size_t size, size_t limit, size_t gap)
{
	if (size < 2)
		return;
	for (size_t i = 0; 2 * i < (size - 1) * gap; i += gap)
		weave_compare_swap(array, offset + i, offset + (size - 1) * gap - i, limit);
	circle(array, offset, size / 2, limit, gap);
	if (offset + size * gap / 2 < limit)
		circle(array, offset + size * gap / 2, size / 2, limit, gap);
}

static void weave_circle(main_array array, size_t offset, size_t size, size_t limit, size_t gap)
{
	if (size < 2)
		return;
	weave_circle(array, offset, size / 2, limit, 2 * gap);
	weave_circle(array, offset + gap, size / 2, limit, 2 * gap);
	circle(array, offset, size, limit, gap);
}

void weave_sorting_network_recursive(main_array array)
{
	size_t i = 1;
	while (i < array.size())
		i *= 2;
	weave_circle(array, 0, i, array.size(), 1);
}