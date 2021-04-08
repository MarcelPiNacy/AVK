#include "all.h"

void odd_even_sort(main_array array)
{
	bool flag;
	do
	{
		flag = true;
		for (size_t i = 1; i < array.size() - 1; i += 2)
		{
			if (array[i] > array[i + 1])
			{
				swap(array[i], array[i + 1]);
				flag = false;
			}
		}
		for (size_t i = 0; i < array.size() - 1; i += 2)
		{
			if (array[i] > array[i + 1])
			{
				swap(array[i], array[i + 1]);
				flag = false;
			}
		}
	} while (!flag);
}