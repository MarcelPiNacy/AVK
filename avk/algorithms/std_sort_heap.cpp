#include "all.h"
#include <algorithm>

void std_sort_heap(main_array& array)
{
	std::make_heap(array.begin(), array.end());
	std::sort_heap(array.begin(), array.end());
}