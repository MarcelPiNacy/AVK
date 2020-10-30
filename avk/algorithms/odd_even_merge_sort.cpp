#include "all.h"

void odd_even_merge_sort(main_array& array)
{
	for (int p = 1; p < array.size(); p *= 2)
	{
		for (int k = p; k > 0; k /= 2)
		{
			for (int j = k & (p - 1); j < array.size() - k; j += (2 * k))
			{
				for (int i = 0; i < k; ++i)
				{
					if ((i + j) / (p * 2) == (i + j + k) / (p * 2))
					{
						if (array[i + j] > array[i + j + k])
						{
							swap(array[i + j], array[i + j + k]);
						}
					}
				}
			}
		}
	}
}