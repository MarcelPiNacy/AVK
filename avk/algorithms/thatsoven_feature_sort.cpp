#include "all.h"
#include "../external_dependencies/ths-sort-cpp/thsSort.h"



void thatsoven_feature_sort(main_array array)
{
	thsSort<item>().featureSort(array.begin(), 0, (int)array.size());
}