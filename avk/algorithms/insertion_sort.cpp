#include "all.h"

void insertion_sort(main_array& array)
{
	for (int i = 1; i < array.size(); ++i)
	{
		item tmp = array[i];
		int j = i - 1;
		for (; j >= 0 && array[j] > tmp; --j)
			array[j + 1] = array[j];
		array[j + 1] = tmp;
	}
}