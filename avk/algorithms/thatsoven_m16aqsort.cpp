#include "all.h"
#include "../external_dependencies/ths-sort-cpp/thsSort.h"



void thatsoven_m16aqsort(main_array array)
{
	thsSort<item>().medianOfSixteenAQSort(array.begin(), 0, (int)array.size(), 64);
}