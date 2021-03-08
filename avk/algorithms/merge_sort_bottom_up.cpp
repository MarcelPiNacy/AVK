#include "all.h"
#include <algorithm>

void merge_sort_bottom_up(main_array array)
{
	for (uint i = 0; i < array.size(); i += 2)
		if (array[i] > array[i + 1])
			swap(array[i], array[i + 1]);

	uint merge_size = 2;
	while (merge_size < array.size())
	{
		uint next = merge_size * 2;
		for (uint i = 0; i < array.size(); i += next)
		{
			auto p = array.begin() + i;
			std::inplace_merge(p, p + merge_size, p + next);
		}
		merge_size = next;
	}
}