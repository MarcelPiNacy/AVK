#include "all.h"

void insertion_sort(main_array array)
{
	for (ptrdiff_t i = 1; i < (ptrdiff_t)array.size(); ++i)
	{
		item tmp = array[i];
		ptrdiff_t j = i - 1;
		for (; j >= 0 && array[j] > tmp; --j)
			array[j + 1] = array[j];
		array[j + 1] = tmp;
	}
}