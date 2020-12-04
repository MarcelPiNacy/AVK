#include "all.h"

void insertion_sort(main_array& array)
{
	for (sint i = 1; i < (sint)array.size(); ++i)
	{
		item tmp = array[i];
		sint j = i - 1;
		for (; j >= 0 && array[j] > tmp; --j)
			array[j + 1] = array[j];
		array[j + 1] = tmp;
	}
}