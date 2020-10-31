#include "all.h"

void insertion_sort(main_array& array)
{
	for (uint i = 1; i < array.size(); ++i)
	{
		item tmp = array[i];
		uint j = i - 1;
		for (; j >= 0 && array[j] > tmp; --j)
			array[j + 1] = array[j];
		array[j + 1] = tmp;
	}
}