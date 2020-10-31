#include "all.h"

void bubble_sort(main_array& array)
{
	uint max = array.size();
	bool flag;
	do
	{
		flag = false;
		for (uint i = 1; i < max; ++i)
		{
			if (array[i - 1] > array[i])
			{
				swap(array[i - 1], array[i]);
				flag = true;
			}
		}
		--max;
	} while (flag);
}