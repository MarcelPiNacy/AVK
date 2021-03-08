#include "all.h"
#include <algorithm>

void max_heap_sort(main_array array)
{
	std::make_heap(array.begin(), array.end());
	std::sort_heap(array.begin(), array.end());
}