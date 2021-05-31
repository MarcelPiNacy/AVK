#include "all.h"
#include "../external_dependencies/ths-sort-cpp/thsSort.h"



void thatsoven_stable_sort(main_array array)
{
	thsSort<item>().stableSort(array.begin(), 0, array.size());
}